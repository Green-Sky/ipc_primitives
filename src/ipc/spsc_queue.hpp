#pragma once

// ipc fifo queue
// not optimized, naive locking (always locking both mutexes)
// uses shared memory and semaphores to sync
// !! make sure not to send pointers !!

#include <cstddef>
#include <string>
#include <mutex> // lock_guard

#include "./shared_memory.hpp"
#include "./semaphore.hpp"

// TODO:
// - index alignment
// - index cacheline spacing


template<typename T>
class IPCSPSCQueue {
	// needs to contain _capacity Ts and 2 size_t for begin and end of ring buffer
	IPCSharedMemory& _shared_memory;

	// used as mutex
	IPCSemaphore& _sem_writer_index;
	IPCSemaphore& _sem_reader_index;

	size_t _capacity {0};

	// technically its "next" write index
	size_t* getWriteIndex(void) const {
		if (_capacity == 0 || _shared_memory.data() == nullptr) {
			return nullptr;
		}

		// start of tail section
		auto* tail_start = _shared_memory.data() + (_capacity*sizeof(T));
		// first write, then read
		return reinterpret_cast<size_t*>(tail_start);
	}

	size_t* getReadIndex(void) const {
		if (_capacity == 0 || _shared_memory.data() == nullptr) {
			return nullptr;
		}

		// start of tail section
		auto* tail_start = _shared_memory.data() + (_capacity*sizeof(T));
		// first write, then read
		return reinterpret_cast<size_t*>(tail_start+sizeof(size_t));
	}

	public:
		explicit IPCSPSCQueue(
			const size_t capacity, // for how many Ts should be in the ring buffer
			IPCSharedMemory& shared_memory,
			IPCSemaphore& sem_writer_index,
			IPCSemaphore& sem_reader_index
		) : _shared_memory(shared_memory), _sem_writer_index(sem_writer_index), _sem_reader_index(sem_reader_index) {
			if (capacity < 1) {
				_capacity = 1;
			} else {
				_capacity = capacity;
			}

			if (_shared_memory.size() < neededSharedSize(_capacity)) {
				_capacity = 0; // hack
				return;
			}

		}

		// only call this once! so ideall on the creator
		void init(void) {
			{ // init writer
				std::lock_guard lg {_sem_writer_index};
				auto idx_ptr = getWriteIndex();
				if (idx_ptr == nullptr) {
					return;
				}
				*idx_ptr = 0u;
			}

			{ // init reader
				std::lock_guard lg {_sem_reader_index};
				auto idx_ptr = getReadIndex();
				if (idx_ptr == nullptr) {
					return;
				}
				*idx_ptr = 0u;
			}
		}

		~IPCSPSCQueue(void) {
			// TODO: pop all remaining?
			// assuming noone is adding to it at this stage
			const auto count = size();
			for (size_t i = 0; i < count; i++) {
				pop();
			}
		}

		bool isConnected(void) const {
			return
				_capacity > 0 &&
				_shared_memory.isOpen() &&
				_sem_reader_index.isOpen() &&
				_sem_writer_index.isOpen()
			;
		}

		[[nodiscard]] T* front(void) {
			// first copy the values
			size_t write_index {0};
			size_t read_index {0};
			{
				// TODO: do we need the multi lock? probably not?
				std::scoped_lock sl {_sem_writer_index, _sem_reader_index};
				write_index = *getWriteIndex();
				read_index = *getReadIndex();
				// we do this so the writer lock is released here
			}

			// if empty
			if (write_index == read_index) {
				return nullptr; // nothing to pop !!
			}

			T* data_array = reinterpret_cast<T*>(_shared_memory.data());
			return data_array + read_index;
		}

		void pop(void) {
			// first copy the values
			size_t write_index {0};
			size_t read_index {0};
			{
				// TODO: do we need the multi lock? probably not?
				std::scoped_lock sl {_sem_writer_index, _sem_reader_index};
				write_index = *getWriteIndex();
				read_index = *getReadIndex();
				// we do this so the writer lock is released here
			}

			// if empty
			if (write_index == read_index) {
				return; // nothing to pop !!
			}

			{ // remove element
				std::lock_guard lg {_sem_reader_index};

				auto* data_array = reinterpret_cast<T*>(_shared_memory.data());
				data_array[read_index].~T(); // destruct

				read_index += 1;
				if (read_index == _capacity) {
					read_index = 0;
				}
				*getReadIndex() = read_index;
			}
		}

		bool try_push(const T& value) {
			// first copy the values
			size_t write_index {0};
			size_t read_index {0};
			{
				// TODO: do we need the multi lock?
				std::scoped_lock sl {_sem_writer_index, _sem_reader_index};
				write_index = *getWriteIndex();
				read_index = *getReadIndex();
				// we do this so the reader lock is released here
			}

			// if full
			if ((write_index+1)%_capacity == read_index) {
				return false; // no space
			}

			{ // add element
				std::lock_guard lg {_sem_writer_index};

				auto* data_array = reinterpret_cast<T*>(_shared_memory.data());
				new(data_array+write_index) T(value); // placement new

				write_index += 1;
				if (write_index == _capacity) {
					write_index = 0;
				}

				*getWriteIndex() = write_index;
			}
			return true;
		}

		size_t size(void) const {
			if (_capacity == 0) {
				return 0;
			}

			// first copy the values
			size_t write_index {0};
			size_t read_index {0};
			{
				// TODO: do we need the multi lock?
				std::scoped_lock sl {_sem_writer_index, _sem_reader_index};
				write_index = *getWriteIndex();
				read_index = *getReadIndex();
			}

			if (write_index >= read_index) {
				return write_index - read_index;
			}

			// wrap around situation
			// TODO: make more elegant
			// else if (write_index < read_index) {
				return (write_index + _capacity) - read_index;
			//}
		}

		// empty

		size_t capacity(void) const {
			return _capacity;
		}

		// return the minimum size a shared memory needs to have
		static constexpr size_t neededSharedSize(const size_t capacity) {
			// items + trailing begin and end of circular buffer
			// TODO: cache line spacing
			return capacity * sizeof(T) + 2 * sizeof(size_t);
		}
};

#include <ipc/spsc_queue.hpp>
#include <ipc/semaphore.hpp>
#include <ipc/shared_memory.hpp>

#include <array>
#include <cassert>
#include <iostream>

struct TestQueueEntry {
	size_t num1 {0};
	std::array<uint8_t, 64> data;
	static_assert(sizeof(data) >= sizeof(uint8_t)*64);
};

static constexpr size_t queue_capacity {128};

int main(void) {
	{ // very simple, same process, same thread
		static constexpr size_t shared_memory_size_minimum {IPCSPSCQueue<TestQueueEntry>::neededSharedSize(queue_capacity)};

		// producer
		// create
		IPCSharedMemory prod_mem("test_queue1_mem_name_xxx", shared_memory_size_minimum, true);
		assert(prod_mem.isOpen());
		std::cout << "created prod_mem\n";

		IPCSemaphore prod_sem_w_idx("test_queue1_sem_w_idx_name_xxx", true, 1);
		assert(prod_sem_w_idx.isOpen());
		std::cout << "created prod_sem_w_idx\n";

		IPCSemaphore prod_sem_r_idx("test_queue1_sem_r_idx_name_xxx", true, 1);
		assert(prod_sem_r_idx.isOpen());
		std::cout << "created prod_sem_r_idx\n";

		IPCSPSCQueue<TestQueueEntry> prod_queue(queue_capacity, prod_mem, prod_sem_w_idx, prod_sem_r_idx);
		assert(prod_queue.isConnected());
		prod_queue.init(); // TODO: make this more pretty

		// consumer
		// open
		IPCSharedMemory cons_mem("test_queue1_mem_name_xxx", shared_memory_size_minimum);
		assert(cons_mem.isOpen());
		std::cout << "opened cons_mem\n";

		IPCSemaphore cons_sem_w_idx("test_queue1_sem_w_idx_name_xxx", true, 1);
		assert(cons_sem_w_idx.isOpen());
		std::cout << "opened cons_sem_w_idx\n";

		IPCSemaphore cons_sem_r_idx("test_queue1_sem_r_idx_name_xxx", true, 1);
		assert(cons_sem_r_idx.isOpen());
		std::cout << "opened cons_sem_r_idx\n";

		IPCSPSCQueue<TestQueueEntry> cons_queue(queue_capacity, cons_mem, cons_sem_w_idx, cons_sem_r_idx);
		assert(cons_queue.isConnected());


		// all set up

		{ // push empty into producer
			const auto push_ret = prod_queue.try_push({});
			assert(push_ret == true);
			assert(prod_queue.size() == 1);
			assert(cons_queue.size() == 1);
			assert(cons_queue.front() != nullptr);
			cons_queue.pop();
			assert(prod_queue.size() == 0);
			assert(cons_queue.size() == 0);
		}

		{ // push magic number
			const auto push_ret = prod_queue.try_push({1337, {}});
			assert(push_ret == true);
			assert(prod_queue.size() == 1);
			assert(cons_queue.size() == 1);
			{
				const auto* element = cons_queue.front();
				assert(element != nullptr);
				assert(element->num1 == 1337);
			}
			cons_queue.pop();
		}

		{ // simple sequential push pop > capacity
			for (size_t i = 0; i < queue_capacity*20; i++) {
				const auto push_ret = prod_queue.try_push({1337+i, {}});
				assert(push_ret == true);
				assert(prod_queue.size() == 1);
				assert(cons_queue.size() == 1);
				{
					const auto* element = cons_queue.front();
					assert(element != nullptr);
					assert(element->num1 == 1337+i);
				}
				cons_queue.pop();
			}
		}

		{ // fill queue to max
			// TODO: due to the fill state logic, the last slot can not be filled
			for (size_t i = 0; i < queue_capacity-1; i++) {
				const auto push_ret = prod_queue.try_push({1337+i, {}});
				assert(push_ret == true);
				assert(prod_queue.size() == i+1);
				assert(cons_queue.size() == i+1);
			}

			const auto push_ret = prod_queue.try_push({42, {}});
			assert(push_ret == false);

			for (size_t i = 0; i < queue_capacity-1; i++) {
				const auto* element = cons_queue.front();
				assert(element != nullptr);
				cons_queue.pop();
			}

			assert(cons_queue.size() == 0);
			assert(cons_queue.front() == nullptr);
		}
	}

	return 0;
}


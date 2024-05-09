#include <chrono>
#include <cstddef>
#include <ipc/spsc_queue.hpp>
#include <ipc/semaphore.hpp>
#include <ipc/shared_memory.hpp>

#include <array>
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

struct TestQueueEntry {
	size_t num1 {0};
	std::array<uint8_t, 64> data;
	static_assert(sizeof(data) >= sizeof(uint8_t)*64);
};

static constexpr size_t queue_capacity {128};
static constexpr size_t shared_memory_size_minimum {IPCSPSCQueue<TestQueueEntry>::neededSharedSize(queue_capacity)};
static constexpr size_t number_of_elements_to_push {queue_capacity*10};

static std::atomic<bool> prod_setup {false};

int main(void) {
	std::thread t_prod{[](void) {
		// producer
		// create
		IPCSharedMemory prod_mem("test_queue2_mem_name_xxx", shared_memory_size_minimum, true);
		assert(prod_mem.isOpen());
		std::cout << "created prod_mem\n";

		IPCSemaphore prod_sem_w_idx("test_queue2_sem_w_idx_name_xxx", true, 1);
		assert(prod_sem_w_idx.isOpen());
		std::cout << "created prod_sem_w_idx\n";

		IPCSemaphore prod_sem_r_idx("test_queue2_sem_r_idx_name_xxx", true, 1);
		assert(prod_sem_r_idx.isOpen());
		std::cout << "created prod_sem_r_idx\n";

		IPCSPSCQueue<TestQueueEntry> prod_queue(queue_capacity, prod_mem, prod_sem_w_idx, prod_sem_r_idx);
		assert(prod_queue.isConnected());
		prod_queue.init(); // TODO: make this more pretty

		std::cout << "created queue\n";

		prod_setup = true;

		// we can just start pumping now
		for (size_t i = 0; i < number_of_elements_to_push;) {
			// TODO: blocking push
			const auto push_ret = prod_queue.try_push({1337+i, {}});
			if (push_ret) {
				//std::cerr << "pushed " << i << "\n";
				i++;
			} else {
				//std::cerr << "failed to push\n";
			}
		}
	}};

	while (!prod_setup) {
		// cheap spin lock
		// could be moved into t_cons
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::thread t_cons{[](void) {
		// consumer
		// open
		IPCSharedMemory cons_mem("test_queue2_mem_name_xxx", shared_memory_size_minimum);
		assert(cons_mem.isOpen());
		std::cout << "opened cons_mem\n";

		IPCSemaphore cons_sem_w_idx("test_queue2_sem_w_idx_name_xxx", true, 1);
		assert(cons_sem_w_idx.isOpen());
		std::cout << "opened cons_sem_w_idx\n";

		IPCSemaphore cons_sem_r_idx("test_queue2_sem_r_idx_name_xxx", true, 1);
		assert(cons_sem_r_idx.isOpen());
		std::cout << "opened cons_sem_r_idx\n";

		IPCSPSCQueue<TestQueueEntry> cons_queue(queue_capacity, cons_mem, cons_sem_w_idx, cons_sem_r_idx);
		assert(cons_queue.isConnected());
		std::cout << "opened queue\n";

		for (size_t i = 0; i < number_of_elements_to_push;) {
			const auto* element = cons_queue.front();
			if (element != nullptr) {
				assert(element->num1 == 1337+i);
				cons_queue.pop();
				i++;
			}
		}
	}};

	t_prod.join();
	t_cons.join();

	return 0;
}


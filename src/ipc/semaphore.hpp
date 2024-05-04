#pragma once

// cross process semaphore

#include <string>

class IPCSemaphore {
	bool _owner {false};
	std::string _name;

#if defined(_WIN32)
	//HANDLE handle;
	void* _handle {0};
#else
	// sem_t*
	void* _semaphore {nullptr};
#endif

	public:
		IPCSemaphore(const std::string& name, bool create_new = false, uint32_t initial_value = 0);
		~IPCSemaphore(void);

		void release(void);
		void aquire(void);
		bool tryAquire(void);

		bool isOpen(void) const;

		const std::string& name(void) const { return _name; }
};

#pragma once

// cross process semaphore

#include <string>


class Semaphore {
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
		Semaphore(const std::string& name, bool create_new = false, uint32_t initial_value = 0);
		~Semaphore(void);

		void release(void);
		void aquire(void);
		bool tryAquire(void);

		bool isOpen(void) const;

		const std::string& name(void) const { return _name; }
};

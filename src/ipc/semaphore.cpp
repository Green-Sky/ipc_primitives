#include "./semaphore.hpp"

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <semaphore.h>
#endif


#if defined(_WIN32)

IPCSemaphore::IPCSemaphore(const std::string& name, bool create_new, uint32_t initial_value) : _owner(create_new), _name(name) {
	if (create_new) {
		_handle = CreateSemaphoreA(nullptr, initial_value, 0x7fffffff, _name.c_str());
	} else {
		_handle = OpenSemaphoreA(0, false, _name.c_str());
	}

	if (_handle == 0) {
		return;
	}
}

IPCSemaphore::~IPCSemaphore(void) {
	// nothing special for owner
	CloseHandle(_handle);
}

void IPCSemaphore::release(void) {
	ReleaseSemaphore(_handle, 1, nullptr);
}

void IPCSemaphore::aquire(void) {
	WaitForSingleObject(_handle, INFINITE);
}

bool IPCSemaphore::tryAquire(void) {
	return WaitForSingleObject(_handle, 0) == WAIT_OBJECT_0;
}

bool IPCSemaphore::isOpen(void) const {
	return _handle != 0;
}

#else

IPCSemaphore::IPCSemaphore(const std::string& name, bool create_new, uint32_t initial_value) : _owner(create_new), _name(name) {
	if (create_new) {
		_semaphore = sem_open(_name.c_str(), O_CREAT, 0700, initial_value);
	} else {
		_semaphore = sem_open(_name.c_str(), 0);
	}

	if (_semaphore == SEM_FAILED) {
		_semaphore = nullptr;
	}
}

IPCSemaphore::~IPCSemaphore(void) {
	sem_close((sem_t*)_semaphore);
	if (_owner) {
		sem_unlink(_name.c_str());
	}
}

void IPCSemaphore::release(void) {
	sem_post((sem_t*)_semaphore);
}

void IPCSemaphore::aquire(void) {
	sem_wait((sem_t*)_semaphore);
}

bool IPCSemaphore::tryAquire(void) {
	return sem_trywait((sem_t*)_semaphore) == 0;
}

bool IPCSemaphore::isOpen(void) const {
	return _semaphore != nullptr;
}

#endif


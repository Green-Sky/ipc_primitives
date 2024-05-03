#include "./shared_memory.hpp"

// TODO: more platforms for the else case (macos, android ...)

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else // !win
	#include <fcntl.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <errno.h>
#endif // !win

#include <iostream>

#if defined(_WIN32)

// windows

SharedMemory::SharedMemory(const char* name_str, size_t size, bool create_new) : _name(name_str), _size(size), _owner(create_new) {
	if (create_new) {
		_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, _size, _name.c_str());

		if (_handle == 0) {
			std::cerr << "failed to create file mapping\n";
			return;
		}

		_data = (std::uint8_t*)MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, _size);
		if (_data == nullptr) {
			std::cerr << "failed to create map file\n";
			CloseHandle(_handle);
			return;
		}
	} else {
		_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, _name.c_str());

		if (_handle <= 0) {
			std::cerr << "failed to open file\n";
			return;
		}

		_data = (std::uint8_t*)MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, /*_size*/0);
		if (_data == nullptr) {
			std::cerr << "failed to map file\n";
			CloseHandle(_handle);
			return;
		}

		//MEMORY_BASIC_INFORMATION mbi;
		//if (VirtualQuery(_data, &mbi, sizeof(mbi)) != sizeof(mbi)) {
			//std::cerr << "failed to get size of mapped view\n";
			//return;
		//}

		//_size = mbi.RegionSize;
		//std::cerr << "the mapped view is of size " << _size << "\n";
	}
}

SharedMemory::~SharedMemory(void) {
	if (_data != nullptr) {
		UnmapViewOfFile(_data);
		_data = nullptr;
		CloseHandle(_handle);
	}
}

#else // !win

// linux (maybe posix-ish)

SharedMemory::SharedMemory(const char* name_str, size_t size, bool create_new) : _name(name_str), _size(size), _owner(create_new) {
	if (create_new) {
		// first delete existing shared memory with the same name
		auto ret = shm_unlink(_name.c_str());
		if (ret < 0 && errno != ENOENT) {
			return;
		}

		_fd = shm_open(_name.c_str(), O_CREAT | O_RDWR, 0755);
		if (_fd < 0) {
			return;
		}

		// resize file to desired size, before we map it
		if (ftruncate(_fd, _size) < 0) {
			close(_fd);
			return;
		}

		_data = (std::uint8_t*)mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
		if (_data == nullptr) {
			close(_fd);
			return;
		}
	} else {
		_fd = shm_open(_name.c_str(), O_RDWR, 0755);
		if (_fd < 0) {
			return;
		}

		//struct stat sb;
		//if (fstat(_fd, &sb) < 0) {
			//close(_fd);
			//return;
		//}

		//_size = sb.st_size;

		_data = (std::uint8_t*)mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
		if (_data == nullptr) {
			close(_fd);
			_size = 0;
			return;
		}
	}
}

SharedMemory::~SharedMemory(void) {
	if (_data != nullptr) {
		munmap(_data, _size);
		close(_fd);
		if (_owner) {
			shm_unlink(_name.c_str());
		}
	}
}

#endif // !win


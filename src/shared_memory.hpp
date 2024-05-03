#pragma once

// crossplatform shared memory c++ abstraction
// for use between processes
//
// based on ipc.h by Jari Komppa https://github.com/jarikomppa/ipc

#include <cstdint>
#include <string>

class SharedMemory final {
	std::string _name;
	std::size_t _size {0};
	std::uint8_t* _data {nullptr};
	bool _owner {false}; // the one who created it

	// the platform specific part
#if defined(_WIN32)
	//HANDLE _handle {0};
	void* _handle {0};
#else
	int _fd {-1};
#endif

	public:
		// seperate open existing / create new
		SharedMemory(const std::string& name, size_t size, bool create_new = false);

		// non copyable
		SharedMemory(const SharedMemory& other) = delete;
		// moveable
		SharedMemory(SharedMemory&& other) = default;

		~SharedMemory(void);

		// clone (for non creating objects, which just opens it again)

		bool isOpen(void) const { return _data != nullptr && _size != 0; }
		std::size_t size(void) const { return _size; }

		std::uint8_t* data(void) { return _data; }
		const std::uint8_t* data(void) const { return _data; }

		const std::string& name(void) const { return _name; }
};


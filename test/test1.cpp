#include <shared_memory.hpp>

#include <cassert>
#include <iostream>

int main(void) {
	{ // very simple, same process
		// create
		SharedMemory mem0_c("test1_mem_name_xxx_0", 16, true);
		assert(mem0_c.isOpen());
		std::cout << "created mem0_c\n";

		// open
		SharedMemory mem0_o("test1_mem_name_xxx_0", 16);
		assert(mem0_o.isOpen());
		std::cout << "created mem0_o\n";

		mem0_c.data()[0] = 13u;

		assert(mem0_o.data()[0] == 13u);
	}

	return 0;
}


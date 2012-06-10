#include <cstring>
#include <iostream>
#include <system_error>

#include "radeon_device.hpp"
#include "radeon_gem_buffer_object.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr
            << "Usage " << argv[0]
            << " /dev/dri/card?"
            << std::endl;
        return 1;
    }

    try {
        radeon_device dev(argv[1], false);
        std::cout << 1 << std::endl;
        radeon_gem_buffer_object bo(dev, 2<<20);
        std::cout << 2 << std::endl;
        {
            void* ptr = bo.mmap(0, 2<<20);
            std::memset(ptr, 0, 2<<20);
            bo.munmap();
        }
        std::cout << 3 << std::endl;
        {
#if 0
            void* ptr = bo.mmap(1<<20, 1<<20); // non-zero offset not implemented
            std::memset(ptr, 77, 1<<20);
            bo.munmap();
#elif 0
            char buffer[1024];
            std::memset(buffer, 77, sizeof(buffer));
            bo.pwrite(1<<20, 1024, buffer); // pwrite not implemented
#elif 1
            char* ptr = static_cast<char*>(bo.mmap(0, 2<<20));
            std::memset(ptr + (1<<20), 77, 1<<20);
            bo.munmap();
#endif
        }
        std::cout << 4 << std::endl;
        {
            char* ptr = static_cast<char*>(bo.mmap(0, 2<<20));
            char* p2 = static_cast<char*>(std::memchr(ptr, 77, 2<<20));
            bo.munmap();
            std::cout << (p2 - ptr) << std::endl;
        }
    }
    catch (std::system_error& e) {
        std::cerr
            << e.what()
            << " : "
            << e.code().message()
            << std::endl;
        return 1;
    }
    return 0;
}

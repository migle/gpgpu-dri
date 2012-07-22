#include <cstring>
#include <iostream>
#include <system_error>

#include "radeon_device.hpp"
#include "radeon_buffer_object.hpp"

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
        std::cout << "Opening " << argv[1] << std::endl;
        radeon_device dev(argv[1], false);
        std::uint32_t bo_size = 2 << 20;
        radeon_buffer_object bo(dev, bo_size, RADEON_GEM_DOMAIN_VRAM);
        std::cout << "BO handle = " << bo.handle() << " size = " << bo_size << std::endl;
        {
            void* ptr = bo.mmap(0, bo_size);
            std::memset(ptr, 0, bo_size);
            bo.munmap();
        }
        std::cout << "BO set to zero" << std::endl;
        {
#if 0
            void* ptr = bo.mmap(bo_size >> 1, bo_size >> 1); // non-zero offset not implemented
            std::memset(ptr, 77, bo_size >> 1);
            bo.munmap();
#elif 0
            char buffer[1024];
            std::memset(buffer, 77, sizeof(buffer));
            bo.pwrite(bo_size >> 1, 1024, buffer); // pwrite not implemented
#elif 1
            char* ptr = static_cast<char*>(bo.mmap(0, bo_size));
            std::memset(ptr + (bo_size >> 1), 77, bo_size >> 1);
            bo.munmap();
#endif
        }
        std::cout << "BO second half set to 0x77" << std::endl;
        bool ok = false;
        {
            char* ptr = static_cast<char*>(bo.mmap(0, bo_size));
            char* p2 = static_cast<char*>(std::memchr(ptr, 77, bo_size));
            bo.munmap();
            std::cout << "First 0x77 appears at " << (p2 - ptr) << std::endl;
            ok = (p2 - ptr) == (bo_size >> 1);
        }
        if (ok)
            std::cout << "Ok" << std::endl;
        else
            std::cout << "Failed" << std::endl;
        return ok ? 0 : 1;
    }
    catch (std::system_error& e) {
        std::cerr
            << e.what()
            << " : "
            << e.code().message()
            << std::endl;
        return 1;
    }
}

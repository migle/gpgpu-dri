#include <cstring>
#include <iostream>
#include <sstream>
#include <system_error>

#include "radeon_device.hpp"
#include "radeon_buffer_object.hpp"
#include "hex_dump.hpp"

int main(int argc, char* argv[])
{
    std::uint32_t name = 0, word = 0;
    std::uint64_t offset = 0, size = 0;

    if (argc != 5 && argc != 6 ||
        !(std::istringstream(argv[2]) >> name) ||
        !(std::istringstream(argv[3]) >> offset) ||
        !(std::istringstream(argv[4]) >> size) || size > 4096 ||
        argc == 6 && !(std::istringstream(argv[5]) >> word) || word > 3)
    {
        std::cerr
            << "Usage " << argv[0]
            << " /dev/dri/card? <bo-name> <offset> <size> [<word>]"
            << std::endl;
        return 1;
    }

    try {
        std::cout << "Opening " << argv[1] << std::endl;
        radeon_device dev(argv[1], false);
        radeon_buffer_object bo(dev, name);
        std::cout << "BO handle = " << bo.handle() << " size = " << bo.size() << std::endl;
#if 1
        void* base = bo.mmap(0, bo.size());
        std::uint8_t* ptr = static_cast<std::uint8_t*>(base) + offset;
        switch (word) {
            default:
            case 0:
                std::cout << hex_dump<std::uint8_t>(reinterpret_cast<std::uint8_t*>(ptr), size / sizeof(std::uint8_t), 16) << std::endl;
                break;
            case 1:
                std::cout << hex_dump<std::uint16_t>(reinterpret_cast<std::uint16_t*>(ptr), size / sizeof(std::uint16_t), 16) << std::endl;
                break;
            case 2:
                std::cout << hex_dump<std::uint32_t>(reinterpret_cast<std::uint32_t*>(ptr), size / sizeof(std::uint32_t), 8) << std::endl;
                break;
            case 3:
                std::cout << hex_dump<std::uint64_t>(reinterpret_cast<std::uint64_t*>(ptr), size / sizeof(std::uint64_t), 4) << std::endl;
                break;
        }
        bo.munmap();
#else
        void* ptr = bo.mmap(offset, size);
        switch (word) {
            default:
            case 0:
                std::cout << hex_dump<std::uint8_t>(static_cast<std::uint8_t*>(ptr), size / sizeof(std::uint8_t), 16) << std::endl;
                break;
            case 1:
                std::cout << hex_dump<std::uint16_t>(static_cast<std::uint16_t*>(ptr), size / sizeof(std::uint16_t), 16) << std::endl;
                break;
            case 2:
                std::cout << hex_dump<std::uint32_t>(static_cast<std::uint32_t*>(ptr), size / sizeof(std::uint32_t), 8) << std::endl;
                break;
            case 3:
                std::cout << hex_dump<std::uint64_t>(static_cast<std::uint64_t*>(ptr), size / sizeof(std::uint64_t), 4) << std::endl;
                break;
        }
        bo.munmap();
#endif
        return 0;
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

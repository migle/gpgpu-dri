#include <cstring>
#include <iostream>
#include <iomanip>
#include <system_error>

#include "radeon_device.hpp"
#include "radeon_buffer_object.hpp"
#include "radeon_command_stream.hpp"

#include "r600d.h"

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

        // Create a single, small BO for communication with the GPU.
        radeon_buffer_object bo(dev, 4096);
        std::cout << 1 << ' ' << bo.handle() << std::endl;
        // Map the BO and initialize it to zero.
        {
            void* ptr = bo.mmap(0, 4096);
            std::memset(ptr, 0, 4096);
            bo.munmap();
        }
        std::cout << 2 << std::endl;

        // Create the CS for the GPU.
        radeon_command_stream cs(dev);
        std::cout << 3 << std::endl;

        {
#if 0
            // This would be simple, but the CS parser in Linux does not support
            // the IT opcode of MEM_WRITE.

            // We start with an easier MEM_WRITE of 64-bit data to QWORD-aligned location.
            cs.write({
                PACKET3(PACKET3_MEM_WRITE, 3),
                0u & ~0x3u, // lower 32 bits of address
                (0 << 18) | // DATA32
                (0 << 17) | // WR_CONFIRM
                (0 << 16) | // CNTR_SEL
                ((0ul >> 32) & 0xff), // upper 32-39 bits of address
                0x9abcdef0,
                0x12345678
                });

            // Then a MEM_WRITE of 32-bit data to a DWORD-aligned location.
            cs.write({
                PACKET3(PACKET3_MEM_WRITE, 3),
                12u & ~0x3u, // lower 32 bits of address
                (1 << 18) | // DATA32
                (0 << 17) | // WR_CONFIRM
                (0 << 16) | // CNTR_SEL
                ((0ul >> 32) & 0xff), // upper 32-39 bits of address
                0x9abcdef1,
                0x12345678
                });

            // Then a MEM_WRITE of the 64-bit timestamp counter of the GPU.
            cs.write({
                PACKET3(PACKET3_MEM_WRITE, 3),
                24& ~0x3u, // lower 32 bits of address
                (0 << 18) | // DATA32
                (0 << 17) | // WR_CONFIRM
                (1 << 16) | // CNTR_SEL
                ((0ul >> 32) & 0xff), // upper 32-39 bits of address
                0x9abcdef2,
                0x12345678
                });
#endif

            /// According to AMD Radeon R6xx/R7xx Acceleration document version 1.0
            /// section 4.4.1 on page 21:
            ///     Driver can know that submitted PM4 packets have been fetched by
            ///     CP. One of the ways is (fence/time stamp write back to scratch
            ///     register) which is often used to confirm some packets have been
            ///     processed by CP.

#if 0
            // Fence, write 32-bit data.
            std::uint64_t address0 = 16;
            cs.write({
                PACKET3(PACKET3_EVENT_WRITE_EOP, 4),
                EVENT_TYPE(CACHE_FLUSH_AND_INV_EVENT) | EVENT_INDEX(5),
                std::uint32_t(address0) & ~3u,
                DATA_SEL(1) | INT_SEL(0) | std::uint32_t((address0 >> 32u) & 0xffu),
                0x89abcdef,
                0x01234567
                });
            cs.write_reloc(bo.handle(), RADEON_GEM_DOMAIN_VRAM, RADEON_GEM_DOMAIN_VRAM);

            // Fence, write 64-bit data.
            std::uint64_t address1 = 32;
            cs.write({
                PACKET3(PACKET3_EVENT_WRITE_EOP, 4),
                EVENT_TYPE(CACHE_FLUSH_AND_INV_EVENT) | EVENT_INDEX(5),
                std::uint32_t(address1) & ~3u,
                DATA_SEL(2) | INT_SEL(0) | std::uint32_t((address1 >> 32u) & 0xffu),
                0x89abcdef,
                0x01234567
                });
            cs.write_reloc(bo.handle(), RADEON_GEM_DOMAIN_VRAM, RADEON_GEM_DOMAIN_VRAM);

            // Write 64-bit timestamp.
            std::uint64_t address2 = 48;
            cs.write({
                PACKET3(PACKET3_EVENT_WRITE_EOP, 4),
                EVENT_TYPE(CACHE_FLUSH_AND_INV_EVENT_TS) | EVENT_INDEX(5),
                std::uint32_t(address2) & ~3u,
                DATA_SEL(3) | INT_SEL(0) | std::uint32_t((address2 >> 32u) & 0xffu),
                0x89abcdef,
                0x01234567
                });
            cs.write_reloc(bo.handle(), RADEON_GEM_DOMAIN_VRAM, RADEON_GEM_DOMAIN_VRAM);
#endif
        }
        std::cout << 4 << std::endl;

        // Emit the command stream.
        cs.emit();
        std::cout << 5 << std::endl;

        // Wait for user input.
        {
            char c;
            std::cin >> c;
        }

        // Map the BO and read back the result.
        {
            std::uint64_t* ptr = static_cast<std::uint64_t*>(bo.mmap(0, 4096));
            std::cout
                << std::hex
                << ptr[0] << '\t'
                << ptr[1] << '\t'
                << ptr[2] << '\t'
                << ptr[3] << '\t'
                << ptr[4] << '\t'
                << ptr[5] << '\t'
                << ptr[6] << '\t'
                << ptr[7] << std::endl;
            bo.munmap();
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

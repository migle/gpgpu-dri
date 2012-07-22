#include <cstring>
#include <iostream>
#include <iomanip>
#include <system_error>

#include "radeon_device.hpp"
#include "radeon_buffer_object.hpp"
#include "radeon_command_stream.hpp"
#include "hex_dump.hpp"

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
        std::cout << "Opening " << argv[1] << std::endl;
        radeon_device dev(argv[1], false);
        // Create a single, small BO for communication with the GPU.
        std::uint32_t bo_size = 4 << 10;
        radeon_buffer_object bo(dev, bo_size, RADEON_GEM_DOMAIN_VRAM);
        std::uint32_t bo_name = bo.flink();
        std::cout << "BO handle = " << bo.handle() << " size = " << bo_size << " name = " << bo_name << std::endl;
        // Map the BO and initialize it to zero.
        {
            void* ptr = bo.mmap(0, bo_size);
            std::memset(ptr, 0, bo_size);
            bo.munmap();
            bo.wait_idle();
        }
        std::cout << "BO set to zero" << std::endl;

        // Create the CS for the GPU.
        radeon_command_stream cs(dev);
        std::cout << "CS id = " << cs.id() << std::endl;

        /// According to AMD Radeon R6xx/R7xx Acceleration document version 1.0
        /// section 4.4.1 on page 21:
        ///     Driver can know that submitted PM4 packets have been fetched by
        ///     CP. One of the ways is (fence/time stamp write back to scratch
        ///     register) which is often used to confirm some packets have been
        ///     processed by CP.

        // Fence, write 32-bit data.
        cs.write({
            PACKET3(PACKET3_EVENT_WRITE_EOP, 4),
            EVENT_TYPE(CACHE_FLUSH_AND_INV_EVENT) | EVENT_INDEX(5),
            0u & ~0x3u, // lower 32 bits of address
            DATA_SEL(1) | INT_SEL(0) | ((0ul >> 32) & 0xffu), // upper 32-39
            0x89abcdef,
            0x01234567
            });
        //cs.write_reloc(bo.handle());
        cs.write_reloc(bo.handle(), 0, RADEON_GEM_DOMAIN_VRAM);

#if 0
        // Fence, write 64-bit data.
        cs.write({
            PACKET3(PACKET3_EVENT_WRITE_EOP, 4),
            EVENT_TYPE(CACHE_FLUSH_AND_INV_EVENT) | EVENT_INDEX(5),
            16u & ~0x3u, // lower 32 bits of address
            DATA_SEL(2) | INT_SEL(0) | ((16ul >> 32) & 0xffu), // upper 32-39
            0x89abcdef,
            0x01234567
            });
        //cs.write_reloc(bo.handle());
        cs.write_reloc(bo.handle(), RADEON_GEM_DOMAIN_VRAM, RADEON_GEM_DOMAIN_VRAM);

        // Write 64-bit timestamp.
        cs.write({
            PACKET3(PACKET3_EVENT_WRITE_EOP, 4),
            EVENT_TYPE(CACHE_FLUSH_AND_INV_EVENT_TS) | EVENT_INDEX(5),
            24u & ~0x3u, // lower 32 bits of address
            DATA_SEL(3) | INT_SEL(0) | ((24ul >> 32) & 0xffu), // upper 32-39
            0x89abcdef,
            0x01234567
            });
        //cs.write_reloc(bo.handle());
        cs.write_reloc(bo.handle(), RADEON_GEM_DOMAIN_VRAM, RADEON_GEM_DOMAIN_VRAM);
#endif

        // Dump the command stream.
        //std::cout << "CS dump:\n" << cs << std::endl;

        // Emit the command stream.
        cs.emit();
        std::cout << "CS emitted" << std::endl;

        // Wait while the BO is busy.
        //bo.wait_idle();
        //while (bo.busy()) {
        //    std::cout << '.' << std::flush;
        //    sleep(1);
        //}
        //std::cout << "\nBO is idle" << std::endl;

        // Wait for user input
        //{ char c; std::cin >> c; }

        // Map the BO and read back the result.
        {
            std::uint64_t* ptr = static_cast<std::uint64_t*>(bo.mmap(0, 4096));
            std::cout << "BO dump:\n"
                << hex_dump<uint64_t>(ptr, 32, 4)
                << std::endl;
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

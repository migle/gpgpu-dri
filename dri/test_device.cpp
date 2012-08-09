#include <cstring>
#include <iostream>
#include <system_error>

#include "radeon_device.hpp"

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

        drm_radeon_gem_info info = dev.gem_info();
        std::cout <<
            "GART size = " << info.gart_size << " "
            "VRAM size = " << info.vram_size << " "
            "VRAM visible = " << info.vram_visible << std::endl;

        std::uint64_t device_id = dev.device_id();
        std::cout <<
            "This is a " << dev.family_name() << " chip with "
            "PCI device id = " << std::hex << device_id << std::endl;

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

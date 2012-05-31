#include <iostream>
#include <system_error>

#include <xf86drm.h>

#include "radeon_state.hpp"

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
        radeon_state(argv[1], false);
    }
    catch (std::system_error& e) {
        drmError(e.code().value(), e.what());
        return 1;
    }
    return 0;
}

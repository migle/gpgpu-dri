#include <stdexcept>
#include <system_error>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <xf86drm.h>

#include "radeon_state.hpp"

using namespace std;

radeon_state::radeon_state(const char* device, bool exclusive) : fd(-1)
{
    open(device, exclusive);
    try {
        get_gem_info();
    }
    catch (exception& e) {
        cleanup();
        close();
        throw e;
    }
}

radeon_state::~radeon_state()
{
    cleanup();
    if (fd != -1) close();
}

void radeon_state::open(const char* device, bool exclusive)
{
    int r = ::open(device, O_RDWR, 0);
    if (r == -1)
        throw system_error(error_code(errno, system_category()), device);
    else
        fd = r;
}

void radeon_state::close()
{
    int r = drmClose(fd);
    if (r != 0) throw system_error(error_code(errno, system_category()), "close");
    fd = -1;
}

void radeon_state::get_gem_info()
{
    int r = drmCommandWriteRead(fd, DRM_RADEON_GEM_INFO, &gem_info, sizeof(gem_info));
    if (r != 0) throw system_error(error_code(errno, system_category()), "GEM info");
}

void radeon_state::init()
{
    // Create a buffer object manager.
    bom = radeon_bo_manager_gem_ctor(fd);
    if (!bom) throw bad_alloc();

    // Create a command stream manager.
    csm = radeon_cs_manager_gem_ctor(fd);
    if (!csm) throw bad_alloc();
}

void radeon_state::cleanup()
{
    // Destroy the CSM if we got to build it.
    if (csm) radeon_cs_manager_gem_dtor(csm), csm = 0;

    // Destroy the BOM if we got to build it.
    if (bom) radeon_bo_manager_gem_dtor(bom), bom = 0;
}


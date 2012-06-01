#include "radeon_device.hpp"

#include <stdexcept>
#include <system_error>

using namespace std;

radeon_device::radeon_device(const char* path, bool exclusive)
    : dri_device(path)
{
}

#if 0
void radeon_device::get_gem_info()
{
    int r = drmCommandWriteRead(fd, DRM_RADEON_GEM_INFO, &gem_info, sizeof(gem_info));
    if (r != 0) throw system_error(error_code(errno, system_category()), "GEM info");
}

void radeon_device::init()
{
    // Create a buffer object manager.
    bom = radeon_bo_manager_gem_ctor(fd);
    if (!bom) throw bad_alloc();

    // Create a command stream manager.
    csm = radeon_cs_manager_gem_ctor(fd);
    if (!csm) throw bad_alloc();
}

void radeon_device::cleanup()
{
    // Destroy the CSM if we got to build it.
    if (csm) radeon_cs_manager_gem_dtor(csm), csm = 0;

    // Destroy the BOM if we got to build it.
    if (bom) radeon_bo_manager_gem_dtor(bom), bom = 0;
}
#endif

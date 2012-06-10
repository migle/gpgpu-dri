#include "radeon_device.hpp"

#include <cstring>
#include <system_error>

#include <sys/ioctl.h>

using namespace std;

radeon_device::radeon_device(const char* path, bool exclusive)
    : dri_device(path)
{
    _gem_info = get_gem_info();
}

drm_radeon_gem_info radeon_device::get_gem_info() const
{
    /// This member function uses DRM_IOCTL_RADEON_GEM_INFO.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_radeon_gem_info args;
    memset(&args, 0, sizeof(args));

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_GEM_INFO, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_GEM_INFO");

    return args;
}

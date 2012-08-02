#include "radeon_device.hpp"

#include <cstring>
#include <iostream>
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

    cout <<
        "GART size = " << args.gart_size << " "
        "VRAM size = " << args.vram_size << " "
        "VRAM visible = " << args.vram_visible << endl;

    return args;
}

void radeon_device::cp_start() const
{
    /// This member function uses DRM_IOCTL_RADEON_CP_START.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_CP_START, 0);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_CP_START");
}

void radeon_device::cp_reset() const
{
    /// This member function uses DRM_IOCTL_RADEON_CP_RESET.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_CP_RESET, 0);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_CP_RESET");
}

void radeon_device::cp_idle() const
{
    /// This member function uses DRM_IOCTL_RADEON_CP_IDLE.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_CP_IDLE, 0);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_CP_IDLE");
}

void radeon_device::cp_resume() const
{
    /// This member function uses DRM_IOCTL_RADEON_CP_RESUME.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_CP_RESUME, 0);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_CP_RESUME");
}

#include "gem_buffer_object.hpp"

#include <cstring>
#include <system_error>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm.h>

using namespace std;

gem_buffer_object::gem_buffer_object(dri_device const& device)
    : _device(device), _size(), _map_addr(), _map_size(), _handle()
    {}

gem_buffer_object::gem_buffer_object(dri_device const& dev, uint32_t name)
    : _device(dev), _size(), _map_addr(), _map_size(), _handle()
{
    /// This constructor is implemented using DRM_IOCTL_GEM_OPEN.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_gem_open args;
    memset(&args, 0, sizeof(args));

    args.name = name;

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_GEM_OPEN, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_GEM_OPEN");

    _handle = args.handle;
    _size = args.size;
}

gem_buffer_object::~gem_buffer_object()
{
    /// If the buffer object is mapped in the local address space, then it is
    /// unmapped as if by a call to the munmap member function.
    if (_map_addr) munmap();

    /// The destructor is implemented using DRM_IOCTL_GEM_CLOSE.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_gem_close args;
    memset(&args, 0, sizeof(args));

    args.handle = handle();

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_GEM_CLOSE, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_GEM_CLOSE");
}

uint32_t gem_buffer_object::flink() const
{
    /// This member function uses DRM_IOCTL_GEM_FLINK.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_gem_flink args;
    memset(&args, 0, sizeof(args));

    args.handle = handle();

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_GEM_FLINK, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_GEM_FLINK");

    return args.name;
}

void gem_buffer_object::munmap()
{
    /// This member function uses munmap.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by munmap.
    int r = ::munmap(_map_addr, _map_size);
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "munmap");

    _map_addr = 0;
    _map_size = 0;
}

#include "radeon_gem_buffer_object.hpp"

#include <cstring>
#include <iostream>
#include <system_error>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm.h>
#include <radeon_drm.h>

using namespace std;

radeon_gem_buffer_object::radeon_gem_buffer_object(
        radeon_device const& dev,
        uint64_t size,
        uint64_t alignment,
        uint32_t domains,
        uint32_t flags)
    : gem_buffer_object(dev)
{
    /// This constructor is implemented using DRM_IOCTL_RADEON_GEM_CREATE.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_radeon_gem_create args;
    memset(&args, 0, sizeof(args));

    args.size = size;
    args.alignment = alignment;
    args.initial_domain = domains;
    args.flags = flags;

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_RADEON_GEM_CREATE, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_GEM_CREATE");

    _handle = args.handle;
    _size = args.size;
}

void* radeon_gem_buffer_object::mmap(uint64_t offset, uint64_t size)
{
    /// This member function uses DRM_IOCTL_RADEON_GEM_MMAP and mmap.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by either of these system calls.
    drm_radeon_gem_mmap args;
    memset(&args, 0, sizeof(args));

    std::cout << offset << std::endl;
    args.handle = _handle;
    args.offset = offset;
    args.size = size;

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_RADEON_GEM_MMAP, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_GEM_MMAP");

    void* addr = ::mmap(0, args.size, PROT_READ|PROT_WRITE, MAP_SHARED, device().descriptor(), args.addr_ptr);
    if (addr == MAP_FAILED)
        throw system_error(error_code(errno, system_category()), "mmap");

    _map_addr = addr;
    _map_size = size;

    return _map_addr;
}

#if 0
void radeon_gem_buffer_object::set_domain(uint32_t read_domains, uint32_t write_domain)
{
    /// This member function uses DRM_IOCTL_RADEON_GEM_SET_DOMAIN.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_radeon_gem_set_domain args;
    memset(&args, 0, sizeof(args));

    args.handle = _handle;
    args.read_domains = read_domains;
    args.write_domain = write_domain;

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_RADEON_GEM_SET_DOMAIN, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_GEM_SET_DOMAIN");
}
#endif

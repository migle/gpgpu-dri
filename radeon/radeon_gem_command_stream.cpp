#include "radeon_gem_command_stream.hpp"

#include <cstring>
#include <iostream>
#include <system_error>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm.h>
#include <radeon_drm.h>

using namespace std;

std::atomic<uint32_t> radeon_gem_command_stream::_unused_id;

uint32_t radeon_gem_command_stream::new_id()
{
    return _unused_id++;
}

radeon_gem_command_stream::radeon_gem_command_stream(
        radeon_device const& dev)
    : gem_command_stream(dev), _id(new_id())
{
}

void radeon_gem_command_stream::emit()
{
    // Two chunks: the instruction buffer and the relocations.
    drm_radeon_cs_chunk chunks[2];

    uint32_t* ib_base = &_ib.front();
    chunks[0].chunk_id = RADEON_CHUNK_ID_IB;
    chunks[0].length_dw = _ib.size();
    chunks[0].chunk_data = reinterpret_cast<uintptr_t>(ib_base);
    chunks[1].chunk_id = RADEON_CHUNK_ID_RELOCS;
    chunks[1].length_dw = _ib.size();
    chunks[1].chunk_data = reinterpret_cast<uintptr_t>(ib_base);

    // The pointers to these two chunks are sent indirectly.
    uint64_t chunk_array[2];

    chunk_array[0] = reinterpret_cast<uintptr_t>(&chunks[0]);
    chunk_array[1] = reinterpret_cast<uintptr_t>(&chunks[1]);

    // Finally, fill in the arguments for the ioctl.
    drm_radeon_cs args;
    memset(&args, 0, sizeof(args));

    args.num_chunks = 2;
    args.cs_id = _id;
    args.chunks = reinterpret_cast<uintptr_t>(chunk_array);

    /// This member function uses DRM_IOCTL_RADEON_CS.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(device().descriptor(), DRM_IOCTL_RADEON_CS, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_CS");

}

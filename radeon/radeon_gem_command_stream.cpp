#include "radeon_gem_command_stream.hpp"

#include <cstring>
#include <system_error>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm.h>
#include <radeon_drm.h>

using namespace std;

std::atomic<uint32_t> radeon_gem_command_stream::_used_id(0);

uint32_t radeon_gem_command_stream::new_id()
{
    for (uint32_t id = 1; id != 0; id <<= 1)
        if ((_used_id.fetch_or(id) & id) == 0)
            return id;
    throw std::bad_alloc();
}

void radeon_gem_command_stream::release_id(uint32_t id)
{
    _used_id &= ~id;
}

radeon_gem_command_stream::radeon_gem_command_stream(radeon_device const& dev)
    : gem_command_stream(dev), _id(new_id())
{
}

radeon_gem_command_stream::~radeon_gem_command_stream()
{
    release_id(_id);
}

void radeon_gem_command_stream::emit() const
{
    // Two chunks: the instruction buffer and the relocations.
    drm_radeon_cs_chunk chunks[2];

    chunks[0].chunk_id = RADEON_CHUNK_ID_IB;
    chunks[0].length_dw = _ib.size();
    chunks[0].chunk_data = _ib.empty() ? 0 : reinterpret_cast<uintptr_t>(&_ib.front());
    chunks[1].chunk_id = RADEON_CHUNK_ID_RELOCS;
    chunks[1].length_dw = _relocs.size() * sizeof(drm_radeon_cs_reloc) / sizeof(uint32_t);
    chunks[1].chunk_data = _relocs.empty() ? 0 : reinterpret_cast<uintptr_t>(&_relocs.front());

    // The pointers to these two chunks are with triple indirection (!)
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

void radeon_gem_command_stream::write_reloc(
        gem_buffer_object const& bo,
        uint32_t read_domains,
        uint32_t write_domain,
        uint32_t flags)
{
    /// This function will make sure that a single relocation record is
    /// sent to the kernel for each distinct buffer object.
    std::unordered_map<uint32_t, uint32_t>::iterator
        p = _reloc_map.find(bo.handle());

    if (p == _reloc_map.end())
    {
        drm_radeon_cs_reloc reloc;
        reloc.handle = bo.handle();
        reloc.read_domains = read_domains;
        reloc.write_domain = write_domain;
        reloc.flags = flags;
        _relocs.push_back(reloc);
        p = _reloc_map.insert(make_pair(bo.handle(), uint32_t(_relocs.size() - 1))).first;
    }
    else
    {
        _relocs[p->second].read_domains |= read_domains;
        _relocs[p->second].write_domain |= write_domain;
        _relocs[p->second].flags |= flags;
    }

    write(0xc0001000);
    write(p->second);
}

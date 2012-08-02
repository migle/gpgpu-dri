#include "radeon_command_stream.hpp"

#include <cstring>
#include <system_error>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm.h>
#include <radeon_drm.h>

#include "radeon/r600d.h"

#if !defined(RADEON_CHUNK_ID_FLAGS)
#define RADEON_CHUNK_ID_FLAGS       0x03

#define RADEON_CS_KEEP_TILING_FLAGS 0x01
#define RADEON_CS_USE_VM            0x02

#define RADEON_CS_RING_GFX          0
#define RADEON_CS_RING_COMPUTE      1
#endif

using namespace std;

std::atomic<uint32_t> radeon_command_stream::_used_id(0);

uint32_t radeon_command_stream::new_id()
{
    for (uint32_t id = 1; id != 0; id <<= 1)
        if ((_used_id.fetch_or(id) & id) == 0)
            return id;
    throw std::bad_alloc();
}

void radeon_command_stream::release_id(uint32_t id)
{
    _used_id &= ~id;
}

radeon_command_stream::radeon_command_stream(radeon_device const& dev)
    : gem_command_stream(dev), _id(new_id())
{
    _flags[0] = _flags[1] = 0;
}

radeon_command_stream::~radeon_command_stream()
{
    release_id(_id);
}

void radeon_command_stream::emit() const
{
    // Three chunks: instruction buffer, relocations and flags.
    drm_radeon_cs_chunk chunks[3];

    chunks[0].chunk_id = RADEON_CHUNK_ID_IB;
    chunks[0].length_dw = _ib.size();
    chunks[0].chunk_data = _ib.empty() ? 0 : reinterpret_cast<uintptr_t>(&_ib.front());
    chunks[1].chunk_id = RADEON_CHUNK_ID_RELOCS;
    chunks[1].length_dw = _relocs.size() * reloc_size;
    chunks[1].chunk_data = _relocs.empty() ? 0 : reinterpret_cast<uintptr_t>(&_relocs.front());
    chunks[2].chunk_id = RADEON_CHUNK_ID_FLAGS;
    chunks[2].length_dw = 2;
    chunks[2].chunk_data = reinterpret_cast<uintptr_t>(&_flags[0]);

    // The pointers to these two chunks are with triple indirection (!)
    uint64_t chunk_array[3];

    chunk_array[0] = reinterpret_cast<uintptr_t>(&chunks[0]);
    chunk_array[1] = reinterpret_cast<uintptr_t>(&chunks[1]);
    chunk_array[2] = reinterpret_cast<uintptr_t>(&chunks[2]);

    // Finally, fill in the arguments for the ioctl.
    drm_radeon_cs args;
    memset(&args, 0, sizeof(args));

    args.num_chunks = 2;
    //args.cs_id = _id;
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

void radeon_command_stream::write_reloc(
        std::uint32_t handle,
        std::uint32_t read_domains,
        std::uint32_t write_domain,
        std::uint32_t flags)
{
    /// This function will make sure that a single relocation record is
    /// sent to the kernel for each distinct buffer object.
    std::unordered_map<uint32_t, uint32_t>::iterator
        p = _reloc_map.find(handle);

    if (p == _reloc_map.end())
    {
        p = _reloc_map.insert(make_pair(handle, uint32_t(_relocs.size()))).first;
        drm_radeon_cs_reloc reloc;
        reloc.handle = handle;
        reloc.read_domains = read_domains;
        reloc.write_domain = write_domain;
        reloc.flags = flags;
        _relocs.push_back(reloc);
    }
    else
    {
        _relocs[p->second].read_domains |= read_domains;
        _relocs[p->second].write_domain |= write_domain;
        _relocs[p->second].flags |= flags;
    }

    write({ PACKET3(PACKET3_NOP, 0), p->second * reloc_size });
}

void radeon_command_stream::write_set_reg(std::uint32_t offset, std::uint32_t n)
{
    if (_ib.empty()) reserve(n + 2);

    /// This function will select the appropriate PM4 packet, either a type-3
    /// or a type-0 packet for setting the register, as different sets of
    /// registers require a slightly different command.

    if (offset >= PACKET3_SET_CONFIG_REG_OFFSET && offset < PACKET3_SET_CONFIG_REG_END)
        write({
            PACKET3(PACKET3_SET_CONFIG_REG, n),
            (offset - PACKET3_SET_CONFIG_REG_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_CONTEXT_REG_OFFSET && offset < PACKET3_SET_CONTEXT_REG_END)
        write({
            PACKET3(PACKET3_SET_CONTEXT_REG, n),
            (offset - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_RESOURCE_OFFSET && offset < PACKET3_SET_RESOURCE_END)
        write({
            PACKET3(PACKET3_SET_RESOURCE, n),
            (offset - PACKET3_SET_RESOURCE_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_SAMPLER_OFFSET && offset < PACKET3_SET_SAMPLER_END)
        write({
            PACKET3(PACKET3_SET_SAMPLER, n),
            (offset - PACKET3_SET_SAMPLER_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_CTL_CONST_OFFSET && offset < PACKET3_SET_CTL_CONST_END)
        write({
            PACKET3(PACKET3_SET_CTL_CONST, n),
            (offset - PACKET3_SET_CTL_CONST_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_ALU_CONST_OFFSET && offset < PACKET3_SET_ALU_CONST_END)
        write({
            PACKET3(PACKET3_SET_ALU_CONST, n),
            (offset - PACKET3_SET_ALU_CONST_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_BOOL_CONST_OFFSET && offset < PACKET3_SET_BOOL_CONST_END)
        write({
            PACKET3(PACKET3_SET_BOOL_CONST, n),
            (offset - PACKET3_SET_BOOL_CONST_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_LOOP_CONST_OFFSET && offset < PACKET3_SET_LOOP_CONST_END)
        write({
            PACKET3(PACKET3_SET_LOOP_CONST, n),
            (offset - PACKET3_SET_LOOP_CONST_OFFSET) >> 2
        });
    else
        write({
            PACKET0(offset, (n - 1))
        });
}

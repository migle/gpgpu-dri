#pragma once

#include "gem_buffer_object.hpp"
#include "gem_command_stream.hpp"
#include "radeon_device.hpp"

#include <cstdint>
#include <atomic>
#include <initializer_list>
#include <unordered_map>
#include <vector>

#include <xf86drm.h>
#include <drm.h>
#include <radeon_drm.h>

/// This class wraps an in-memory GEM command stream.
class radeon_gem_command_stream : public gem_command_stream {
public:
    /// This constructor creates a command stream for a radeon device.
    /// This command stream is created in memory and when full, it can be
    /// sent for execution (emited) through the GEM.
    /// \param device The DRI device on which to create the object.
    radeon_gem_command_stream(radeon_device const& device);
    /// The destructure releases resources associated with the command stream.
    ~radeon_gem_command_stream();

    /// Emit the command stream for execution.
    void emit() const;

    /// Append a double word to the end of the instruction buffer.
    /// \param data 32-bit word to append.
    void write(std::uint32_t data)
    {
        if (_ib.empty()) _ib.reserve(0x200);
        _ib.push_back(data);
    }
    /// Append data from an initializer list of double words to the end of the
    /// instruction buffer.
    /// \param data Initializer list of 32-bit words to append.
    void write(std::initializer_list<std::uint32_t> data)
    {
        if (_ib.empty()) _ib.reserve((data.size() + 0x1ffu) & ~0x1ffu);
        _ib.insert(_ib.end(), data.begin(), data.end());
    }
    /// Append a relocation packet to the end of the instruction buffer.
    void write_reloc(
        gem_buffer_object const& bo,
        uint32_t read_domains = 0,
        uint32_t write_domain = 0,
        uint32_t flags = 0);

    /// iostream-like output operator for appending a double word to the end of
    /// the instruction buffer.
    /// \param cs The command stream to append to.
    /// \param dw The double word to append.
    /// \returns The same command stream, cs.
    friend radeon_gem_command_stream&
    operator << (radeon_gem_command_stream& cs, std::uint32_t dw)
    {
        if (_ib.empty()) _ib.reserve(0x200);
        _ib.push_back(data);
        return cs;
    }
    /// iostream-like output operator for appending a list of double words from
    /// an initializer list to the end of the instruction buffer.
    /// \param cs The command stream to write to.
    /// \param ls The list of double words to append.
    /// \returns The same command stream, cs.
    friend radeon_gem_command_stream&
    operator << (radeon_gem_command_stream& cs, std::initializer_list<std::uint32_t> ls)
    {
        if (_ib.empty()) _ib.reserve((data.size() + 0x1ffu) & ~0x1ffu);
        _ib.insert(_ib.end(), data.begin(), data.end());
        return cs;
    }

protected:
    /// This mask allows us to generate a distinct id for each command stream.
    static std::atomic<std::uint32_t> _used_id;
    /// This function generates an unused id for a command stream.
    static std::uint32_t new_id();
    /// This function releases the id of a command stream.
    static void release_id(std::uint32_t);

protected:
    /// The instruction buffer chunk.
    std::vector<std::uint32_t> _ib;
    /// The relocations chunk.
    std::vector<drm_radeon_cs_reloc> _relocs;
    /// Maps an buffer object handle to an index in the relocations chunk.
    std::unordered_map<std::uint32_t,std::uint32_t> _reloc_map;
    /// The unique id of this command stream.
    const std::uint32_t _id;
};

#pragma once

#include "gem_command_stream.hpp"
#include "radeon_device.hpp"
#include "hex_dump.hpp"

#include <cstdint>
#include <atomic>
#include <initializer_list>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <xf86drm.h>
#include <drm.h>
#include <radeon_drm.h>

/// This class wraps an in-memory GEM command stream.
class radeon_command_stream : public gem_command_stream {
public:
    /// This constructor creates a command stream for a radeon device.
    /// This command stream is created in memory and when full, it can be
    /// sent for execution (emited) through the GEM.
    /// \param device The DRI device on which to create the object.
    radeon_command_stream(radeon_device const& device);
    /// The destructure releases resources associated with the command stream.
    ~radeon_command_stream();

    /// This function returns the DRI device on which this object exists.
    radeon_device const& device() const { return _device; }

    /// Access the id associated with this command stream.
    std::uint32_t id() const { return _id; }

    /// Emit the command stream for execution.
    void emit() const;

    /// Get the current capacity of the instruction buffer.
    /// \returns The capacity in number of double words of the IB.
    std::size_t capacity() const { return _ib.capacity(); }
    /// Get the current size of the instruction buffer.
    /// \returns The number of double words in the IB.
    std::size_t size() const { return _ib.size(); }
    /// Reserve storage in the instruction buffer.
    /// Calling this function is unnecessary, it is an optimization.
    /// \param n The least number of double words for which to reserve storage.
    void reserve(std::size_t n) { _ib.reserve((n + 01777u) & ~01777u); }

    /// Append a double word to the end of the instruction buffer.
    /// \param x The double word to append.
    void write(std::uint32_t x)
    {
        reserve(size() + 1);
        _ib.push_back(x);
    }
    /// Append a float number to the end of the instruction buffer.
    /// \param x The float number to append.
    void write(float x) { write(*reinterpret_cast<std::uint32_t*>(&x)); }
    /// Append a list of double words from an initializer list to the end of
    /// the instruction buffer.
    /// \param x The list of double words to append.
    void write(std::initializer_list<std::uint32_t> x)
    {
        reserve(size() + x.size());
        _ib.insert(_ib.end(), x.begin(), x.end());
    }

    /// Append a relocation packet to the end of the instruction buffer.
    /// \param handle Handle of the buffer object on which the datum is found.
    /// \param read_domains Read domains.
    /// \param write_domain Write domain.
    /// \param flags Flags.
    void write_reloc(
        std::uint32_t handle,
        std::uint32_t read_domains = 0,
        std::uint32_t write_domain = 0,
        std::uint32_t flags = 0);

    /// Dump the CS to an output stream.
    /// \param os Output stream.
    /// \param cs The CS object.
    /// \returns The same output stream passed as \c os.
    friend std::ostream& operator << (std::ostream& os, radeon_command_stream const& cs)
    {
        os << hex_dump<std::uint32_t>(&cs._ib[0], cs._ib.size());
        return os;
    }

private:
    /// The size in double words of the relocation structure.
    static const std::uint32_t reloc_size =
        sizeof(drm_radeon_cs_reloc) / sizeof(std::uint32_t);
    /// This mask allows us to generate a distinct id for each command stream.
    static std::atomic<std::uint32_t> _used_id;
    /// This function generates an unused id for a command stream.
    static std::uint32_t new_id();
    /// This function releases the id of a command stream.
    static void release_id(std::uint32_t);

private:
    /// A const reference to the DRI device wrapper.
    radeon_device const& _device;
    /// The instruction buffer chunk.
    std::vector<std::uint32_t> _ib;
    /// The relocations chunk.
    std::vector<drm_radeon_cs_reloc> _relocs;
    /// The flags chunk.
    std::uint32_t _flags[2];
    /// Maps an buffer object handle to an index in the relocations chunk.
    std::unordered_map<std::uint32_t,std::uint32_t> _reloc_map;
    /// The unique id of this command stream.
    const std::uint32_t _id;
};

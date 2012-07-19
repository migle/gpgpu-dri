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
class radeon_command_stream : public gem_command_stream {
public:
    /// This constructor creates a command stream for a radeon device.
    /// This command stream is created in memory and when full, it can be
    /// sent for execution (emited) through the GEM.
    /// \param device The DRI device on which to create the object.
    radeon_command_stream(radeon_device const& device);
    /// The destructure releases resources associated with the command stream.
    ~radeon_command_stream();

    /// Emit the command stream for execution.
    void emit() const;

    /// Reserve storage in the instruction buffer.
    /// Calling this function is unnecessary, it is an optimization.
    /// \param n The least number of double words for which to reserve storage.
    void reserve(std::size_t n) { _ib.reserve((n + 01777u) & ~01777u); }
    /// Append a double word to the end of the instruction buffer.
    /// \param x The double word to append.
    void write(std::uint32_t x)
    {
        if (_ib.empty()) reserve(1);
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
        if (_ib.empty()) reserve(x.size());
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

    /// Append the header of a PM4 packet for setting a number of registers
    /// starting at the given register offset.
    /// \param offset Register offset of the first register in the series.
    /// \param n Number of registers to set.
    void write_set_reg(std::uint32_t offset, std::uint32_t n);

    /// The structure of the proxy object which is used for conveniently
    /// setting a series of registers.
    ///
    /// This was inspired on Adam Rak's HD-Radeon-Compute project. I considered
    /// other options, such as iostream syntax, but initializer lists cannot be
    /// used in a general expression as they can be on the right side of an
    /// assignment, as function arguments, etc.
    /// Given this, this syntax seems to be the most convenient.
    struct register_setter {
        radeon_command_stream& cs;  ///< Command stream for which to proxy.
        std::uint32_t offset;   ///< Offset of the first register in the series.
        /// Set the register at offset to the given double word.
        void operator = (std::uint32_t x) { cs.write_set_reg(offset, 1); cs.write(x); }
        /// Set the register at offset to the given float number.
        void operator = (float x) { cs.write_set_reg(offset, 1); cs.write(x); }
        /// Set registers starting at offset to the given values.
        /// \param x Double words in an initializer list.
        void operator = (std::initializer_list<std::uint32_t> x)
            { cs.write_set_reg(offset, x.size()); cs.write(x); }
    };
    /// Append a PM4 packet to the instruction buffer that sets a series of
    /// consecutive hardware registers starting at the given register offset.
    ///
    /// This member function merely returns the \c register_setter proxy
    /// object that will actually append the packet to the IB.
    ///
    /// \param offset Register offset of the first register in the series.
    /// \returns Proxy \c register_setter object.
    register_setter operator [] (std::uint32_t offset)
        { return { *this, offset }; }

protected:
    /// The size in double words of the relocation structure.
    static const std::uint32_t reloc_size =
        sizeof(drm_radeon_cs_reloc) / sizeof(std::uint32_t);
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
    /// The flags chunk.
    std::uint32_t _flags[2];
    /// Maps an buffer object handle to an index in the relocations chunk.
    std::unordered_map<std::uint32_t,std::uint32_t> _reloc_map;
    /// The unique id of this command stream.
    const std::uint32_t _id;
};

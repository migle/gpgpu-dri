#pragma once

#include "radeon_command_stream.hpp"
#include "radeon_device.hpp"

#include <cstdint>
#include <initializer_list>

/// This class wraps an in-memory command stream for an R600.
class evergreen_command_stream : public radeon_command_stream {
public:
    /// This constructor creates a command stream for a radeon device.
    /// This command stream is created in memory and when full, it can be
    /// sent for execution (emited) through the GEM.
    /// \param device The DRI device on which to create the object.
    evergreen_command_stream(radeon_device const& device);

    /// Append the header of a PM4 packet for setting a number of registers
    /// starting at the given register offset.
    /// \param start The first register in the series.
    /// \param n Number of registers to set.
    void write_set_reg(std::uint32_t start, std::uint32_t n);

    /// The structure of the proxy object which is used for conveniently
    /// setting a series of registers.
    ///
    /// This was inspired on Adam Rak's HD-Radeon-Compute project. I considered
    /// other options, such as iostream syntax, but initializer lists cannot be
    /// used in a general expression as they can be on the right side of an
    /// assignment, as function arguments, etc.
    /// Given this, this syntax seems to be the most convenient.
    struct register_setter {
        evergreen_command_stream& cs;   ///< Command stream for which to proxy.
        std::uint32_t start;            ///< First register in the series.
        /// Set the register at \c start to the given double word.
        void operator = (std::uint32_t x) { cs.write_set_reg(start, 1); cs.write(x); }
        /// Set the register at \c start to the given float number.
        void operator = (float x) { cs.write_set_reg(start, 1); cs.write(x); }
        /// Set registers starting at \c start to the given values.
        /// \param x Double words in an initializer list.
        void operator = (std::initializer_list<std::uint32_t> x)
            { cs.write_set_reg(start, x.size()); cs.write(x); }
    };
    /// Append a PM4 packet to the instruction buffer that sets a series of
    /// consecutive hardware registers starting at the given register offset.
    ///
    /// This member function merely returns the \c register_setter proxy
    /// object that will actually append the packet to the IB.
    ///
    /// \param start The first register in the series.
    /// \returns Proxy \c register_setter object.
    register_setter operator [] (std::uint32_t start)
        { return { *this, start }; }

    /// Initialize a command stream.
    void start_3d();

    /// Compute shader dispatch.
    void dispatch_direct(std::vector<unsigned int> group_dims,
        std::vector<unsigned int> grid_dims);

    void set_gds(std::uint32_t addr, std::uint32_t size);
    void set_export(std::uint32_t handle, std::uint32_t offset, std::uint32_t size);
};

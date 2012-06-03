#pragma once

#include "gem_command_stream.hpp"
#include "radeon_device.hpp"

#include <atomic>
#include <cstdint>
#include <vector>

#include <xf86drm.h>
#include <drm.h>
#include <radeon_drm.h>

/// This class wraps a handle to a GEM buffer object of the radeon driver.
class radeon_gem_command_stream : public gem_command_stream {
public:
    /// This constructor creates a GEM buffer object on a radeon device.
    /// \param device The DRI device on which to create the object.
    /// \param size The BO size in bytes.
    /// \param alignment The BO alignment in bytes.
    /// \param domains The initial read/write domains of the BO.
    /// \param flags The BO creation flags.
    radeon_gem_command_stream(
        radeon_device const& device);

    void emit();

    /// Append a double word to the end of the instruction buffer.
    /// \param data 32-bit word to append.
    void write(uint32_t data) { _ib.push_back(data); }

    /// Append data to the end of the instruction buffer.
    /// The size of the data is measured in number of 32-bit words.
    /// \param p Pointer to the data to append to the IB.
    /// \param n Number of double words to append.
    void write(void* p, std::size_t n)
    {
        uint32_t* ptr = static_cast<uint32_t*>(p);
        _ib.insert(_ib.end(), ptr, ptr + n);
    }

protected:
    /// This allows us to generate a distinct id to each command stream.
    static std::atomic<uint32_t> _unused_id;
    /// Generate a new id for a command stream.
    static uint32_t new_id();

protected:
    /// The relocation chunk.
    std::vector<uint32_t> _relocs;
    /// The instruction buffer chunk.
    std::vector<uint32_t> _ib;
    /// The id of this command stream.
    uint32_t _id;
};

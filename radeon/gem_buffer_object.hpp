#pragma once

#include "dri_device.hpp"

#include <cstddef>
#include <cstdint>

/// This class wraps a handle to a GEM buffer object.
class gem_buffer_object {
public:
    /// The destructor destroys the buffer object.
    ~gem_buffer_object();
    /// This member function associates the buffer object with a global name.
    /// \returns The global name now associated with the object.
    std::uint32_t flink() const;
    /// This member function unmaps the buffer object from the address space
    /// of the calling process.
    void munmap();

    /// This function returns the DRI device on which this object exists.
    dri_device const& device() const { return _device; }
    /// This function returns the GEM handle of this buffer object.
    std::uint32_t handle() const { return _handle; }
    /// This function returns the size of this buffer object.
    std::uint64_t size() const { return _size; }

    /// This function returns the address at which this buffer is mapped.
    void* map_addr() const { return _map_addr; }
    /// This function returns the size of the mapped region.
    std::size_t map_size() const { return _map_size; }

protected:
    /// This constructor is a building block for other constructors.
    /// It associates the buffer object wrapper with a DRI device and
    /// initializes member variables to sane defaults.
    /// \param device The DRI device wrapper to associate with.
    gem_buffer_object(dri_device const& device);
    /// This constructor opens an existing GEM buffer object in the given GPU.
    /// The object is identified by a global name given by a previous call
    /// to flink.
    /// \param device The DRI device on which the object exists.
    /// \param name The global name of the buffer object.
    gem_buffer_object(dri_device const& device, std::uint32_t name);

protected:
    /// A const reference to the DRI device wrapper.
    dri_device const& _device;
    std::uint32_t _handle;      ///< The GEM handle of the buffer object.
    std::uint32_t _pad0;        ///< Padding.
    std::uint64_t _size;        ///< The size of the buffer object.
    void* _map_addr;            ///< The address at which it is mapped.
    std::size_t _map_size;      ///< The size of the mapping.
};

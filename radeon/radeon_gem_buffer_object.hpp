#pragma once

#include "gem_buffer_object.hpp"
#include "radeon_device.hpp"

#include <xf86drm.h>
#include <drm.h>
#include <radeon_drm.h>

/// This class wraps a handle to a GEM buffer object of the radeon driver.
class radeon_gem_buffer_object : public gem_buffer_object {
public:
    /// This constructor creates a GEM buffer object on a radeon device.
    /// \param device The DRI device on which to create the object.
    /// \param size The BO size in bytes.
    /// \param alignment The BO alignment in bytes.
    /// \param domains The initial read/write domains of the BO.
    /// \param flags The BO creation flags.
    radeon_gem_buffer_object(
        radeon_device const& device,
        uint64_t size,
        uint64_t alignment = 0,
        uint32_t domains = RADEON_GEM_DOMAIN_VRAM,
        uint32_t flags = 0);

    /// This function maps the BO into the address space of the calling process.
    /// \param offset The offset into the BO of the region to map.
    /// \param size The size of the region to map.
    /// \returns The address at which the BO was mapped.
    void* mmap(uint64_t offset, uint64_t size);
    /// This function maps the BO into the address space of the calling process.
    /// This member function always maps the BO from the beginning (it seems not
    /// to work otherwise anyway).
    /// \param size The size of the region to map.
    /// \returns The address at which the BO was mapped.
    void* mmap(uint64_t size) { return mmap(0, size); }
    /// This function maps the BO into the address space of the calling process.
    /// This member function always maps the entire BO.
    /// \returns The address at which the BO was mapped.
    void* mmap() { return mmap(0, size()); }

    /// This function waits until the BO is idle.
    void wait_idle();
    /// This function determines the domains in which the BO is in use.
    /// \returns The domains in which the BO is in use.
    uint32_t busy();

    /// This function reads the BO from the given offset and for the given
    /// number of bytes into the buffer pointed to by ptr.
    /// The user-supplied buffer is assumed to be valid for the whole length.
    /// \param offset The offset into the BO to start reading from.
    /// \param size The number of bytes to read.
    /// \param ptr Pointer to the user-supplied buffer.
    void pread(uint64_t offset, uint64_t size, void* ptr);
    /// This function writes the BO from the given offset and for the given
    /// number of bytes from the buffer pointed to by ptr.
    /// The user-supplied buffer is assumed to be valid for the whole length.
    /// \param offset The offset into the BO to start writing to.
    /// \param size The number of bytes to write.
    /// \param ptr Pointer to the user-supplied buffer.
    void pwrite(uint64_t offset, uint64_t size, const void* ptr);
};

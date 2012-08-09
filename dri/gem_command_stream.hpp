#pragma once

#include "dri_device.hpp"

/// This class wraps a handle to a GEM command stream.
class gem_command_stream {
#if 0
public:
    /// This function returns the DRI device on which this object exists.
    dri_device const& device() const { return _device; }

protected:
    /// This constructor is a building block for other constructors.
    /// It associates the command stream wrapper with a DRI device and
    /// initializes member variables to sane defaults.
    /// \param device The DRI device wrapper to associate with.
    gem_command_stream(dri_device const& device);

private:
    /// A const reference to the DRI device wrapper.
    dri_device const& _device;
#endif
};

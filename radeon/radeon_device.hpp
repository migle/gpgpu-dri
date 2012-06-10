#pragma once

#include "dri_device.hpp"

#include <xf86drm.h>
#include <drm.h>
#include <radeon_drm.h>

/// This class wraps a file descriptor for a DRI device of the radeon driver.
class radeon_device : public dri_device {
public:
    /// This constructor opens a radeon device node given its pathname.
    /// \param path Pathname to the device node.
    radeon_device(const char* path, bool exclusive);

    /// Access cached copy of the information on the GEM.
    drm_radeon_gem_info const& gem_info() const { return _gem_info; }

protected:
    /// Obtain information related to the Graphics Execution Manager (GEM).
    /// The GEM is a DRM concept. This structure contains information on
    /// the amount of RAM, visibility and size of the GART.
    drm_radeon_gem_info get_gem_info() const;

private:
    /// Information related to the Graphics Execution Manager (GEM).
    /// The GEM is a DRM concept. This structure contains information on
    /// the amount of RAM, visibility and size of the GART.
    drm_radeon_gem_info _gem_info;
};

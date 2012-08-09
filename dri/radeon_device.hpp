#pragma once

#include "dri_device.hpp"

#include <xf86drm.h>
#include <drm.h>
#include <radeon_drm.h>

#include <cstdint>

/// This class wraps a file descriptor for a DRI device of the radeon driver.
class radeon_device : public dri_device {
public:
    /// This constructor opens a radeon device node given its pathname.
    /// \param path Pathname to the device node.
    radeon_device(const char* path, bool exclusive);

    /// Access cached copy of the information on the GEM.
    drm_radeon_gem_info const& gem_info() const { return _gem_info; }
    /// Access cached device id.
    std::uint32_t device_id() const { return _device_id; }

    /// Enumeration of radeon device families.
    /// This was copy&pasted from libdrm.
    typedef enum {
        CHIP_UNKNOWN,
        CHIP_LEGACY,
        CHIP_RADEON,
        CHIP_RV100,
        CHIP_RS100,    /* U1 (IGP320M) or A3 (IGP320)*/
        CHIP_RV200,
        CHIP_RS200,    /* U2 (IGP330M/340M/350M) or A4 (IGP330/340/345/350), RS250 (IGP 7000) */
        CHIP_R200,
        CHIP_RV250,
        CHIP_RS300,    /* RS300/RS350 */
        CHIP_RV280,
        CHIP_R300,
        CHIP_R350,
        CHIP_RV350,
        CHIP_RV380,    /* RV370/RV380/M22/M24 */
        CHIP_R420,     /* R420/R423/M18 */
        CHIP_RV410,    /* RV410, M26 */
        CHIP_RS400,    /* xpress 200, 200m (RS400) Intel */
        CHIP_RS480,    /* xpress 200, 200m (RS410/480/482/485) AMD */
        CHIP_RV515,    /* rv515 */
        CHIP_R520,    /* r520 */
        CHIP_RV530,    /* rv530 */
        CHIP_R580,    /* r580 */
        CHIP_RV560,   /* rv560 */
        CHIP_RV570,   /* rv570 */
        CHIP_RS600,
        CHIP_RS690,
        CHIP_RS740,
        CHIP_R600,    /* r600 */
        CHIP_RV610,
        CHIP_RV630,
        CHIP_RV670,
        CHIP_RV620,
        CHIP_RV635,
        CHIP_RS780,
        CHIP_RS880,
        CHIP_RV770,   /* r700 */
        CHIP_RV730,
        CHIP_RV710,
        CHIP_RV740,
        CHIP_CEDAR,   /* evergreen */
        CHIP_REDWOOD,
        CHIP_JUNIPER,
        CHIP_CYPRESS,
        CHIP_HEMLOCK,
        CHIP_PALM,
        CHIP_SUMO,
        CHIP_SUMO2,
        CHIP_BARTS,
        CHIP_TURKS,
        CHIP_CAICOS,
        CHIP_CAYMAN,
        CHIP_ARUBA,
        CHIP_TAHITI,
        CHIP_PITCAIRN,
        CHIP_VERDE,
        CHIP_LAST
    } radeon_family;

    /// Get the chip family to which this device belongs.
    radeon_family family() const { return _family; }
    /// Get the device family identification string.
    const char* family_name() const { return get_family_name(_family); }

protected:
    /// Obtain information related to the Graphics Execution Manager (GEM).
    /// The GEM is a DRM concept. This structure contains information on
    /// the amount of RAM, visibility and size of the GART.
    drm_radeon_gem_info get_gem_info() const;

    /// Information request on radeon device.
    /// \param request Information request.
    /// \param value Pointer to the destination value.
    void get_info(std::uint32_t request, void* value) const;

    /// Get the device family corresponding to a given PCI device id.
    /// \param device_id PCI device ID.
    /// \returns The radeon device family.
    static radeon_family get_family(std::uint32_t device_id);
    /// Get a pointer to an internal string containing the name of the
    /// given device family.
    /// \param family Chip family.
    /// \returns The radeon device family name.
    static const char* get_family_name(radeon_family family);

private:
    /// Information related to the Graphics Execution Manager (GEM).
    /// The GEM is a DRM concept. This structure contains information on
    /// the amount of RAM, visibility and size of the GART.
    drm_radeon_gem_info _gem_info;

    /// Cached device id.
    std::uint32_t _device_id;
    /// Cached device family.
    radeon_family _family;
};

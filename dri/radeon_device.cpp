#include "radeon_device.hpp"

#include <cstring>
#include <system_error>

#include <sys/ioctl.h>

using namespace std;

radeon_device::radeon_device(const char* path, bool exclusive)
    : dri_device(path)
{
    _gem_info = get_gem_info();
    get_info(RADEON_INFO_DEVICE_ID, &_device_id);
    _family = get_family(_device_id);
}

drm_radeon_gem_info radeon_device::get_gem_info() const
{
    /// This member function uses DRM_IOCTL_RADEON_GEM_INFO.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_radeon_gem_info args;
    memset(&args, 0, sizeof(args));

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_GEM_INFO, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_GEM_INFO");

    return args;
}

void radeon_device::get_info(std::uint32_t request, void* value) const
{
    /// This member function uses DRM_IOCTL_RADEON_INFO.
    /// It may throw a std::system_error exception wrapping the error returned
    /// by ioctl.
    drm_radeon_info args;
    memset(&args, 0, sizeof(args));

    args.request = request;
    args.value = reinterpret_cast<std::uintptr_t>(value);

    // Don't be put away by a simple EINTR or EAGAIN...
    int r;
    do {
        r = ioctl(descriptor(), DRM_IOCTL_RADEON_INFO, &args);
    } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    if (r == -1)
        throw system_error(error_code(errno, system_category()), "DRM_IOCTL_RADEON_INFO");
}

radeon_device::radeon_family radeon_device::get_family(std::uint32_t device_id)
{
    /// This function is implemented using a big switch, this was mostly copied
    /// from libdrm, radeon/radeon_surface.c.
    /// A macro transforms the contents of r600_pci_ids.h into the case labels.
    switch (device_id) {
#define CHIPSET(pci_id, name, family)   case pci_id:    return CHIP_##family;
#include <r600_pci_ids.h>
#undef CHIPSET
        default:
            return CHIP_UNKNOWN;
    };
}

const char* radeon_device::get_family_name(radeon_family family)
{
    switch (family) {
        default:
        case CHIP_UNKNOWN:  return "UNKNOWN";
        case CHIP_LEGACY:   return "LEGACY";
        case CHIP_RADEON:   return "RADEON";
        case CHIP_RV100:    return "RV100";
        case CHIP_RS100:    return "RS100";
        case CHIP_RV200:    return "RV200";
        case CHIP_RS200:    return "RS200";
        case CHIP_R200:     return "R200";
        case CHIP_RV250:    return "RV250";
        case CHIP_RS300:    return "RS300";
        case CHIP_RV280:    return "RV280";
        case CHIP_R300:     return "R300";
        case CHIP_R350:     return "R350";
        case CHIP_RV350:    return "RV350";
        case CHIP_RV380:    return "RV380";
        case CHIP_R420:     return "R420";
        case CHIP_RV410:    return "RV410";
        case CHIP_RS400:    return "RS400";
        case CHIP_RS480:    return "RS480";
        case CHIP_RV515:    return "RV515";
        case CHIP_R520:     return "R520";
        case CHIP_RV530:    return "RV530";
        case CHIP_R580:     return "R580";
        case CHIP_RV560:    return "RV560";
        case CHIP_RV570:    return "RV570";
        case CHIP_RS600:    return "RS600";
        case CHIP_RS690:    return "RS690";
        case CHIP_RS740:    return "RS740";
        case CHIP_R600:     return "R600";
        case CHIP_RV610:    return "RV610";
        case CHIP_RV630:    return "RV630";
        case CHIP_RV670:    return "RV670";
        case CHIP_RV620:    return "RV620";
        case CHIP_RV635:    return "RV635";
        case CHIP_RS780:    return "RS780";
        case CHIP_RS880:    return "RS880";
        case CHIP_RV770:    return "RV770";
        case CHIP_RV730:    return "RV730";
        case CHIP_RV710:    return "RV710";
        case CHIP_RV740:    return "RV740";
        case CHIP_CEDAR:    return "CEDAR";
        case CHIP_REDWOOD:  return "REDWOOD";
        case CHIP_JUNIPER:  return "JUNIPER";
        case CHIP_CYPRESS:  return "CYPRESS";
        case CHIP_HEMLOCK:  return "HEMLOCK";
        case CHIP_PALM:     return "PALM";
        case CHIP_SUMO:     return "SUMO";
        case CHIP_SUMO2:    return "SUMO2";
        case CHIP_BARTS:    return "BARTS";
        case CHIP_TURKS:    return "TURKS";
        case CHIP_CAICOS:   return "CAICOS";
        case CHIP_CAYMAN:   return "CAYMAN";
        case CHIP_ARUBA:    return "ARUBA";
        case CHIP_TAHITI:   return "TAHITI";
        case CHIP_PITCAIRN: return "PITCAIRN";
        case CHIP_VERDE:    return "VERDE";
    };
}

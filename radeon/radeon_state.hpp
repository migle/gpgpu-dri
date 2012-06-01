#pragma once

/// This class gathers the program variables related to DRM and the GPU.
class radeon_state {
public:
    radeon_state(const char* device, bool exclusive);
    ~radeon_state();

private:
    void open(const char* device, bool exclusive);
    void close();
    void init();

private:
    /// File descriptor for the DRI device.
    int fd;

    /// Information related to the Graphics Execution Manager (GEM).
    /// The GEM is a DRM concept. This structure contains information on
    /// the amount of RAM, visibility and size of the GART.
    drm_radeon_gem_info gem_info;

    /// The buffer object manager (BOM) we will use. The BOM allows creating
    /// buffer objects (BO), mapping, unmapping, etc.
    /// This is a C-style object, it mostly contains pointers to functions and
    /// is reference-counted.
    radeon_bo_manager* bom;

    /// The command stream manager (CSM) we will use. The CSM allows
    /// creating command streams (CS), performing relocations, etc.
    /// This is a C-style object, it mostly contains pointers to functions and
    /// is reference-counted.
    radeon_cs_manager* csm;
};

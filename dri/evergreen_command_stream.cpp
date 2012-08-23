#include "evergreen_command_stream.hpp"

#include <stdexcept>

using namespace std;

#include "radeon/evergreend.h"

#define SPI_THREAD_GROUPING             0x286C8
#define         CS_GROUPING(v)                  (v << 29)
#define         CS_SIMD_EN(a)                   (a << 16)

#define SPI_COMPUTE_INPUT_CNTL          0x286E8
#define         TID_IN_GROUP_ENA                1
#define         TGID_ENA                        2
#define         DISABLE_INDEX_PACK              4

#define SPI_COMPUTE_NUM_THREAD_X        0x286EC
#define SPI_COMPUTE_NUM_THREAD_Y        0x286F0
#define SPI_COMPUTE_NUM_THREAD_Z        0x286F4

//#define SQ_LDS_RESOURCE_MGMT            0x8E2C
#define         NUM_LS_LDS(x)                   ((x) << 16)

#define SQ_LDS_ALLOC                    0x288E8
#define         SQ_LDS_ALLOC_SIZE(x)            ((x) << 0)
#define         SQ_LDS_ALLOC_HS_NUM_WAVES(x)    ((x) << 14)

#define VGT_GS_MODE                     0x28A40
#define         COMPUTE_MODE                    (1 << 14)
#define         PARTIAL_THD_AT_EOI              (1 << 17)

#define VGT_COMPUTE_START_X             0x899C
#define VGT_COMPUTE_START_Y             0x89A0
#define VGT_COMPUTE_START_Z             0x89A4

#define VGT_COMPUTE_THREAD_GROUP_SIZE   0x89AC

#define VGT_DISPATCH_INITIATOR          0x28B74

#define         S_028838_PS_GPRS(x)             (((x) & 0x1F) << 0)
#define         S_028838_VS_GPRS(x)             (((x) & 0x1F) << 5)
#define         S_028838_GS_GPRS(x)             (((x) & 0x1F) << 10)
#define         S_028838_ES_GPRS(x)             (((x) & 0x1F) << 15)
#define         S_028838_HS_GPRS(x)             (((x) & 0x1F) << 20)
#define         S_028838_LS_GPRS(x)             (((x) & 0x1F) << 25)

#define SX_MEMORY_EXPORT_SIZE           0x9014

evergreen_command_stream::evergreen_command_stream(radeon_device const& device)
    : radeon_command_stream(device)
{
    if (device.family() < radeon_device::CHIP_CEDAR ||
        device.family() >= radeon_device::CHIP_CAYMAN)
        throw runtime_error(device.family_name());
}

void evergreen_command_stream::write_set_reg(std::uint32_t start, std::uint32_t n)
{
    reserve(size() + 2 + n);

    /// This function will select the appropriate PM4 packet, either a type-3
    /// or a type-0 packet for setting the register, as different sets of
    /// registers require a slightly different command.

    if (start >= PACKET3_SET_CONFIG_REG_START && start < PACKET3_SET_CONFIG_REG_END)
        write({
            PACKET3(PACKET3_SET_CONFIG_REG, n),
            (start - PACKET3_SET_CONFIG_REG_START) >> 2
        });
    else if (start >= PACKET3_SET_CONTEXT_REG_START && start < PACKET3_SET_CONTEXT_REG_END)
        write({
            PACKET3(PACKET3_SET_CONTEXT_REG, n),
            (start - PACKET3_SET_CONTEXT_REG_START) >> 2
        });
    else if (start >= PACKET3_SET_BOOL_CONST_START && start < PACKET3_SET_BOOL_CONST_END)
        write({
            PACKET3(PACKET3_SET_BOOL_CONST, n),
            (start - PACKET3_SET_BOOL_CONST_START) >> 2
        });
    else if (start >= PACKET3_SET_LOOP_CONST_START && start < PACKET3_SET_LOOP_CONST_END)
        write({
            PACKET3(PACKET3_SET_LOOP_CONST, n),
            (start - PACKET3_SET_LOOP_CONST_START) >> 2
        });
    else if (start >= PACKET3_SET_RESOURCE_START && start < PACKET3_SET_RESOURCE_END)
        write({
            PACKET3(PACKET3_SET_RESOURCE, n),
            (start - PACKET3_SET_RESOURCE_START) >> 2
        });
    else if (start >= PACKET3_SET_SAMPLER_START && start < PACKET3_SET_SAMPLER_END)
        write({
            PACKET3(PACKET3_SET_SAMPLER, n),
            (start - PACKET3_SET_SAMPLER_START) >> 2
        });
    else if (start >= PACKET3_SET_CTL_CONST_START && start < PACKET3_SET_CTL_CONST_END)
        write({
            PACKET3(PACKET3_SET_CTL_CONST, n),
            (start - PACKET3_SET_CTL_CONST_START) >> 2
        });
    else
        write({
            PACKET0(start, (n - 1))
        });
}

void evergreen_command_stream::start_3d()
{
    write({
        PACKET3(PACKET3_CONTEXT_CONTROL, 1),
        0x80000000,
        0x80000000 });
}

void evergreen_command_stream::set_gds(std::uint32_t addr, std::uint32_t size)
{
    (*this)[GDS_ADDR_BASE] = {
        addr,
        size,
        1
    };
}

void evergreen_command_stream::set_export(std::uint32_t handle, std::uint32_t offset, std::uint32_t size)
{
    if (size)
    {
        (*this)[SX_MEMORY_EXPORT_BASE] = offset;
        write_reloc(handle, 0, RADEON_GEM_DOMAIN_VRAM);
    }

    (*this)[SX_MEMORY_EXPORT_SIZE] = size;
}

//void evergreen_command_stream::set_loop_consts(std::vector<loop_const> const& v)
//{
//}

void evergreen_command_stream::dispatch_direct(
        std::vector< unsigned int > group_dims, std::vector< unsigned int > grid_dims)
{
    // Iterator type for the work group and grid dimensions.
    typedef std::vector< unsigned int >::const_iterator dims_iterator;

    // Compute the total number of work items in a work group.
    unsigned int group_size = 1;
    for (dims_iterator p = group_dims.begin(); p != group_dims.end(); ++p)
        group_size *= *p;

    // Compute the total number of work groups in the domain.
    unsigned int grid_size = 1;
    for (dims_iterator p = grid_dims.begin(); p != grid_dims.end(); ++p)
        grid_size *= *p;

    // Compute the number of wavefronts per work group.
    const unsigned int items_per_wave = 64; // fix, see mesa evergreen_compute.c
    const unsigned int waves_per_group =
        (group_size + items_per_wave - 1) / items_per_wave;

    // This follows evergreen_emit_direct_dispatch almost exactly.
    (*this)[VGT_NUM_INDICES] = group_size;

    (*this)[VGT_COMPUTE_START_X] = {
        0,  // X
        0,  // Y
        0   // Z
    };

    (*this)[VGT_COMPUTE_THREAD_GROUP_SIZE] = group_size;

    (*this)[SPI_COMPUTE_NUM_THREAD_X] = {
        group_dims.size() >= 1 ? group_dims[0] : 1,
        group_dims.size() >= 2 ? group_dims[1] : 1,
        group_dims.size() >= 3 ? group_dims[2] : 1
    };

    unsigned int lds_dwords = 0, lds_size = 0;
    (*this)[SQ_LDS_RESOURCE_MGMT] = NUM_LS_LDS(lds_dwords);
    (*this)[SQ_LDS_ALLOC] = SQ_LDS_ALLOC_SIZE(lds_size) | SQ_LDS_ALLOC_HS_NUM_WAVES(waves_per_group);

    write({
        PACKET3(PACKET3_DISPATCH_DIRECT, 3),
        grid_dims.size() >= 1 ? grid_dims[0] : 1,
        grid_dims.size() >= 2 ? grid_dims[1] : 1,
        grid_dims.size() >= 3 ? grid_dims[2] : 1,
        1 // VGT_DISPATCH_INITIATOR = COMPUTE_SHADER_EN
        });
}

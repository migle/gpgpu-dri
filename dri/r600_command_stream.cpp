#include "r600_command_stream.hpp"

#include "radeon/r600d.h"

#include <stdexcept>

using namespace std;

r600_command_stream::r600_command_stream(radeon_device const& device)
    : radeon_command_stream(device)
{
    if (device.family() < radeon_device::CHIP_R600 ||
        device.family() >= radeon_device::CHIP_CEDAR)
        throw runtime_error(device.family_name());
}

void r600_command_stream::write_set_reg(std::uint32_t offset, std::uint32_t n)
{
    reserve(size() + 2 + n);

    /// This function will select the appropriate PM4 packet, either a type-3
    /// or a type-0 packet for setting the register, as different sets of
    /// registers require a slightly different command.

    if (offset >= PACKET3_SET_CONFIG_REG_OFFSET && offset < PACKET3_SET_CONFIG_REG_END)
        write({
            PACKET3(PACKET3_SET_CONFIG_REG, n),
            (offset - PACKET3_SET_CONFIG_REG_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_CONTEXT_REG_OFFSET && offset < PACKET3_SET_CONTEXT_REG_END)
        write({
            PACKET3(PACKET3_SET_CONTEXT_REG, n),
            (offset - PACKET3_SET_CONTEXT_REG_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_ALU_CONST_OFFSET && offset < PACKET3_SET_ALU_CONST_END)
        write({
            PACKET3(PACKET3_SET_ALU_CONST, n),
            (offset - PACKET3_SET_ALU_CONST_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_BOOL_CONST_OFFSET && offset < PACKET3_SET_BOOL_CONST_END)
        write({
            PACKET3(PACKET3_SET_BOOL_CONST, n),
            (offset - PACKET3_SET_BOOL_CONST_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_LOOP_CONST_OFFSET && offset < PACKET3_SET_LOOP_CONST_END)
        write({
            PACKET3(PACKET3_SET_LOOP_CONST, n),
            (offset - PACKET3_SET_LOOP_CONST_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_RESOURCE_OFFSET && offset < PACKET3_SET_RESOURCE_END)
        write({
            PACKET3(PACKET3_SET_RESOURCE, n),
            (offset - PACKET3_SET_RESOURCE_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_SAMPLER_OFFSET && offset < PACKET3_SET_SAMPLER_END)
        write({
            PACKET3(PACKET3_SET_SAMPLER, n),
            (offset - PACKET3_SET_SAMPLER_OFFSET) >> 2
        });
    else if (offset >= PACKET3_SET_CTL_CONST_OFFSET && offset < PACKET3_SET_CTL_CONST_END)
        write({
            PACKET3(PACKET3_SET_CTL_CONST, n),
            (offset - PACKET3_SET_CTL_CONST_OFFSET) >> 2
        });
    else
        write({
            PACKET0(offset, (n - 1))
        });
}

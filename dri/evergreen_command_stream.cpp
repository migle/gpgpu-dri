#include "evergreen_command_stream.hpp"

#include "radeon/evergreend.h"

#include <stdexcept>

using namespace std;

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

//void evergreen_command_stream::

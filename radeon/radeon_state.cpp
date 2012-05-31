#include <system_error>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <xf86drm.h>

#include "radeon_state.hpp"

radeon_state::radeon_state(const char* device, bool exclusive)
    : fd(0)
{
    open(device, exclusive);
}

radeon_state::~radeon_state()
{
    close();
}

void radeon_state::open(const char* device, bool exclusive)
{
    int r = ::open(device, O_RDWR, 0);
    if (r == -1)
        throw std::system_error(std::error_code(errno, std::system_category()), device);
    else
        fd = r;
}

void radeon_state::close()
{
    if (fd)
    {
        int r = drmClose(fd);
        if (r == -1) throw std::system_error(std::error_code(errno, std::system_category()), "close");
        fd = 0;
    }
}

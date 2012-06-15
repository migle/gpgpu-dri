#include "dri_device.hpp"

#include <iostream>
#include <system_error>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

dri_device::dri_device(const char* path)
    : _fd(-1)
{
    open(path);
}

dri_device::~dri_device() throw()
{
    try {
        if (_fd != -1) close();
    }
    catch (system_error& e) {
        // destructor is no-throw
#if !defined(NDEBUG)
        cerr << e.what()
             << " : "
             << e.code().message()
             << endl;
#endif
    }
}

void dri_device::open(const char* path)
{
    int r = ::open(path, O_RDWR, 0);
    if (r == -1)
        throw system_error(error_code(errno, system_category()), path);

    _fd = r;
}

void dri_device::close()
{
    if (_fd != -1)
    {
        int r = ::close(_fd);
        if (r != 0)
            throw system_error(error_code(errno, system_category()), "close");

        _fd = -1;
    }
}

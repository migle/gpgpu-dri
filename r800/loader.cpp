#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <r800_state.h>
#include <cs_image.h>
#include <evergreen_reg.h>

#include <cassert>
#include <cstring>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <stdexcept>

#include "hex_dump.hpp"

template < typename T >
void loader(r800_state& state, const char* shader, int x, int y, int z, int num_format_all)
{
    compute_shader sh(&state, shader);
    state.set_kms_compute_mode(true);

    assert(x >= 1 && y >= 1 && z >= 1 && sizeof(T) == 4);
    int size = x * y * z;
    int bufsize = size * sizeof(T);
    uint32_t domain = RADEON_GEM_DOMAIN_VRAM;

    std::cerr << "initializing output buffer " << bufsize << " bytes ... " << std::flush;
    radeon_bo* write_buffer = state.bo_open(0, bufsize, 4096, domain, 0);
    radeon_bo_map(write_buffer, 1);
    {
        T* ptr = static_cast<T*>(write_buffer->ptr);
        T* end = ptr + size;
        while (ptr != end)
            *ptr++ = 0xFF; // garbage on the output buffer
    }
    radeon_bo_unmap(write_buffer);
    std::cerr << "done." << std::endl;

    std::cerr << "initializing input buffer " << bufsize << " bytes ... " << std::flush;
    radeon_bo* read_buffer = state.bo_open(0, bufsize, 4096, domain, 0);
    radeon_bo_map(read_buffer, 1);
    {
        T* ptr = static_cast<T*>(read_buffer->ptr);
        for (int k = 0; k != z; ++k)
            for (int j = 0; j != y; ++j)
                for (int i = 0; i != x; ++i)
                    *ptr++ = T(k * 65536 + j * 256 + i); // the input pattern
    }
    radeon_bo_unmap(read_buffer);
    std::cerr << "done." << std::endl;

    std::cerr << "creating VTX and RAT resources ... " << std::flush;
    vtx_resource_t vtxr;
    std::memset(&vtxr, 0, sizeof(vtxr));

    vtxr.id = SQ_FETCH_RESOURCE_cs;
    vtxr.stride_in_dw = 1;
    vtxr.size_in_dw = bufsize / 4;
    vtxr.vb_offset = 0;
    vtxr.bo = read_buffer;
    vtxr.dst_sel_x = SQ_SEL_X;
    vtxr.dst_sel_y = SQ_SEL_Y;
    vtxr.dst_sel_z = SQ_SEL_Z;
    vtxr.dst_sel_w = SQ_SEL_W;
    vtxr.endian = SQ_ENDIAN_NONE;
    vtxr.num_format_all = num_format_all;
    vtxr.format = FMT_32_32_32_32;

    state.set_vtx_resource(&vtxr, domain); ///< for vertex read
    state.set_rat(11, write_buffer, 0, bufsize); ///< For RAT write
    std::cerr << "done." << std::endl;
  
    std::cerr << "initializing the rest of the CS ... " << std::flush;
    state.set_gds(0, 0);
    state.set_tmp_ring(NULL, 0, 0);
    state.set_lds(0, 0, 0);
    state.load_shader(&sh);
    state.direct_dispatch(
        std::initializer_list<int>({ x, y, z }),
        std::initializer_list<int>({ std::max(x, 16), std::max(y, 16), std::max(z, 16) }));
    std::cerr << "done." << std::endl;

    std::cerr << "start kernel of " << x << "x" << y << "x" << z << " ... " << std::flush;
    timespec time1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time1);

    state.flush_cs();
    radeon_bo_wait(write_buffer);

    timespec time2;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time2);
    std::cerr << "done." << std::endl;
  
    long long
        timediff = (time2.tv_sec - time1.tv_sec) * 1000000000ll
                 + (time2.tv_nsec - time1.tv_nsec);

    std::cerr
        << "Execution time GPU: " << timediff << " ns\n"
        << ldexp(bufsize, -30) / (timediff * 1e-9) << " Gbyte/s\n"
        << ldexp(bufsize, 3 - 30) / (timediff * 1e-9) << " Gbit/s"
        << std::endl;
  
    radeon_bo_map(write_buffer, 0);
    {
        std::cout << "Result:\n"
            << hex_dump<uint32_t>(static_cast<uint32_t*>(write_buffer->ptr), size, 4, 6)
            << std::endl;
    }
    radeon_bo_unmap(write_buffer);  
  
    radeon_bo_unref(write_buffer);
    radeon_bo_unref(read_buffer);
  
    std::cerr << "OK" << std::endl;
}

int open_drm(const char* fname)
{
    int fd = open(fname, O_RDWR, 0);
    if (fd == -1) throw std::runtime_error(fname);
    return fd;
}

int main(int argc, char* argv[])
{
    const char *card = "/dev/dri/card0";
    const char *shader = 0;
    int x = 1, y = 1, z = 1;
    bool reset = false;

    for (int i = 1; i < argc; i++) {
        switch (argv[i][0]) {
        case '-':
            if (!std::strcmp(argv[i], "-r") || !std::strcmp(argv[i], "--reset")) {
                reset = true;
                std::cerr << "Resetting card" << std::endl;
                break;
            }
            if (argv[i][1] == 'c') {
                card = &argv[i][2];
                std::cerr << "Use alternative card " << card << std::endl;
                break;
            }
            if (argv[i][1] == 'x') {
                x = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'y') {
                y = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'z') {
                z = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (std::strcmp(argv[i], "-h") && std::strcmp(argv[i], "--help"))
                std::cerr << "Unrecognized option: " << argv[i] << "\n\n";
            std::cerr <<
                "Usage: " << argv[0] << " [-r] [-c<card>] [-x<n>] [-y<n>] [-z<n>] <shader-name>\n"
                "\n"
                "\t-r --reset\treset GPU before starting\n"
                "\t-c/dev/dri/card<n> use alternate card\n"
                "\t-x<n>\tset X dimension\n"
                "\t-y<n>\tset Y dimension\n"
                "\t-z<n>\tset Z dimension\n" << std::endl;
            return 1;
        default:
            shader = argv[i];
            break;
        }
    }

    if (shader)
    {
        std::cerr << "Shader \"" << shader << "\" at " << x << "x" << y << "x" << z << std::endl;

        int drm_fd = open_drm(card);
        r800_state state(drm_fd, reset);
        state.set_default_state();

        loader<int32_t>(state, shader, x, y, z, SQ_NUM_FORMAT_INT);
    }
    return 0;
}

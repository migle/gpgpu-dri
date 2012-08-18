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
void loader(r800_state& state, const char* shader, const int Gx, const int Gy, const int Gz, const int Dx, const int Dy, const int Dz, const int width, const int guard, const int num_format_all)
{
    compute_shader sh(&state, shader);
    std::cerr <<
        "GPRs = " << sh.num_gprs << " stack = " << sh.stack_size << " alloc = " << sh.alloc_size << " "
        "temp GPRs = " << sh.temp_gprs << " global GPRs = " << sh.global_gprs << std::endl;

    state.set_kms_compute_mode(true);

    const int size = Gx * Gy * Gz * Dx * Dy * Dz * width;
    const int safesize = size + guard;
    const int bufsize = safesize * sizeof(T);
    const uint32_t domain = RADEON_GEM_DOMAIN_VRAM;
    std::cerr <<
        "items per group = " << Gx << "x" << Gy << "x" << Gz << "\n"
        "groups total = " << Dx << "x" << Dy << "x" << Dz << "\n"
        "item width = " << width << "\n"
        "size = " << size << " safesize = " << safesize << " bufsize = " << bufsize << std::endl;
    assert(Gx >= 1 && Gy >= 1 && Gz >= 1 && Dx >= 1 && Dy >= 1 && Dz >= 1 && (width == 1 || width == 2 || width == 4) && guard >= 0 && sizeof(T) == 4);

    std::cerr << "initializing output buffer " << bufsize << " bytes ... " << std::flush;
    radeon_bo* write_buffer = state.bo_open(0, bufsize, 4096, domain, 0);
    radeon_bo_map(write_buffer, 1);
    {
        T* ptr = static_cast<T*>(write_buffer->ptr);
        T* end = ptr + size;
        T* limit = ptr + safesize;
        while (ptr != end)
            *ptr++ = 0xff; // garbage on the output buffer
        while (ptr != limit)
            *ptr++ = 0xea; // garbage on the output buffer
    }
    radeon_bo_unmap(write_buffer);
    std::cerr << "done." << std::endl;

    std::cerr << "initializing input buffer " << bufsize << " bytes ... " << std::flush;
    radeon_bo* read_buffer = state.bo_open(0, bufsize, 4096, domain, 0);
    radeon_bo_map(read_buffer, 1);
    {
        T* ptr = static_cast<T*>(read_buffer->ptr);
        T* limit = ptr + safesize;
        for (int k = 0; k != Gz * Dz; ++k)
            for (int j = 0; j != Gy * Dy; ++j)
                for (int i = 0; i != Gx * Dx; ++i)
                    for (int w = 0; w != width; ++w)
                        *ptr++ = T((w + 1) * 0x1000000 + k * 0x10000 + j * 0x100 + i); // the input pattern
        while (ptr != limit)
            *ptr++ = 0xae; // guard pattern
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
        std::initializer_list<int>({ Dx, Dy, Dz }),
        std::initializer_list<int>({ Gx, Gy, Gz }));
    std::cerr << "done." << std::endl;

    std::cerr << "start kernel ... " << std::flush;
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
        << ldexp(bufsize, -30) / (timediff * 1e-9) << " GByte/s\n"
        << ldexp(bufsize, 3 - 30) / (timediff * 1e-9) << " Gbit/s"
        << std::endl;
  
    radeon_bo_map(write_buffer, 0);
    {
        std::cout << "Result:\n"
            << hex_dump<uint32_t>(static_cast<uint32_t*>(write_buffer->ptr), safesize, width == 1 ? 4 : width, 6)
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
    int Gx = 1, Gy = 1, Gz = 1;
    int Dx = 1, Dy = 1, Dz = 1;
    int width = 1, guard = 16;
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
                Gx = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'y') {
                Gy = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'z') {
                Gz = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'X') {
                Dx = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'Y') {
                Dy = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'Z') {
                Dz = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'w') {
                width = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (argv[i][1] == 'G') {
                guard = std::strtol(argv[i] + 2, 0, 10);
                break;
            }
            if (std::strcmp(argv[i], "-h") && std::strcmp(argv[i], "--help"))
                std::cerr << "Unrecognized option: " << argv[i] << "\n\n";
            std::cerr <<
                "Usage: " << argv[0] << " [-r] [-c<card>] [-x<n>] [-y<n>] [-z<n>] [-X<n>] [-Y<n>] [-Z<n>] [-w<n>] [-G<n>] <shader-name>\n"
                "\n"
                "\t-r --reset\treset GPU before starting\n"
                "\t-c/dev/dri/card<n> use alternate card\n"
                "\t-x<n>\tnumber of items per group in X\n"
                "\t-y<n>\tnumber of items per group in Y\n"
                "\t-z<n>\tnumber of items per group in Z\n"
                "\t-X<n>\tnumber of groups in X\n"
                "\t-Y<n>\tnumber of groups in Y\n"
                "\t-Z<n>\tnumber of groups in Z\n"
                "\t-w<n>\tvector width\n"
                "\t-G<n>\tnumber of items of guard\n" << std::endl;
            return 1;
        default:
            shader = argv[i];
            break;
        }
    }

    if (shader)
    {
        std::cerr << "Shader \"" << shader << "\":" << std::endl;

        int drm_fd = open_drm(card);
        r800_state state(drm_fd, reset);
        state.set_default_state();

        loader<int32_t>(state, shader, Gx, Gy, Gz, Dx, Dy, Dz, width, guard, SQ_NUM_FORMAT_INT);
    }
    return 0;
}

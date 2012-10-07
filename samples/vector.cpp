#include <cassert>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <stdexcept>
#include <string>

#include "main.hpp"
#include "hex_dump.hpp"
#include "timespec.hpp"

using namespace std;

void load(r800_state& state, string const& shader, int x, int y, int z, int X, int Y, int Z, int guard, int cols, int addr)
{
    if (x < 1 || y < 1 || z < 1 || X < 1 || Y < 1 || Z < 1 || guard < 0 ||
                 y > 1 || z > 1 ||          Y > 1 || Z > 1)
        throw runtime_error("domain size error");

    compute_shader sh(&state, shader);

    sh.lds_alloc = 0;
    sh.num_gprs = 4;
    sh.temp_gprs = 0;
    sh.global_gprs = 0;
    sh.stack_size = 16;

    state.set_kms_compute_mode(true);

    const int
        g = x * y * z,
        G = X * Y * Z,
        Dx = x * X, Dy = y * Y, Dz = z * Z,
        size = Dx * Dy * Dz,
        wavefront = 64, num_pipes = 2, wave_divisor = 16 * num_pipes,
        num_waves = (g + wave_divisor - 1) / wave_divisor;

    cerr << "Shader \"" << shader << "\"\n"
        "GPRs = " << sh.num_gprs << " temp GPRs = " << sh.temp_gprs << " global GPRs = " << sh.global_gprs << "\n"
        "stack = " << sh.stack_size << " alloc = " << sh.alloc_size << "\n"
        "wavefronts per group = " << num_waves << " using " << num_pipes << " pipes\n\n"
        "items per group = " << x << "x" << y << "x" << z << " = " << g << "\n"
        "number of groups = " << X << "x" << Y << "x" << Z << " = " << G << "\n"
        "domain size = " << Dx << "x" << Dy << "x" << Dz << " = " << size << "\n" << endl;

    const int
        outsize = size * 4,
        outsafe = outsize + guard,
        outbytes = outsafe * sizeof(float);
    cerr << "This shader has one output buffer of 4 32-bit floats per work item.\n"
        "Output size = " << outsize << " floats, " << outsafe << " w/guard, " << outbytes << " bytes.\n";

    cerr << "Initializing output buffer, " << outbytes << " bytes ... " << flush;
    radeon_bo* outbo = state.bo_open(0, outbytes, 4096, RADEON_GEM_DOMAIN_VRAM, 0);
    {
        radeon_bo_map(outbo, 1);
        uint32_t* ptr = static_cast<uint32_t*>(outbo->ptr);
        uint32_t* end = ptr + outsize;
        uint32_t* limit = ptr + outsafe;
        while (ptr != end)
            *ptr++ = 0xffffffff; // NaN
        while (ptr != limit) 
            *ptr++ = 0x7f800000; // +infinity
        radeon_bo_unmap(outbo);
        cerr << "done." << endl;
    }

    cerr << "Mapping output buffer as RAT resource (id=0) ... " << flush;
    state.set_rat(0, outbo, 0, outbytes);
    cerr << "done." << endl;
  
    cerr << "Initializing the constant cache (id=0) ... " << flush;
    radeon_bo* constbo = state.bo_open(0, 16*4, 4096, RADEON_GEM_DOMAIN_VRAM, 0);
    {
        radeon_bo_map(constbo, 1);
        uint32_t* ptr = static_cast<uint32_t*>(constbo->ptr);
        *ptr++ = x, *ptr++ = y, *ptr++ = z, *ptr++ = 0;
        *ptr++ = X, *ptr++ = Y, *ptr++ = Z, *ptr++ = 0;

        float* vptr = reinterpret_cast<float*>(ptr);
        *vptr++ = 0, *vptr++ = 0, *vptr++ = -1, *vptr++ = 0;
        *vptr++ = 1, *vptr++ = 0, *vptr++ = 0, *vptr++ = 0;
        *vptr++ = 0, *vptr++ = -1, *vptr++ = 0, *vptr++ = 0;
        *vptr++ = 0, *vptr++ = 0, *vptr++ = 0, *vptr++ = 1;

        radeon_bo_unmap(constbo);
        state.setup_const_cache(0, constbo, 16, 0);
        cerr << "done." << endl;
    }

    cerr << "Initializing the vertex input buffer (id=1), " << outbytes << " bytes ... " << flush;
    radeon_bo* vbo = state.bo_open(0, outbytes, 4096, RADEON_GEM_DOMAIN_VRAM, 0);
    {
        radeon_bo_map(vbo, 1);
        float* ptr = static_cast<float*>(vbo->ptr);
        float* end = ptr + outsize;
        float m = 0, s = 1;
        while (ptr != end)
            *ptr++ = s * m, m = m + 1, s = -s;
        radeon_bo_unmap(vbo);

        vtx_resource_t vtxr;
        memset(&vtxr, 0, sizeof(vtxr));

        vtxr.id = SQ_FETCH_RESOURCE_cs + 1;
        vtxr.stride_in_dw = 1;
        vtxr.size_in_dw = outsize;
        vtxr.bo = vbo;
        vtxr.dst_sel_x = SQ_SEL_X;
        vtxr.dst_sel_y = SQ_SEL_Y;
        vtxr.dst_sel_z = SQ_SEL_Z;
        vtxr.dst_sel_w = SQ_SEL_W;
        vtxr.endian = SQ_ENDIAN_NONE;
        vtxr.num_format_all = SQ_NUM_FORMAT_NORM;
        vtxr.format = FMT_32_32_32_32_FLOAT;

        state.set_vtx_resource(&vtxr, RADEON_GEM_DOMAIN_VRAM);
        cerr << "done." << endl;
    }

    cerr << "Initializing the rest of the CS ... " << flush;
    state.set_gds(0, 0);
    state.set_tmp_ring(NULL, 0, 0);
    state.set_lds(0, 0, num_waves);
    state.load_shader(&sh);
    state.direct_dispatch(
        initializer_list<int>({ X, Y, Z }),
        initializer_list<int>({ x, y, z }));
    cerr << "done." << endl;

    cerr << "\nExecuting kernel ... " << flush;
    {
        timespec time1;
        clock_gettime(CLOCK_MONOTONIC_RAW, &time1);

        state.flush_cs();
        radeon_bo_wait(outbo);

        timespec time2;
        clock_gettime(CLOCK_MONOTONIC_RAW, &time2);
        cerr << "done.\n" << endl;

        double elapsed = as_seconds(time2 - time1);

        cerr << "Execution time: " << (elapsed * 1e9) << " ns\n"
             << (ldexp(outbytes, -30) / elapsed) << " GByte/s\n"
             << (ldexp(outbytes, 3 - 30) / elapsed) << " Gbit/s\n"
             << endl;
    }
  
    cerr << "Kernel output:" << endl;
    {
#if 0
        radeon_bo_map(vbo, 0);
        {
            float const* p = static_cast<float*>(vbo->ptr);
            for (std::size_t i = 0; i != outsize; ++i, ++p)
            {
                if (i % cols == 0) {
                    if (i != 0) cout << '\n';
                    if (addr != 0) {
                        cout.width(addr);
                        cout << i << ':' << '\t' << flush;
                    }
                }
                else cout << '\t';
                cout.width(8);
                cout << *p;
            }
            cout << endl;
        }
        radeon_bo_unmap(vbo);  
#endif
        radeon_bo_map(outbo, 0);
        {
            float const* p = static_cast<float*>(outbo->ptr);
            for (std::size_t i = 0; i != outsafe; ++i, ++p)
            {
                if (i % cols == 0) {
                    if (i != 0) cout << '\n';
                    if (addr != 0) {
                        cout.width(addr);
                        cout << i << ':' << '\t' << flush;
                    }
                }
                else cout << '\t';
                cout.width(8);
                cout << *p;
            }
            cout << endl;
#if 0
            cout << hex_dump<uint32_t>(static_cast<uint32_t*>(outbo->ptr), outsafe, cols, addr)
                 << endl;
#endif
        }
        radeon_bo_unmap(outbo);  
    }
  
    radeon_bo_unref(vbo);
    radeon_bo_unref(constbo);
    radeon_bo_unref(outbo);
  
    cerr << "OK" << endl;
}

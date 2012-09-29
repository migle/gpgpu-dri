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
    if (x < 1 || y < 1 || z < 1 || X < 1 || Y < 1 || Z < 1 || guard < 0)
        throw runtime_error("domain size error");

    compute_shader sh(&state, shader);

    sh.lds_alloc = 0;
    sh.num_gprs = 4;
    sh.temp_gprs = 0;
    sh.global_gprs = 0;
    sh.stack_size = 16;
    //sh.thread_num = 4;

    state.set_kms_compute_mode(true);

    const int
        g = x * y * z,
        G = X * Y * Z,
        Dx = x * X, Dy = y * Y, Dz = z * Z,
        size = Dx * Dy * Dz;
    cerr << "Shader \"" << shader << "\"\n"
        "GPRs = " << sh.num_gprs << " temp GPRs = " << sh.temp_gprs << " global GPRs = " << sh.global_gprs << "\n"
        "stack = " << sh.stack_size << " alloc = " << sh.alloc_size << "\n\n"
        "items per group = " << x << "x" << y << "x" << z << " = " << g << "\n"
        "number of groups = " << X << "x" << Y << "x" << Z << " = " << G << "\n"
        "domain size = " << Dx << "x" << Dy << "x" << Dz << " = " << size << "\n" << endl;

    const int
        outsize = size * 4,
        outsafe = outsize + guard,
        outbytes = outsafe * sizeof(uint32_t);
    cerr << "This shader has one output buffer of 4 32-bit ints per work item.\n"
        "Output size = " << outsize << " ints, " << outsafe << " w/guard, " << outbytes << " bytes.\n";

    cerr << "Initializing output buffer, " << outbytes << " bytes ... " << flush;
    radeon_bo* outbo = state.bo_open(0, outbytes, 4096, RADEON_GEM_DOMAIN_VRAM, 0);
    {
        radeon_bo_map(outbo, 1);
        uint32_t* ptr = static_cast<uint32_t*>(outbo->ptr);
        uint32_t* end = ptr + outsize;
        uint32_t* limit = ptr + outsafe;
        while (ptr != end)
            *ptr++ = 0xffffffff;
        while (ptr != limit) 
            *ptr++ = 0xeaeaeaea;
        radeon_bo_unmap(outbo);
        cerr << "done." << endl;
    }

    cerr << "Mapping output buffer as RAT resource 11 ... " << flush;
    state.set_rat(11, outbo, 0, outbytes);
    cerr << "done." << endl;
  
    cerr << "Initializing the constant cache ... " << flush;
    radeon_bo* constbo = state.bo_open(0, 1024, 4096, RADEON_GEM_DOMAIN_VRAM, 0);
    {
        radeon_bo_map(constbo, 1);
        uint32_t* ptr = static_cast<uint32_t*>(constbo->ptr);
        *ptr++ = x, *ptr++ = y, *ptr++ = z, *ptr++ = 0;
        *ptr++ = X, *ptr++ = Y, *ptr++ = Z, *ptr++ = 0;
        // domain mapping:
        *ptr++ = 1, *ptr++ = Dx, *ptr++ = Dx*Dy, *ptr++ = 0;
        *ptr++ = x, *ptr++ = Dx*y, *ptr++ = Dx*Dy*z, *ptr++ = 0;
        // more coalescent mapping:
        *ptr++ = 1, *ptr++ = x, *ptr++ = x*y, *ptr++ = 0;
        *ptr++ = g, *ptr++ = X*g, *ptr++ = X*Y*g, *ptr++ = 0;
        radeon_bo_unmap(constbo);
        state.setup_const_cache(0, constbo, 256, 0);
        cerr << "done." << endl;
    }

    cerr << "Initializing the rest of the CS ... " << flush;
    state.set_gds(0, 0);
    state.set_tmp_ring(NULL, 0, 0);
    state.set_lds(0, 0, 0);
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
  
    cerr << "Making GPU timestamps relative ... " << flush;
    {
        radeon_bo_map(outbo, 0);
        uint32_t* beg = static_cast<uint32_t*>(outbo->ptr);
        uint32_t* end = beg + outsize;
        uint32_t min_ts = 0xffffffff;
        for (uint32_t* ptr = beg; ptr != end; ptr += 4)
            if (ptr[3] < min_ts)
                min_ts = ptr[3];
        for (uint32_t* ptr = beg; ptr != end; ptr += 4)
            ptr[3] -= min_ts;
        radeon_bo_unmap(outbo);  
        cerr << "done." << endl;
    }

    cerr << "Kernel output:" << endl;
    {
        radeon_bo_map(outbo, 0);
        cout << hex_dump<uint32_t>(static_cast<uint32_t*>(outbo->ptr), outsafe, cols, addr)
             << endl;
        radeon_bo_unmap(outbo);  
    }
  
    radeon_bo_unref(constbo);
    radeon_bo_unref(outbo);
  
    cerr << "OK" << endl;
}

#pragma once

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <system_error>
#include <string>

#include <r800_state.h>
#include <cs_image.h>
#include <evergreen_reg.h>

using namespace std;

void load(r800_state& state, string const& shader, int x, int y, int z, int X, int Y, int Z, int guard, int cols, int addr);

int main(int argc, char* argv[])
{
    const char *card = "/dev/dri/card0";
    int x = 1, y = 1, z = 1;
    int X = 1, Y = 1, Z = 1;
    int guard = 16, columns = 4, address = 6;
    bool reset = false;

    for (int opt = 0; (opt = getopt(argc, argv, "rc:x:y:z:X:Y:Z:G:w:a:")) != -1; )
        switch (opt) {
            case 'r':
                reset = true;
                break;
            case 'c':
                card = optarg;
                break;
            case 'x':
                x = atoi(optarg);
                break;
            case 'y':
                y = atoi(optarg);
                break;
            case 'z':
                z = atoi(optarg);
                break;
            case 'X':
                X = atoi(optarg);
                break;
            case 'Y':
                Y = atoi(optarg);
                break;
            case 'Z':
                Z = atoi(optarg);
                break;
            case 'G':
                guard = atoi(optarg);
                break;
            case 'w':
                columns = atoi(optarg);
                break;
            case 'a':
                address = atoi(optarg);
                break;
            default:
                cerr << "Usage: " << argv[0] << " [-r] [-c<card>] [-x<n>] [-y<n>] [-z<n>] [-X<n>] [-Y<n>] [-Z<n>] [-G<n>] [-w<n>] [-a<n>]\n\n"
                    "\t-r\treset GPU before starting\n"
                    "\t-c/dev/dri/card<n> use alternate card\n"
                    "\t-x <n>\tnumber of items per group in X (1)\n"
                    "\t-y <n>\tnumber of items per group in Y (1)\n"
                    "\t-z <n>\tnumber of items per group in Z (1)\n"
                    "\t-X <n>\tnumber of groups in X (1)\n"
                    "\t-Y <n>\tnumber of groups in Y (1)\n"
                    "\t-Z <n>\tnumber of groups in Z (1)\n"
                    "\t-G <n>\tbuffer guard size (16)\n"
                    "\t-w <n>\tcolumns in the output (4)\n"
                    "\t-a <n>\taddress columns (6)\n" << endl;
                return EXIT_FAILURE;
        }

    try
    {
        string shader = argv[0];
        shader += ".bin";

        int fd = open(card, O_RDWR, 0);
        if (fd == -1) throw system_error(error_code(errno, system_category()), card);

        r800_state state(fd, reset);
        state.set_default_state();

        load(state, shader, x, y, z, X, Y, Z, guard, columns, address);
    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

ASM_SOURCES = $(wildcard *.asm)
ASM_TARGETS = $(addsuffix .bin, $(basename $(wildcard *.asm)))
CPP_SOURCES = $(wildcard *.cpp)
CPP_TARGETS = $(basename $(wildcard *.cpp))

all : all-asm all-cpp

all-asm : $(ASM_TARGETS)

all-cpp : $(CPP_TARGETS)

clean :
	rm -fv $(ASM_TARGETS)
	rm -fv $(CPP_TARGETS)

%.bin : %.asm
	./as_r800 $^

%: %.cpp
	g++ -std=c++0x -O -I /usr/include/libdrm -I ../watch/rakadam/HD-Radeon-Compute -o $@ $^ libr800_compute.a -ldrm -ldrm_radeon

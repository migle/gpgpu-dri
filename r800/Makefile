
all : loader $(addsuffix .bin, $(basename $(wildcard *.asm)))

clean :
	rm -f *.bin loader

%.bin : %.asm
	./as_r800 $^

loader: loader.cpp
	g++ -std=c++0x -O -I /usr/include/libdrm -I ../watch/rakadam/HD-Radeon-Compute -o $@ $^ libr800_compute.a -ldrm -ldrm_radeon

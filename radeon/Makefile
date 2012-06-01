CXXFLAGS=-std=c++0x -pthread -O3 $(shell pkg-config --cflags libdrm libdrm_radeon)
LDFLAGS=$(shell pkg-config --libs libdrm libdrm_radeon)

#SOURCES=$(wildcard r*.cpp)
#HEADERS=$(wildcard r*.hpp)
SOURCES=dri_device.cpp radeon_device.cpp gem_buffer_object.cpp radeon_gem_buffer_object.cpp
HEADERS=dri_device.hpp radeon_device.hpp gem_buffer_object.hpp radeon_gem_buffer_object.hpp
OBJECTS=$(SOURCES:.cpp=.o)
TARGETS=test

all : $(TARGETS)

clean :
	@rm -vf *.o $(TARGETS)
	@rm -rf html latex

doc :
	doxygen
	$(MAKE) -C latex pdf

%.o : %.cpp $(HEADERS)

test : test.cpp $(OBJECTS)

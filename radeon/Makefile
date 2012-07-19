CXXFLAGS=-std=c++0x -pthread -O3 $(shell pkg-config --cflags libdrm libdrm_radeon)
LDFLAGS=$(shell pkg-config --libs libdrm libdrm_radeon)

#HEADERS=$(wildcard r*.hpp)
#SOURCES=$(wildcard r*.cpp)
HEADERS=dri_device.hpp gem_buffer_object.hpp gem_command_stream.hpp radeon_device.hpp radeon_buffer_object.hpp radeon_command_stream.hpp
SOURCES=dri_device.cpp gem_buffer_object.cpp gem_command_stream.cpp radeon_device.cpp radeon_buffer_object.cpp radeon_command_stream.cpp
OBJECTS=$(SOURCES:.cpp=.o)
TARGETS=test_buffer_object

all : $(TARGETS)

clean :
	@rm -vf *.o $(TARGETS)
	@rm -rf html latex

doc :
	doxygen
	$(MAKE) -C latex pdf

%.o : %.cpp $(HEADERS)

test_buffer_object : test_buffer_object.cpp $(OBJECTS)

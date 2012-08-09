CXXFLAGS=-std=c++0x -pthread -O2 $(shell pkg-config --cflags libdrm libdrm_radeon)
#LDFLAGS=$(shell pkg-config --libs libdrm libdrm_radeon)

#HEADERS=$(wildcard r*.hpp)
#SOURCES=$(wildcard r*.cpp)
HEADERS=dri_device.hpp gem_buffer_object.hpp gem_command_stream.hpp radeon_device.hpp radeon_buffer_object.hpp radeon_command_stream.hpp hex_dump.hpp
SOURCES=dri_device.cpp gem_buffer_object.cpp gem_command_stream.cpp radeon_device.cpp radeon_buffer_object.cpp radeon_command_stream.cpp
OBJECTS=$(SOURCES:.cpp=.o)

LIBS=libdri.a
PROGS=inspect_buffer_object test_device test_buffer_object test_command_stream

all : $(LIBS) $(PROGS)

clean :
	@$(RM) -r html latex
	@$(RM) -v $(LIBS)
	@$(RM) -v $(PROGS) $(PROGS:%=%.o)

doc :
	doxygen
	$(MAKE) -C latex pdf

%.o : %.cpp $(HEADERS)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

(%.o) : %.cpp $(HEADERS)
	$(COMPILE.cpp) $< -o $%
	@$(AR) $(ARFLAGS) $@ $%
	@$(RM) -v $%

libdri.a : libdri.a($(OBJECTS))
	ranlib $@

inspect_buffer_object : inspect_buffer_object.o libdri.a
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

test_device : test_device.o libdri.a
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

test_buffer_object : test_buffer_object.o libdri.a
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

test_command_stream : test_command_stream.o libdri.a
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

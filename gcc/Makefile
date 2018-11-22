
include Makefile.in

.PHONY: clean

CXXFLAGS=-std=c++11 -pedantic -O3 #-Wall
CXXFLAGS+=-I../src/ -I../src/libsasm -I../src/SIDdisasm -I../src/sasmSIDdump \
		-I../src/libsasmemu -I../src/libsasmdisasm -I../src/HueUtil

all: ../bin/SIDdecompiler ../bin/SIDcompare ../bin/sasmSIDdump ../bin/sasm

../bin/SIDdecompiler: $(SIDDISASM_OBJECTS) $(SIDDISASM_HEADERS) $(SIDDISASM_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -o ../bin/SIDdecompiler $(SIDDISASM_OBJECTS) $(LIBSASM_OBJECTS) $(LIBSASMDISASM_OBJECTS) $(LIBSASMEMU_OBJECTS)

../bin/SIDcompare: $(SIDCOMPARE_OBJECTS) $(SIDCOMPARE_HEADERS) $(SIDCOMPARE_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -o ../bin/SIDcompare $(SIDCOMPARE_OBJECTS) $(LIBSASM_OBJECTS) $(LIBSASMDISASM_OBJECTS) $(LIBSASMEMU_OBJECTS)

../bin/sasmSIDdump: $(SASMSIDDUMP_OBJECTS) $(SASMSIDDUMP_HEADERS) $(SASMSIDDUMP_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -o ../bin/sasmSIDdump $(SASMSIDDUMP_OBJECTS) $(LIBSASM_OBJECTS) $(LIBSASMEMU_OBJECTS)
	
../bin/sasm: $(SASM_OBJECTS) $(SASM_HEADERS) $(SASM_DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -o ../bin/sasm $(SASM_OBJECTS) $(LIBSASM_OBJECTS)
	
libsasm.a: $(LIBSASM_OBJECTS) $(LIBSASM_HEADERS)
	$(AR) -r libsasm.a $(LIBSASM_OBJECTS)
libsasmdisasm.a: $(LIBSASMDISASM_OBJECTS) $(LIBSASMDISASM_HEADERS)
	$(AR) -r libsasmdisasm.a $(LIBSASMDISASM_OBJECTS)
libsasmemu.a: $(LIBSASMEMU_OBJECTS) $(LIBSASMEMU_HEADERS)
	$(AR) -r libsasmemu.a $(LIBSASMEMU_OBJECTS)
	
clean:
	$(RM) ../bin/SIDdecompiler
	$(RM) ../bin/SIDcompare
	$(RM) ../bin/sasmSIDdump
	$(RM) ../bin/sasm
	$(RM) $(SIDDISASM_OBJECTS)
	$(RM) $(SIDCOMPARE_OBJECTS)
	$(RM) $(SASMSIDDUMP_OBJECTS)
	$(RM) $(SASM_OBJECTS)
	$(RM) $(LIBSASM_OBJECTS)
	$(RM) $(LIBSASMDISASM_OBJECTS)
	$(RM) $(LIBSASMEMU_OBJECTS)
	$(RM) libsasm.a
	$(RM) libsasmdisasm.a
	$(RM) libsasmemu.a

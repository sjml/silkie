BASE_DIR = $(shell pwd)
export PREFIX = $(BASE_DIR)/third-party
BUILT_DIR = $(BASE_DIR)/build
INTERMEDIATE_DIR = $(BUILT_DIR)/intermediate
SRC_DIR = $(BASE_DIR)/src
LIB_DIR = $(PREFIX)/lib

export MK_JOBS := 4
ifeq ($(shell uname),Darwin)
    export CC := clang
    export CXX := clang++
else
    export CC := gcc
    export CXX := g++
endif
AR := ar rcu
RANLIB := ranlib

export MACOSX_DEPLOYMENT_TARGET = 10.13
export SDKROOT = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk
export OSX_FLAGS = -arch x86_64 -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET) -isysroot $(SDKROOT)

export CFLAGS = -O3
export CXXFLAGS = -O3
ifeq ($(shell uname),Darwin)
    export CFLAGS += $(OSX_FLAGS)
    export CXXFLAGS += $(OSX_FLAGS)
endif

LLVM_VERSION = 4.0.1
MESA_VERSION = 17.1.10
ZLIB_VERSION = 1.2.11
LIBPNG_VERSION = 1.6.35
GLFW_VERSION = 3.2.1
GLU_VERSION = 9.0.0

SILKIE_DEPS = zlib LLVM OSMesa GLU GLFW


ZLIB_LIB = $(LIB_DIR)/libz.a
$(ZLIB_LIB):
	$(BASE_DIR)/build-scripts/build_z.sh $(ZLIB_VERSION)

.PHONY: zlib zlib-clean-build
zlib: $(ZLIB_LIB)
zlib-clean-build:
	rm -rf $(BASE_DIR)/libzips/zlib-$(ZLIB_VERSION)
	rm -rf $(BASE_DIR)/libzips/zlib-$(ZLIB_VERSION).tar.gz


LLVM_LIB = $(LIB_DIR)/libLLVMCore.a
$(LLVM_LIB):
	$(BASE_DIR)/build-scripts/build_llvm.sh $(LLVM_VERSION)

.PHONY: LLVM LLVM-clean-build
LLVM: $(LLVM_LIB)
LLVM-clean-build:
	rm -rf $(BASE_DIR)/libzips/llvm-$(LLVM_VERSION).src
	rm -rf $(BASE_DIR)/libzips/llvm-$(LLVM_VERSION).src.tar.xz


MESA_LIB = $(LIB_DIR)/libOSMesa32.a
$(MESA_LIB): $(LLVM_LIB)
	$(BASE_DIR)/build-scripts/build_osmesa.sh $(MESA_VERSION)

.PHONY: OSMesa OSMesa-clean-build
OSMesa: $(MESA_LIB)
OSMesa-clean-build:
	rm -rf $(BASE_DIR)/libzips/mesa-$(MESA_VERSION)
	rm -rf $(BASE_DIR)/libzips/mesa-$(MESA_VERSION).tar.gz


GLU_LIB = $(LIB_DIR)/libGLU.a
$(GLU_LIB):
	$(BASE_DIR)/build-scripts/build_glu.sh $(GLU_VERSION)

.PHONY: GLU GLU-clean-build
GLU: $(GLU_LIB)
GLU-clean-build:
	rm -rf $(BASE_DIR)/libzips/glu-$(GLU_VERSION)
	rm -rf $(BASE_DIR)/libzips/glu-$(GLU_VERSION).tar.bz2


GLFW_LIB = $(LIB_DIR)/libglfw3.a
$(GLFW_LIB):
	$(BASE_DIR)/build-scripts/build_glfw.sh $(GLFW_VERSION)

.PHONY: GLFW GLFW-clean-build
GLFW: $(GLFW_LIB)
GLFW-clean-build:
	rm -rf $(BASE_DIR)/libzips/glfw-$(GLFW_VERSION)
	rm -rf $(BASE_DIR)/libzips/glfw-$(GLFW_VERSION).zip



SILKIE_SRCS = silkie.c
SILKIE_OBJS = $(patsubst %.c,$(INTERMEDIATE_DIR)/%.o,$(SILKIE_SRCS))
SILKIE_NOBJS = $(patsubst %.c,$(INTERMEDIATE_DIR)/%.n.o,$(SILKIE_SRCS))
SILKIE_DOBJS = $(patsubst %.c,$(INTERMEDIATE_DIR)/%.d.o,$(SILKIE_SRCS))
SILKIE_NDOBJS = $(patsubst %.c,$(INTERMEDIATE_DIR)/%.n.d.o,$(SILKIE_SRCS))
SILKIE_DEBUG_FLAGS = -g -O0
SILKIE_RELEASE_FLAGS = -O3

$(INTERMEDIATE_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(INTERMEDIATE_DIR)
	$(CC) -c $(CFLAGS) -DSILKIE_OFFSCREEN=1 $(SILKIE_RELEASE_FLAGS) -I$(BASE_DIR)/src/lib -I$(PREFIX)/include -o $@ $^

$(INTERMEDIATE_DIR)/%.d.o: $(SRC_DIR)/%.c
	@mkdir -p $(INTERMEDIATE_DIR)
	$(CC) -c $(CFLAGS) -DSILKIE_OFFSCREEN=1 $(SILKIE_DEBUG_FLAGS) -I$(BASE_DIR)/src/lib -I$(PREFIX)/include -o $@ $^

$(INTERMEDIATE_DIR)/%.n.o: $(SRC_DIR)/%.c
	@mkdir -p $(INTERMEDIATE_DIR)
	$(CC) -c $(CFLAGS) -DSILKIE_OFFSCREEN=0 $(SILKIE_RELEASE_FLAGS) -I$(BASE_DIR)/src/lib -I$(PREFIX)/include -o $@ $^

$(INTERMEDIATE_DIR)/%.n.d.o: $(SRC_DIR)/%.c
	@mkdir -p $(INTERMEDIATE_DIR)
	$(CC) -c $(CFLAGS) -DSILKIE_OFFSCREEN=0 $(SILKIE_DEBUG_FLAGS) -I$(BASE_DIR)/src/lib -I$(PREFIX)/include -o $@ $^

$(BUILT_DIR)/libSilkie.a: $(MESA_LIB) $(LIBPNG_LIB) $(SILKIE_OBJS)
	$(AR) $@ $(SILKIE_OBJS)
	$(RANLIB) $@

$(BUILT_DIR)/libSilkieN.a: $(GLFW_LIB) $(SILKIE_NOBJS)
	$(AR) $@ $(SILKIE_NOBJS)
	$(RANLIB) $@

$(BUILT_DIR)/libSilkie-Debug.a: $(MESA_LIB) $(LIBPNG_LIB) $(SILKIE_DOBJS)
	$(AR) $@ $(SILKIE_DOBJS)
	$(RANLIB) $@

$(BUILT_DIR)/libSilkieN-Debug.a: $(GLFW_LIB) $(SILKIE_NDOBJS)
	$(AR) $@ $(SILKIE_NDOBJS)
	$(RANLIB) $@


.PHONY: silkie silkieN silkie-Debug silkieN-Debug debug release silkie-all

silkie: $(BUILT_DIR)/libSilkie.a
silkieN: $(BUILT_DIR)/libSilkieN.a
silkie-Debug: $(BUILT_DIR)/libSilkie-Debug.a
silkieN-Debug: $(BUILT_DIR)/libSilkieN-Debug.a

debug: silkie-Debug silkieN-Debug
release: silkie silkieN

.PHONY: examples
examples:
	cd $(BASE_DIR)/examples && make all

all: | debug release


.PHONY: silkie-clean silkie-clean-deps clean
silkie-clean:
	rm -rf $(BUILT_DIR)/libSilkie* $(INTERMEDIATE_DIR)

SILKIE_CLEAN_TARGETS = $(patsubst %,%-clean-build,$(SILKIE_DEPS))
silkie-clean-deps: $(SILKIE_CLEAN_TARGETS)

clean: silkie-clean-deps silkie-clean
	rm -rf $(PREFIX)/bin
	rm -rf $(PREFIX)/include
	rm -rf $(PREFIX)/lib
	rm -rf $(PREFIX)/share
	rm -rf $(BUILT_DIR)/examples

BASE_DIR = $(shell pwd)/..
PREFIX = $(BASE_DIR)/third-party
LIB_DIR = $(PREFIX)/lib
BUILT_DIR = $(BASE_DIR)/build

MK_JOBS := 4
ifeq ($(shell uname),Darwin)
    export CC := clang
    export CXX := clang++
else
    export CC := gcc
    export CXX := g++
endif
AR := ar rcu
RANLIB := ranlib

ifeq ($(SILKIE_OFFSCREEN),)
    SILKIE_OFFSCREEN = 1
endif

LLVM_LIB_LIST = $(shell $(PREFIX)/bin/llvm-config --ldflags --libs --system-libs --libs engine)

SILKIE_OFFSCREEN_LIBS_RELEASE = -lSilkie -lOSMesa32 -lz -lpng $(LLVM_LIB_LIST)
SILKIE_OFFSCREEN_LIBS_DEBUG = -lSilkie-Debug -lOSMesa32 -lz -lpng $(LLVM_LIB_LIST)
SILKIE_ONSCREEN_LIBS_RELEASE = -lSilkieN -lglfw3
SILKIE_ONSCREEN_LIBS_DEBUG = -lSilkieN-Debug -lglfw3
SILKIE_OFFSCREEN_CFLAGS_RELEASE = -DSILKIE_OFFSCREEN=1 -O3
SILKIE_OFFSCREEN_CFLAGS_DEBUG = -DSILKIE_OFFSCREEN=1 -O0 -g
SILKIE_ONSCREEN_CFLAGS_RELEASE = -DSILKIE_OFFSCREEN=0 -O3
SILKIE_ONSCREEN_CFLAGS_DEBUG = -DSILKIE_OFFSCREEN=0 -O0 -g

ifeq ($(shell uname),Darwin)
	SILKIE_ONSCREEN_LIBS_RELEASE += -framework Cocoa -framework OpenGL -framework CoreVideo -framework IOKit
	SILKIE_ONSCREEN_LIBS_DEBUG += -framework Cocoa -framework OpenGL -framework CoreVideo -framework IOKit
endif

EXAMPLE_LIST = basic animation advanced
EXAMPLE_LIST_D = $(patsubst %,%-Debug,$(EXAMPLE_LIST))
EXAMPLE_EXES = $(addprefix $(BUILT_DIR)/examples/, $(EXAMPLE_LIST))
EXAMPLE_NEXES = $(patsubst %,$(BUILT_DIR)/examples/%-onscreen,$(EXAMPLE_LIST))
EXAMPLE_DEXES = $(patsubst %,$(BUILT_DIR)/examples/%-Debug,$(EXAMPLE_LIST))
EXAMPLE_NDEXES = $(patsubst %,$(BUILT_DIR)/examples/%-onscreen-Debug,$(EXAMPLE_LIST))

.PHONY: $(EXAMPLE_LIST) $(EXAMPLE_LIST_D) all clean offscreen onscreen
all:
	@echo "No \"all\" target for this Makefile. Try 'make offscreen' or 'make onscreen' to build sets of the examples."
offscreen: $(EXAMPLE_EXES)
onscreen: $(EXAMPLE_NEXES)
debug: $(EXAMPLE_LIST_D)
clean:
	rm -rf $(BUILT_DIR)/examples/

$(EXAMPLE_LIST): % : $(BUILT_DIR)/examples/% $(BUILT_DIR)/examples/%-onscreen
$(BUILT_DIR)/examples/%: $(BASE_DIR)/examples/%.cpp
	@mkdir -p $(BUILT_DIR)/examples
	$(CXX) $< \
		$(CFLAGS) $(SILKIE_OFFSCREEN_CFLAGS_RELEASE) \
		-DGL_GLEXT_PROTOTYPES=1 \
		-I$(PREFIX)/include -I$(BASE_DIR)/src -I$(BASE_DIR)/examples/include \
		-L$(BASE_DIR)/build -L$(LIB_DIR) $(SILKIE_OFFSCREEN_LIBS_RELEASE) \
		-o $(BUILT_DIR)/examples/$*

$(BUILT_DIR)/examples/%-onscreen: $(BASE_DIR)/examples/%.cpp
	@mkdir -p $(BUILT_DIR)/examples
	$(CXX) $< \
		$(CFLAGS) $(SILKIE_ONSCREEN_CFLAGS_RELEASE) \
		-DGL_GLEXT_PROTOTYPES=1 \
		-I$(PREFIX)/include -I$(BASE_DIR)/src -I$(BASE_DIR)/examples/include \
		-L$(BASE_DIR)/build -L$(LIB_DIR) $(SILKIE_ONSCREEN_LIBS_RELEASE) \
		-o $(BUILT_DIR)/examples/$*-onscreen

$(EXAMPLE_LIST_D): % : $(BUILT_DIR)/examples/% $(BUILT_DIR)/examples/%-onscreen
$(BUILT_DIR)/examples/%-Debug: $(BASE_DIR)/examples/%.cpp
	@mkdir -p $(BUILT_DIR)/examples
	$(CXX) $< \
		$(CFLAGS) $(SILKIE_OFFSCREEN_CFLAGS_DEBUG) \
		-DGL_GLEXT_PROTOTYPES=1 \
		-I$(PREFIX)/include -I$(BASE_DIR)/src -I$(BASE_DIR)/examples/include \
		-L$(BASE_DIR)/build -L$(LIB_DIR) $(SILKIE_OFFSCREEN_LIBS_DEBUG) \
		-o $(BUILT_DIR)/examples/$*-Debug

$(BUILT_DIR)/examples/%-Debug-onscreen: $(BASE_DIR)/examples/%.cpp
	@mkdir -p $(BUILT_DIR)/examples
	$(CXX) $< \
		$(CFLAGS) $(SILKIE_ONSCREEN_CFLAGS_DEBUG) \
		-DGL_GLEXT_PROTOTYPES=1 \
		-I$(PREFIX)/include -I$(BASE_DIR)/src -I$(BASE_DIR)/examples/include \
		-L$(BASE_DIR)/build -L$(LIB_DIR) $(SILKIE_ONSCREEN_LIBS_DEBUG) \
		-o $(BUILT_DIR)/examples/$*-Debug-onscreen

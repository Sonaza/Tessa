MAKE_DIR = $(PWD)

BUILD_DIR   := $(MAKE_DIR)/builds
INT_DIR     := $(MAKE_DIR)/obj

# ROOT_DIR    := $(MAKE_DIR)/root 
# DRV_DIR     := $(MAKE_DIR)/driver
# INCLUDE_DIR := $(MAKE_DIR)/include
# DEBUG_DIR   := $(MAKE_DIR)/debug

EXTERNALS_DIR := $(MAKE_DIR)/linuxext

INC_PATH := 
INC_PATH += -I$(MAKE_DIR)
INC_PATH += -I$(EXTERNALS_DIR)/include
INC_PATH += -I$(ROOT_DIR)

LIB_PATH :=
LIB_PATH += -L$(EXTERNALS_DIR)/libs

CXX = g++
LD  = ld
RM  = rm
AR  = ar

CXXFLAGS :=
CXXFLAGS += $(INC_PATH) $(LIB_PATH) 
CXXFLAGS += -Wall -O -ggdb -Wno-unknown-pragmas -std=c++17 -fms-extensions
CXXFLAGS += -DTS_BUILD_DEBUG -DDEBUG

MODULES := -lcontainer -llang -lmath -lstring -lfile -lsys -lthread -lrenderer -lengine -linput -lprofiling -lresource

LDFLAGS := $(MODULES)
LDFLAGS += -lfreeimage
LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
LDFLAGS += -lfmt -lvpx -lnestegg -lsiphash -llz4

export MAKE_DIR CXX LD RM AR CXXFLAGS LDFLAGS LIBS INC_PATH BUILD_DIR INT_DIR

all:
	# Level 0
	@$(MAKE) -C ts/container -f container.mk
	@$(MAKE) -C ts/lang -f lang.mk
	@$(MAKE) -C ts/math -f math.mk
	@$(MAKE) -C ts/string -f string.mk

	# Level 1
	@$(MAKE) -C ts/file -f file.mk
	@$(MAKE) -C ts/sys -f sys.mk
	@$(MAKE) -C ts/thread -f thread.mk

	# Level 2
	@$(MAKE) -C ts/renderer -f renderer.mk

	# Level 3
	@$(MAKE) -C ts/engine -f engine.mk
	@$(MAKE) -C ts/input -f input.mk
# 	@$(MAKE) -C ts/profiling -f profiling.mk
# 	@$(MAKE) -C ts/resource -f resource.mk

# 	# Level 4
# 	@$(MAKE) -C ts/ivie -f ivie.mk

.PHONY: clean
clean:
	# Level 0
	@$(MAKE) -C ts/container -f container.mk clean
	@$(MAKE) -C ts/lang -f lang.mk clean
	@$(MAKE) -C ts/math -f math.mk clean
	@$(MAKE) -C ts/string -f string.mk clean

	# Level 1
	@$(MAKE) -C ts/file -f file.mk clean
	@$(MAKE) -C ts/sys -f sys.mk clean
	@$(MAKE) -C ts/thread -f thread.mk clean

	# Level 2
	@$(MAKE) -C ts/renderer -f renderer.mk clean

	# Level 3
	@$(MAKE) -C ts/engine -f engine.mk clean
	@$(MAKE) -C ts/input -f input.mk clean
	@$(MAKE) -C ts/profiling -f profiling.mk clean
	@$(MAKE) -C ts/resource -f resource.mk clean

	# Level 4
	@$(MAKE) -C ts/ivie -f ivie.mk clean

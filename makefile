MAKE_DIR = $(PWD)

BUILD_DIR   := $(MAKE_DIR)/builds
INT_DIR     := $(MAKE_DIR)/obj

EXTERNALS_DIR := $(MAKE_DIR)/linuxext

INC_PATH := 
INC_PATH += -I$(MAKE_DIR)
INC_PATH += -I$(EXTERNALS_DIR)/include

LIB_PATH :=
LIB_PATH += -L$(INT_DIR)
LIB_PATH += -L$(EXTERNALS_DIR)/libs

CC  = clang
CXX = clang++
LD  = ld
RM  = rm -f
AR  = ar

WFLAGS :=
WFLAGS += -Wall -Werror -Wextra
WFLAGS += -Wno-unknown-pragmas -Wno-unused-parameter
WFLAGS += -Wno-unused-private-field

CXXFLAGS :=
CXXFLAGS += $(INC_PATH)
CXXFLAGS += $(WFLAGS) -ggdb -std=c++17 -fms-extensions -pthread
CXXFLAGS += -DTS_BUILD_DEBUG -DDEBUG
# CXXFLAGS += -DSFML_STATIC

CCFLAGS :=
CCFLAGS += $(INC_PATH)
CCFLAGS += $(WFLAGS) -ggdb -std=c11 -pthread
CCFLAGS += -DTS_BUILD_DEBUG -DDEBUG

MODULES := 
# MODULES += -Wl,--whole-archive
MODULES += -Wl,-Bstatic
MODULES += -lts-container -lts-lang -lts-math -lts-string
# MODULES += -lts-file -lts-sys -lts-thread 
MODULES += -lts-file -lts-thread 
MODULES += -lts-renderer
MODULES += -lts-engine -lts-input -lts-profiling -lts-resource
MODULES += -Wl,-Bdynamic
# MODULES += -Wl,--no-whole-archive

LDFLAGS := 
LDFLAGS += $(LIB_PATH)
LDFLAGS += $(MODULES)
LDFLAGS += -lstdc++fs
LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system 
LDFLAGS += -lfreeimage
LDFLAGS += -lfmt -lvpx -lnestegg -lsiphash -llz4 
# LDFLAGS += -Wl,--verbose

export MAKE_DIR CC CXX LD RM AR CCFLAGS CXXFLAGS LDFLAGS INC_PATH BUILD_DIR INT_DIR

all:
	# Level 0
	@$(MAKE) -C ts/container -f container.mk
	@$(MAKE) -C ts/lang -f lang.mk
	@$(MAKE) -C ts/math -f math.mk
	@$(MAKE) -C ts/string -f string.mk

	# Level 1
	@$(MAKE) -C ts/file -f file.mk
# 	@$(MAKE) -C ts/sys -f sys.mk
	@$(MAKE) -C ts/thread -f thread.mk

	# Level 2
	@$(MAKE) -C ts/renderer -f renderer.mk

	# Level 3
	@$(MAKE) -C ts/engine -f engine.mk
	@$(MAKE) -C ts/input -f input.mk
	@$(MAKE) -C ts/profiling -f profiling.mk
	@$(MAKE) -C ts/resource -f resource.mk

	# Level 4
	@$(MAKE) -C ts/ivie -f ivie.mk

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


# Foreground colors
FBLACK   :=$(shell tput setaf 0)
FRED     :=$(shell tput setaf 1)
FGREEN   :=$(shell tput setaf 2)
FYELLOW  :=$(shell tput setaf 3)
FBLUE    :=$(shell tput setaf 4)
FMAGENTA :=$(shell tput setaf 5)
FCYAN    :=$(shell tput setaf 6)
FGRAY    :=$(shell tput setaf 7)
# Background colors
BBLACK   :=$(shell tput setab 0)
BRED     :=$(shell tput setab 1)
BGREEN   :=$(shell tput setab 2)
BYELLOW  :=$(shell tput setab 3)
BBLUE    :=$(shell tput setab 4)
BMAGENTA :=$(shell tput setab 5)
BCYAN    :=$(shell tput setab 6)
BGRAY    :=$(shell tput setab 7)
# Reset colors
NC       :=$(shell tput sgr0)

export FBLACK FRED FGREEN FYELLOW FBLUE FMAGENTA FCYAN FGRAY BBLACK BRED BGREEN BYELLOW BBLUE BMAGENTA BCYAN BGRAY NC

MODULE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJ_DIR := $(INT_DIR)/$(MODULE_NAME)
TARGET  := $(INT_DIR)/libts-$(MODULE_NAME).a

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard *.c) $(wildcard */*.c)
SRCS := $(filter-out Precompiled.cpp, $(SRCS))
SRCS := $(filter-out CallStack.cpp, $(SRCS))
SRCS := $(filter-out %Windows.cpp, $(SRCS))

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS_TARGET := $(addprefix $(OBJ_DIR)/, $(OBJS))

CXXFLAGS += -I$(MODULE_DIR)

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

$(TARGET): $(OBJS_TARGET)
	@mkdir -p $(INT_DIR)
	@echo "\n    $(FCYAN)Archive   $(FMAGENTA)$(notdir $@)$(NC)\n"
	@$(AR) crv $@ $^
	@echo " $(OBJS)\n"

$(OBJ_DIR)/%.o: %.cpp
	@echo "    $(FGREEN)CXX       $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $^ -o $@

$(OBJ_DIR)/%.o: %.c
	@echo "    $(FGREEN)CC        $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@$(RM) $(TARGET) $(OBJS)
	@$(RM) *.expand
	@echo "    Remove Objects:   $(OBJS)"

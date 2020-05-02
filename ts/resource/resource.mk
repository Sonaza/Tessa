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
DEPS := $(OBJS_TARGET:.o=.d)

CXXFLAGS := -I$(MODULE_DIR) $(CXXFLAGS)

$(TARGET): $(OBJS_TARGET)
	@mkdir -p $(INT_DIR)
	@echo "    $(FCYAN)Archive        $(FMAGENTA)$(notdir $@)$(NC)"
	@$(AR) cr $@ $^
	@echo "    $(FYELLOW)Shared Object  $(FMAGENTA)$(notdir $(patsubst %.a,%.so,$@))$(NC)\n"
	$(CXX) $^ -shared -o $(patsubst %.a,%.so,$@)

$(OBJ_DIR)/%.o: %.cpp
	@echo "    $(FGREEN)CXX            $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -fPIC -MMD -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo "    $(FGREEN)CC             $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) -fPIC -MMD -c $< -o $@

.PHONY: clean
clean:
	@$(RM) $(TARGET) $(OBJS) $(DEPS)
	@echo "    Remove Objects:   $(OBJS)"
# 	@$(RM) *.expand

-include $(DEPS)

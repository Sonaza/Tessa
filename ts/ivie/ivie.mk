MODULE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJ_DIR := $(INT_DIR)/$(MODULE_NAME)
TARGET  := $(BUILD_DIR)/$(MODULE_NAME)

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard *.c) $(wildcard */*.c)
SRCS := $(filter-out Precompiled.cpp, $(SRCS))
SRCS := $(filter-out %Windows.cpp, $(SRCS))

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS_TARGET := $(addprefix $(OBJ_DIR)/, $(OBJS))
DEPS := $(OBJS_TARGET:.o=.d)

CXXFLAGS := -I$(MODULE_DIR) $(CXXFLAGS)

$(TARGET): $(OBJS_TARGET)
	@mkdir -p $(BUILD_DIR)
	@echo "    Generate Program $(notdir $(TARGET)) from $^"
	$(CXX) $^ $(CXXFLAGS) -Wl,-Map=$(TARGET).map $(LDFLAGS) -o $@
	cp $(TARGET) $(MAKE_DIR)/workdir/ivie
	
$(OBJ_DIR)/%.o: %.cpp
	@echo "    $(FGREEN)CXX       $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo "    $(FGREEN)CC        $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -MMD -c $< -o $@

.PHONY: clean
clean:
	@$(RM) $(TARGET) $(OBJS) $(DEPS)
	@$(RM) -r $(INT_DIR) $(BUILD_DIR)
	@echo "    Remove Objects:   $(OBJS)"
	@echo "    Remove Libraries:  $(notdir $(TARGET))"
# 	@$(RM) *.expand

-include $(DEPS)

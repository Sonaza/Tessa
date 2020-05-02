MODULE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJ_DIR := $(INT_DIR)/$(MODULE_NAME)
TARGET  := $(BUILD_DIR)/$(MODULE_NAME)

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard *.c) $(wildcard */*.c)
SRCS := $(filter-out Precompiled.cpp, $(SRCS))
SRCS := $(filter-out %Windows.cpp, $(SRCS))

LIB_DEPS := $(wildcard $(INT_DIR)/*.so)

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS_TARGET := $(addprefix $(OBJ_DIR)/, $(OBJS))
DEPS := $(OBJS_TARGET:.o=.d)

CXXFLAGS := -I$(MODULE_DIR) $(CXXFLAGS)

$(TARGET): $(OBJS_TARGET) $(LIB_DEPS)
	@mkdir -p $(BUILD_DIR)
	@echo "\n    $(FYELLOW)Linking program binary $(FBLUE)$(notdir $(TARGET))"
	@echo "    $(FYELLOW)Using: $(FGRAY)$^$(NC)\n\n"
	$(CXX) $^ $(CXXFLAGS) -Wl,-Map=$(TARGET).map $(LDFLAGS) -o $@
	cp $(TARGET) $(MAKE_DIR)/workdir/ivie
	
	@echo "    $(FGREEN)Build finished.$(NC)\n"
	
$(OBJ_DIR)/%.o: %.cpp
	@echo "    $(FGREEN)CXX            $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo "    $(FGREEN)CC             $(FMAGENTA)$<$(NC)"
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

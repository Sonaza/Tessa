SOURCE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJS_DIR    := $(INT_DIR)/$(MODULE_NAME)
PROG_TARGET := $(BUILD_DIR)/$(MODULE_NAME)

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard *.c) $(wildcard */*.c)
SRCS := $(filter-out Precompiled.cpp, $(SRCS))
SRCS := $(filter-out %Windows.cpp, $(SRCS))

SHARED_OBJECT_DEPS := $(wildcard $(INT_DIR)/*.so)

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS_TARGET := $(addprefix $(OBJS_DIR)/, $(OBJS))
DEPFILES    := $(OBJS_TARGET:.o=.d)

CXXFLAGS := -I$(SOURCE_DIR) $(CXXFLAGS)

$(PROG_TARGET): $(OBJS_TARGET) $(SHARED_OBJECT_DEPS)
	@mkdir -p $(BUILD_DIR)
	@echo "\n    $(FYELLOW)Linking program binary $(FBLUE)$(notdir $(PROG_TARGET))"
	@echo "    $(FYELLOW)Using: $(FGRAY)$^$(NC)\n\n"
	@$(CXX) $^ $(CXXFLAGS) -Wl,-Map=$(PROG_TARGET).map $(LDFLAGS) -o $@
	@cp $(PROG_TARGET) $(MAKE_DIR)/workdir/ivie
	@echo "    $(FGREEN)Build finished.$(NC)\n"
	
$(OBJS_DIR)/%.o: %.cpp
	@echo "    $(FGREEN)CXX            $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

$(OBJS_DIR)/%.o: %.c
	@echo "    $(FGREEN)CC             $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -MMD -c $< -o $@

.PHONY: clean
clean:
	@$(RM) $(PROG_TARGET) $(OBJS_TARGET) $(DEPFILES)
	@$(RM) -r $(INT_DIR) $(BUILD_DIR)
	@echo "\n"
	@echo "    $(FRED)Removing program objects and target:   $(FYELLOW)$(MODULE_NAME)$(NC)"

-include $(DEPFILES)

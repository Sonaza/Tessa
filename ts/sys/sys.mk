SOURCE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJS_DIR   := $(INT_DIR)/$(MODULE_NAME)
LIB_TARGET := $(INT_DIR)/libts-$(MODULE_NAME).a
SO_TARGET  := $(patsubst %.a,%.so,$(LIB_TARGET))

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard *.c) $(wildcard */*.c)
SRCS := $(filter-out Precompiled.cpp, $(SRCS))
SRCS := $(filter-out CallStack.cpp, $(SRCS))
SRCS := $(filter-out WindowsUtils.cpp, $(SRCS))
SRCS := $(filter-out LZ4Compressor.cpp, $(SRCS))
SRCS := $(filter-out %Windows.cpp, $(SRCS))

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS_TARGET := $(addprefix $(OBJS_DIR)/, $(OBJS))
DEPFILES    := $(OBJS_TARGET:.o=.d)

CXXFLAGS := -I$(SOURCE_DIR) $(CXXFLAGS)

$(LIB_TARGET): $(OBJS_TARGET)
	@mkdir -p $(INT_DIR)
	@echo "    $(FCYAN)Archive        $(FMAGENTA)$(notdir $@)$(NC)"
	@$(AR) cr $@ $^
	@echo "    $(FYELLOW)Shared Object  $(FMAGENTA)$(notdir $(patsubst %.a,%.so,$@))$(NC)\n"
	@$(CXX) $^ -shared -o $(LIB_TARGET)

$(OBJS_DIR)/%.o: %.cpp
	@echo "    $(FGREEN)CXX            $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -fPIC -MMD -c $< -o $@

$(OBJS_DIR)/%.o: %.c
	@echo "    $(FGREEN)CC             $(FMAGENTA)$<$(NC)"
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) -fPIC -MMD -c $< -o $@

.PHONY: clean
clean:
	@$(RM) $(LIB_TARGET) $(OBJS_TARGET) $(SHARED_OBJS) $(DEPFILES)
	@$(RM) -r $(OBJS_DIR)
	@echo "    $(FRED)Removing module objects and targets:   $(FYELLOW)$(MODULE_NAME)$(NC)"
	@echo "\n"

-include $(DEPFILES)

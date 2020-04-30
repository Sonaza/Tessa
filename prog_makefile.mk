MODULE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJ_DIR := $(INT_DIR)/$(MODULE_NAME)
TARGET  := $(BUILD_DIR)/$(MODULE_NAME)

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp)

OBJS := $(SRCS:.cpp=.o)
# OBJS := $(addprefix $(OBJ_DIR)/, $(OBJS))

CXXFLAGS += -I$(MODULE_DIR) -pie

$(TARGET): $(SRCS)
    @mkdir -p $(BUILD_DIR)
    @$(CC) $^ $(CXXFLAGS) -Wl,-Map=$(TARGET).map $(LDFLAGS) -o $@
    @echo "    Generate Program $(notdir $(TARGET)) from $^"

.PHONY: clean
clean:
    @$(RM) -f $(OBJS) $(TARGET)
    @$(RM) -f *.expand
    @$(RM) -rf $(INT_DIR) $(BUILD_DIR)
    @echo "    Remove Objects:   $(OBJS)"
    @echo "    Remove Libraries:  $(notdir $(TARGET))"

MODULE_DIR  = $(CURDIR)
MODULE_NAME = $(shell basename $(CURDIR))

OBJ_DIR := $(INT_DIR)/$(MODULE_NAME)
TARGET  := $(INT_DIR)/$(MODULE_NAME).a

SRCS := $(wildcard *.cpp) $(wildcard */*.cpp)
SRCS := $(filter-out Precompiled.cpp, $(SRCS))
SRCS := $(filter-out CallStack.cpp, $(SRCS))

OBJS := $(SRCS:.cpp=.o)
OBJS_TARGET := $(addprefix $(OBJ_DIR)/, $(OBJS))

CXXFLAGS += -I$(MODULE_DIR)

# .PHONY: test
# test:
# 	@echo MODULE_DIR $(MODULE_DIR)
# 	@echo MODULE_NAME $(MODULE_NAME)
# 	@echo OBJ_DIR $(OBJ_DIR)
# 	@echo TARGET $(TARGET)
# 	@echo SRCS $(SRCS)
# 	@echo OBJS $(OBJS)
# 	@echo CXXFLAGS $(CXXFLAGS)

$(TARGET): $(OBJS_TARGET)
	@mkdir -p $(INT_DIR)
	@$(AR) cr $@ $^
	@echo "    Archive    $(notdir $@)"

# $(OBJS): $(SRCS)

$(OBJ_DIR)/%.o: %.cpp
	@echo "    CXX       $<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $^ -o $@
# 	@echo "              -> $@"
	
# 	@mv *.o $(dir $@)

.PHONY: clean
clean:
	@$(RM) -f $(TARGET) $(OBJS)
	@$(RM) -f *.expand
	@echo "    Remove Objects:   $(OBJS)"

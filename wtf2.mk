SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard *.c) $(wildcard */*.c)

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

all: 
	@echo "$(OBJS)"

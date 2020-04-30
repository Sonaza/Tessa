OBJDIR := obj/helloworld
potato := $(addprefix $(OBJDIR)/, potato.o tomato.o)

all:
	@echo $(potato)

OBJDIR := obj/helloworld
potato := $(addprefix $(OBJDIR)/, potato.o tomato.o)

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
NC       :=$(shell tput setaf 9)$(shell tput setab 9)

all:
	@echo " $(FRED)$(potato) $(FGREEN)HELLOO  $(NC)"
	@echo "$(FBLACK)$(BGREEN)BLACK$(NC)  NCWORKS?"
	@echo "$(FRED)RED$(NC)"
	@echo "$(FGREEN)GREEN$(NC)"
	@echo "$(FYELLOW)YELLOW$(NC)"
	@echo "$(FBLUE)BLUE$(NC)"
	@echo "$(FMAGENTA)MAGENTA$(NC)"
	@echo "$(FCYAN)CYAN$(NC)  NCWORKS?"
	@echo "$(FGRAY)$(BMAGENTA)GRAY$(NC) NCWORKS?"
	


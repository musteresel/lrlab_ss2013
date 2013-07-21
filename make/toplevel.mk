# Actual toplevel makefile code
# * Grabs general options
# * Includes util makefiles
# * Handles all target
# * Place to include submakefiles

all:
	@echo "Built $^"

include make/general.mk
include make/asf.mk

#include usart_demo/usart_demo.mk
#...
include test/test.mk

include make/autoprereq.mk

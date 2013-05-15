# Toplevel makefile
# * Grabs general options
# * Includes util makefiles
# * Handles all target
# * Place to include submakefiles

#include make/general.mk
#include make/asf.mk
#include make/autoprereq.mk

all:
	@echo "Built $^"

#include usart_demo/usart_demo.mk
#...

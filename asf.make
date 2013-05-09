# Makefile include to define targets for various parts of the Atmel Software
# Framework
# -----------------------------------------------------------------------------
# To use this, just include this file in your makefile and specify the required
# asf parts as dependency. On first run (or after deleting the .asf.location
# file) you will be asked for the path to the asf root directory!




# -----------------------------------------------------------------------------




# Check if there is already an asf location provided. If not, ask for the
# location and store it for later use.
LOCATION_FILE:=".asf.location"
LOCATION:=$(shell \
	if [ -f $(LOCATION_FILE) ]; then \
		cat $(LOCATION_FILE); \
	else \
		echo -n "Please enter path to asf: " 1>&2; \
		read -r location; \
		echo $$location | tee $(LOCATION_FILE); \
	fi)
# Check if the provided location is valid and fail if not.
# Note that in order to be valid, a location has to be a path to a directory.
# This does not check whether that directory is a valid asf directory!
ifeq '$(shell [ -d $(LOCATION) ] && echo Y || echo N)' 'N'
$(shell [ -f $(LOCATION_FILE) ] && rm $(LOCATION_FILE))
$(error Given location of Atmel Software Framework is not a directory)
endif




# Use the information target as default target (for this makefile) and make it
# phony so it is always run.
.PHONY: asf-information
asf-information:
	@echo "Using asf.make with asf @ $(LOCATION)"




# -----------------------------------------------------------------------------




ASF_INCLUDE_PATHS := $(shell find $(LOCATION)/./*/boards -maxdepth 1 -type d) \
	$(shell find $(LOCATION)/./*/drivers -mindepth 1 -maxdepth 1 -type d) \
	$(shell find $(LOCATION)/./*/components -mindepth 1 -maxdepth 2 -path '*doxygen*' -prune -o -type d -print) \
	$(shell find $(LOCATION)/./*/utils -maxdepth 1 -type d)
# Missing: services
# Untested/unsure utils





# -----------------------------------------------------------------------------



# Recipe to build the asf library. The resulting (static) library will be
# placed to the top of the build directory and will be called "asf.a"
# The rule has no prerequisites defined here, they need to be defined by the
# including makefile. This way one can specify which parts of the asf are
# needed.
asf-lib:
	@echo "Building asf library with:"
	@for i in $^;do echo "  * $$i";done
	@echo "Include paths:"
	@for p in $(ASF_INCLUDE_PATHS);do echo "  $$p";done




# -----------------------------------------------------------------------------




# Rule to build gpio lib.
asf_gpio_files := avr32/drivers/gpio/gpio.c
# TODO: convert source to object file names, add LOCATION, builddirectory
asf-gpio:
	@echo "Building $@"

asf-usart:
	@echo "Building $@"




# -----------------------------------------------------------------------------




# To use it:
asf-lib: asf-gpio

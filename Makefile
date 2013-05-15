# Toplevel makefile which provides:
# * Out of source build
# * ASF integration
# * No recursion
# TODO: make/builddir.mk is included twice, time intense stuff would sum up!

include make/builddir.mk
ifneq ($(BUILD_LOCATION),$(notdir $(CURDIR)))
include make/outofsource.mk
else
SOURCE_LOCATION := $(SOURCE_LOCATION)/src # TODO: could be done better
VPATH += $(SOURCE_LOCATION)
include make/toplevel.mk
endif

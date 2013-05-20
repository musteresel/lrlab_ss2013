# Toplevel makefile which provides:
# * Out of source build
# * ASF integration
# * No recursion

ifndef SOURCE_LOCATION
include make/builddir.mk
include make/outofsource.mk
else
VPATH += $(SOURCE_LOCATION)
include make/toplevel.mk
endif

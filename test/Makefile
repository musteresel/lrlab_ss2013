include ../make/general.make
include ../make/autoprereq.make
include ../make/asf.make
#include $(BUILDDIR)/autoprereq/test.d
#include $(BUILDDIR)/autoprereq/$(subst ..,UP,../file.d)


SOURCES := test.c ../file.c
OBJS := $(addprefix $(BUILDDIR)/,$(SOURCES:.c=.o))

all: $(OBJS)
	echo Hi

-include $(shell find $(BUILDDIR)/autoprereq -type f -name '*.d')


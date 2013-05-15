# This file is responsible of changing to the appropriate build directory and
# reexecuting make there.

.SUFFIXES:
MAKETARGET = $(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile \
						 -I $(CURDIR) SOURCE_LOCATION=$(CURDIR) $(MAKECMDGOALS)
.PHONY: $(BUILD_LOCATION)
$(BUILD_LOCATION):
	+@[ -d $@ ] || mkdir -p $@
	+@$(MAKETARGET)
Makefile: ;
%.mk:: ;
%:: $(BUILD_LOCATION) ; :
.PHONY: clean
clean:
	rm -rf $(BUILD_LOCATION)

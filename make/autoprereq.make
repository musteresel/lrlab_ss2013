# Let the compiler generate prerequisites.
# This is implemented according to the section on Automatic Prerequisites in
# the GNU make manual.
# To use the prerequisites, all one has to do is
# include $(BUILDDIR)/autoprereq/$(sourcefiles:.c=.d)
# TODO handle ../ correctly!
# TODO delete temporary file even if something fails!

$(BUILDDIR)/autoprereq/%.d: %.c
	@set -e; \
		mkdir -p "$(dir $@)"; \
		rm -f "$@"; \
		$(CC) -M $(CPPFLAGS) "$<" > "$@.$$$$"; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < "$@.$$$$" > "$@"; \
		rm -f "$@.$$$$"


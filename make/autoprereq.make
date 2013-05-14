# Let the compiler generate prerequisites.
# This is implemented according to the section on Automatic Prerequisites in
# the GNU make manual.
# To use the prerequisites, all one has to do is
# include $(BUILDDIR)/autoprereq/$(sourcefiles:.c=.d)
# TODO handle ../ correctly!
# TODO delete temporary file even if something fails!

#$(BUILDDIR)/autoprereq/%.d: %.c
#	@set -e; \
#		mkdir -p "$(dir $@)"; \
#		rm -f "$@"; \
#		$(CC) -M $(CPPFLAGS) "$<" > "$@.$$$$"; \
#		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < "$@.$$$$" > "$@"; \
#		rm -f "$@.$$$$"


$(BUILDDIR)/%.o: %.c
	mkdir -p "$(dir $(subst ..,UP,$@))"
	mkdir -p "$(dir $(BUILDDIR)/autoprereq/$(subst ..,UP,$*).c)"
	$(CC) -c $(CPPFLAGS) $(CFLAGS) "$*.c" -o "$(BUILDDIR)/$(subst ..,UP,$*).o"
	$(CC) -M -MP -MT "$(BUILDDIR)/$(subst ..,UP,$*).o" "$*.c" > "$(BUILDDIR)/autoprereq/$(subst ..,UP,$*).d"



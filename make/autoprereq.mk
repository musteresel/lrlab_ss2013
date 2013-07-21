# Let the compiler generate prerequisites.

%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MD -c -o $@ $<
# TODO sed magic!

%.a:
	mkdir -p $(dir $@)
	$(AR) rcs $@ $^


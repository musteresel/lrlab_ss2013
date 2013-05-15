
SOURCES := test/main.c

test: $(SOURCES:.c=.o)
	@echo "Building test"

-include $(SOURCES:.c=.d)


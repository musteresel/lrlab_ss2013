

SOURCES := test/main.c avr32/drivers/gpio/gpio.c

test: $(SOURCES:.c=.o)
	@echo "Building test"

-include $(SOURCES:.c=.d)


DEVICE = /dev/ttyUSB0
OBJECTS = main.o coroutine.o

all: main.hex

%.o: %.c
	avr-gcc -Wall -O1 -DF_CPU=16000000UL -mmcu=atmega328p -c -I. -o $@ $<

main.S: main.c
	avr-gcc -Wall -O1 -DF_CPU=16000000UL -mmcu=atmega328p -S -o $@ $<

main.bin: $(OBJECTS)
	avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -g -o $@ $(OBJECTS)

main.hex: main.bin
	avr-objcopy -O ihex -R .eeprom $< $@

install: main.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P $(DEVICE) -b 57600 -U flash:w:$<

clean:
	rm -f main.S main.hex main.bin $(OBJECTS)

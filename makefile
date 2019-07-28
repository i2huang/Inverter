# AVR-GCC Makefile
PROJECT=inverter
SOURCES=inverter.c
CC=avr-gcc
SIZE=avr-size
OBJCOPY=avr-objcopy
MMCU=atmega328p

CFLAGS=-mmcu=$(MMCU) -Wall

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)
	$(SIZE) $(PROJECT).out

program: $(PROJECT).hex
	#avrdude -p m8 -c avrusb500 -e -U flash:w:$(PROJECT).hex
	avrdude -p m328p -P//./COM9 -c arduino -C ../avrdude.conf -b 115200 -F -u -U flash:w:$(PROJECT).hex

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex


../flashtools/mac/tool-avrdude/avrdude -C ../flashtools/mac/tool-avrdude/avrdude.conf -v -patmega32u4 -cavr109 -P/dev/cu.usbmodem14201 -b57600 -D -Uflash:w:firmware.hex:i 

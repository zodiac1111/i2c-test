# Select from the following
all:adxl345-dbg

# all target
other:compile-other upload
eeporm:compile-eeprom upload
adxl345:compile-adxl345 upload
adxl345-dbg:compile-adxl345-dbg upload
adxl345-hello:compile-adxl345-helloworld upload
adxl345-simple:compile-adxl345-simple upload


compile-adxl345-helloworld:
	arm-linux-gcc adxl345-helloworld.c -o app -Wall

compile-adxl345-simple:
	arm-linux-gcc adxl345-simplest-use.c -o app -Wall

compile-adxl345-dbg:
	arm-linux-gcc i2c-adxl345-dbg.c -o app -Wall

compile-adxl345:
	arm-linux-gcc i2c-adxl345.c -o app -Wall

compile-eeprom:
	arm-linux-gcc i2c-eeprom.c -o app -Wall

compile-other:
	arm-linux-gcc i2c-other.c -o app -Wall

#mini2440 ftp upload
upload:
	lftp -u plg,plg 192.168.1.230 \
	-e "put app;quit"

clean:
	rm -f app
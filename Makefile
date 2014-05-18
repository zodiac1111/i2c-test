# Select from the following
all:adxl345-hello
CC=/home/zodiac1111/Mysoft/gcc-linaro-arm-linux-gnueabihf-4.8-2014.03_linux/bin/arm-linux-gnueabihf-gcc
# all target
other:compile-other upload
eeporm:compile-eeprom upload
adxl345:compile-adxl345 upload
adxl345-dbg:compile-adxl345-dbg upload
adxl345-hello:compile-adxl345-helloworld upload
adxl345-simple:compile-adxl345-simple upload


compile-adxl345-helloworld:
	${CC} adxl345-helloworld.c -o app -Wall

compile-adxl345-simple:
	${CC} adxl345-simplest-use.c -o app -Wall

compile-adxl345-dbg:
	${CC} i2c-adxl345-dbg.c -o app -Wall

compile-adxl345:
	${CC} i2c-adxl345.c -o app -Wall

compile-eeprom:
	${CC} i2c-eeprom.c -o app -Wall

compile-other:
	${CC} i2c-other.c -o app -Wall

#mini2440 ftp upload
upload:
	lftp -u plg,plg 192.168.1.230 \
	-e "put app;quit"

clean:
	rm -f app
// i2c 应用层调试,读写挂载在i2c总线上的adxl345,
// 与eeprom公用一个设配文件
// 设置从设备,mini2440 参考其给的例子,完全在应用层
// 驱动层到 /dev/i2c/0 这个设备的过程完全没有涉及,
// 对于eeprom配合原始的i2c -w 和 i2c -r 检查结果
///@filename adxl345-simplest-use.c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <unistd.h>
#define ADDRESS 0x53 		// adxl345地址
#define DEV "/dev/i2c-2" 	//设配文件,与原先的eeprom一样,通过地址区分
#define DEVID 	0x00 		// adxl345器件id的寄存器地址,见数据手册
#define POWER_CTL 	0x2D
#define DATAX0		0x32
#define DATAX1		0x33
#define DATAY0		0x34
#define DATAY1 		0x35
#define DATAZ0 		0x36
#define DATAZ1 		0x37
unsigned char adxl_read(int fd, unsigned char reg);
int adxl_write(int fd, unsigned char reg, unsigned char val);
signed short  adxl_read_short(int fd, unsigned char reg);
int main(int argc, char* argv[])
{
	int fd;
	int ret = 0;
	signed short int x,y,z;//三轴加速度
	// 1  打开 ,从设备打开
	fd = open(DEV, O_RDWR);
	if (fd<0) {
		perror("open:");
		return -1;
	}
	// 2 设置从设备地址
	ret = ioctl(fd, I2C_SLAVE, ADDRESS);
	if (ret<0) {
		perror("ioctl:");
		goto CLOSE;
	}
	// 3 操作:读取器件id
	ret = adxl_read(fd, DEVID);
	printf("器件ID[0x%02x]=0x%02x\n", DEVID, ret);
	// 4 设置->启动,具体参见芯片手册.
	//   这里仅设置有限的读取数值功能.其实太复杂的还没搞懂:P
	adxl_write(fd, POWER_CTL, 0x28);
	// 5 循环测量(读取三轴加速度)
	for (;;) {
		x = adxl_read_short(fd, DATAX0);
		y = adxl_read_short(fd, DATAY0);
		z = adxl_read_short(fd, DATAZ0);
		printf("x=%5d y=%5d z=%5d\n", x,y,z);
		usleep(100*1000);
	}
CLOSE:
	close(fd);
	return 0;
}

unsigned char adxl_read(int fd, unsigned char reg)
{
	union i2c_smbus_data data; 		//数据联合体
	struct i2c_smbus_ioctl_data args;	//参数结构
	int ret;
	// 读数据
	args.read_write = I2C_SMBUS_READ;	//读
	args.command = reg;			//读取的位置(寄存器地址)
	args.size = I2C_SMBUS_BYTE_DATA;	//1个字节的数据
	args.data = &data;			//保存数据到data联合体
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		perror("ioctl[读取]");
		return 0xff;
	}
	return data.byte;
}
int adxl_write(int fd, unsigned char reg, unsigned char val)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int ret;
	data.byte = val;			//数据
	args.read_write = I2C_SMBUS_WRITE;	//写[与上面读相对应]
	args.command = reg;			//地址
	args.size = I2C_SMBUS_BYTE_DATA;	//1个字节大小的数据
	args.data = &data;			//保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {
		perror("写入ioctl错误");
		return -1;
	}
	return 0;
}
signed short  adxl_read_short(int fd, unsigned char reg)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int ret;
	args.read_write = I2C_SMBUS_READ;	//读
	args.command = reg;			//地址
	args.size = I2C_SMBUS_WORD_DATA;	//2字节大小的数据(short型)
	args.data = &data;			//保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {
		perror("读取short ioctl错误");
		return -1;
	}
	return data.word;
}

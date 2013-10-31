// i2c 应用层调试,读写挂载在i2c总线上的eeprom
// 设置从设备,mini2440 参考其给的例子,完全在应用层
// 驱动层到 /dev/i2c/0 这个设备的过程完全没有涉及,
// 对于eeprom配合原始的i2c -w 和 i2c -r 检查结果
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
// i2c
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "adxl345.h"
//切换 EEPROM 则1 其他则测试桩
//切换 DEV_CLASS 指定三种测试方案
//1 自带的eeprom驱动
//2 stub测试桩驱动
//3 利用s3c2440平台i2c驱动,指定adxl345的,即在应用层实现驱动
//通用的i2c使用2440的平台驱动
#define DEV_CLASS 3
#if DEV_CLASS==1
#define ADDRESS 0x50 // eeprom设备地址
#define DEV "/dev/i2c/0"
#elif DEV_CLASS==2
#define ADDRESS 0x1D // stub测试桩地址
#define DEV "/dev/i2c/1"
#elif DEV_CLASS==3
#define ADDRESS 0x53 // adxl地址证实
#define DEV "/dev/i2c/0"
#endif

#define CHECK_I2C_FUNC( var, label ) \
	do { 	if(0 == (var & label)) { \
		fprintf(stderr, "\nError: " \
			#label " function is required. Program halted.\n\n"); \
		return 1; } \
	} while(0);

struct Pos{
	short x;
	short y;
	short z;
};

__u8 adxl_read(int fd, __u8 reg);
int adxl_write(int fd, __u8 reg, __u8 val);
__u16 adxl_read_byte(int fd, __u8 reg);
int adxl_read_six_byte(int fd, __u8 reg,struct Pos *pos);
int print_regs(int fd, int from, int to);
int guixyz(char name, int val);

// 主函数
int main(int argc, char* argv[])
{
	int fd;
	int ret = 0;
	int funcs;
	int i;
	struct Pos pos;
	// 1  打开 ,从设备打开
	if (argc==2) {
		fd = open(argv[1], O_RDWR);
	} else {
		fd = open(DEV, O_RDWR);
	}
	if (fd<0) {
		perror("打开:");
		printf("fd=%d\n", fd);
		return -1;
	}
	// 2 设置从设备地址
	ret = ioctl(fd, I2C_SLAVE, ADDRESS);
	if (ret<0) {
		perror("ioctl:");
		printf("i2c_slave=%d\n", I2C_SLAVE);
		goto CLOSE;
	}
#if 1
	//读取器件id
	ret = adxl_read(fd, REG_DEVID);
	if (ret<0) {
		perror("器件id");
	}
	printf("器件ID[0x%X]=0x%X\n", REG_DEVID, ret);
#endif
#if 0
	//其他寄存器测试
	printf("所有寄存器读取测试:\n");
	print_regs(fd, 0x1E, 0x39);
#endif
#if 1
	//adxl_write(fd, DATA_FORMAT, 0b0000);
	adxl_write(fd, POWER_CTL, 0x28);
	//adxl_write(fd, BW_RATE, 0x0B);
	//adxl_write(fd, INT_ENABLE, 0x00);
#endif
#if 1
	__s16 x,z,y;
	for (;;) {
	//x = adxl_read_byte(fd, DATAX0);
	//y = adxl_read_byte(fd, DATAY0);
	//z = adxl_read_byte(fd, DATAZ0);
	//printf("x=%5d y=%5d z=%5d\n", x,y,z);
	adxl_read_six_byte(fd, DATAX0,&pos);
	printf("x=%5d y=%5d z=%5d\n", pos.x,pos.y,pos.z);
	printf("\n\n");
	usleep(1000*1000);
	}
#endif

#if 0
	int x, y, z;
	int x0, x1, y0, y1, z0, z1;
	for (;;) {
		x0 = adxl_read(fd, DATAX0);
		x1 = adxl_read(fd, DATAX1);
		y0 = adxl_read(fd, DATAY0);
		y1 = adxl_read(fd, DATAY1);
		z0 = adxl_read(fd, DATAZ0);
		z1 = adxl_read(fd, DATAZ1);
		x = x0;
		y = y0;
		z = z0;
		guixyz('x', x);
		guixyz('y', y);
		guixyz('z', z);
		printf("\n");
		//printf("x0:%03d x1:%03d \n",x0,x1);
		//printf("y0:%03d y1:%03d \n",y0,y1);
		//printf("z0:%03d z1:%03d \n",z0,z1);
		//printf("x=%03d y=%03d z=%03d \n",x,y,z);
		usleep(100*1000);
		printf("\f");
	}
#endif
	CLOSE:
	close(fd);
	return 0;
}
int guixyz(char name, int val)
{
	int i;
	printf("%c:%4d", name, val);
	for (i = -128; i<127; i += 2) {
		if (i==0) {
			printf("o");
			continue;
		}
		if (val<i) {
			printf("-");
		} else {
			printf("=");
		}
	}
	printf("\n");
	return 0;
}
int adxl_write(int fd, __u8 reg, __u8 val)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int ret;
	data.byte = val;     //数据
	args.read_write = I2C_SMBUS_WRITE;     //读
	args.command = reg;     //命令?
	args.size = I2C_SMBUS_BYTE_DATA;     //大小一比特
	args.data = &data;     //保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		printf("写入ioctl错误");
		return -1;
	}
	return data.byte;
}
__u8 adxl_read(int fd, __u8 reg)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int ret;
#if 0
	args.read_write = I2C_SMBUS_WRITE;     //写
	args.command = reg;     //地址
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = NULL;     //没有数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret<0) {
		printf("r<0 r=%d\n", ret);
		perror("ioctl");
	}
#endif
	//再访问,读
	args.read_write = I2C_SMBUS_READ;     //读
	args.command = reg;     //命令?
	args.size = I2C_SMBUS_BYTE_DATA;     //大小一比特
	args.data = &data;     //保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		printf("读取ioctl错误");
		return -1;
	}
	return data.byte;
}
int adxl_read_six_byte(int fd, __u8 reg,struct Pos *pos)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int ret;
	int len=6;//读取字节长度
#if 0

	args.read_write = I2C_SMBUS_WRITE;     //写
	args.command = 6;     //地址
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = NULL;     //没有数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret<0) {
		printf("r<0 r=%d\n", ret);
		perror("ioctl");
	}
#endif
#if 1
	//读
	data.byte=len; //读取的长度从这里传进去
	args.read_write = I2C_SMBUS_READ;     //读
	args.command = reg;     //开始地址
	args.size = I2C_SMBUS_I2C_BLOCK_DATA;     //2字节
	args.data = &data;     //保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		printf("读取ioctl错误");
		return -1;
	}
#endif
#if 0
	int i;
	for (i = 0; i<7; i++) {
		printf(" %02X ", data.block[i]);
		fflush(stdout);
	}
#endif
	memcpy(pos,&data.block[1],len);
	return 0;
}
unsigned short adxl_read_byte(int fd, __u8 reg)
{
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int ret;
#if 0
	args.read_write = I2C_SMBUS_WRITE;     //写
	args.command = reg;     //地址
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = NULL;     //没有数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret<0) {
		printf("r<0 r=%d\n", ret);
		perror("ioctl");
	}
#endif
#if 1
	//读
	args.read_write = I2C_SMBUS_READ;     //读
	args.command = reg;     //开始地址
	args.size = I2C_SMBUS_WORD_DATA;     //2字节
	args.data = &data;     //保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		printf("读取ioctl错误");
		return -1;
	}
#endif
#if 0
	int i;
	for (i = 0; i<2; i++) {
		printf(" %02X ", data.block[i]);
		fflush(stdout);
	}
#endif

	return data.word;
}

int print_regs(int fd, int from, int to)
{
	int i;
	int ret;
	for (i = from; i<=to; i++) {
		ret = adxl_read(fd, i);
		if (ret<0) {
			perror("读取测试寄存器");
		}
		printf("[0x%02X]=0x%02X\t", i, ret);
		if ((i-from+1)%4==0) {
			printf("\n");
		}
	}
	printf("\n");
	return 0;
}

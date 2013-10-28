// i2c 应用层调试,读写挂载在i2c总线上的eeprom
// 设置从设备,mini2440 参考其给的例子,完全在应用层
// 驱动层到 /dev/i2c/0 这个设备的过程完全没有涉及,
// 对于eeprom配合原始的i2c -w 和 i2c -r 检查结果
//
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
#define CHECK_I2C_FUNC( var, label ) \
	do { 	if(0 == (var & label)) { \
		fprintf(stderr, "\nError: " \
			#label " function is required. Program halted.\n\n"); \
		return 1; } \
	} while(0);

int main(int argc, char* argv[])
{
	int fd;
	int ret;
	char buf[10];
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	int r;
	int funcs;
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
	// 1.1 获得支持函数列表<不必须>但是可以知道很多有用的信息
	// 支持的函数返回,并保存在 funcs 中.
	if ((r = ioctl(fd, I2C_FUNCS, &funcs)<0))
	{
		perror("获得支持函数列表");
		return -1;
	}
	// check for req funcs 通过 & 操作得到是不是支持这些函数.
	// 应用层可以判断什么函数能用,什么不能用.类似向下兼容的作用
	CHECK_I2C_FUNC(funcs, I2C_FUNC_SMBUS_READ_BYTE);
	CHECK_I2C_FUNC(funcs, I2C_FUNC_SMBUS_WRITE_BYTE);
	CHECK_I2C_FUNC(funcs, I2C_FUNC_SMBUS_READ_BYTE_DATA);
	CHECK_I2C_FUNC(funcs, I2C_FUNC_SMBUS_WRITE_BYTE_DATA);
	CHECK_I2C_FUNC(funcs, I2C_FUNC_SMBUS_READ_WORD_DATA);
	CHECK_I2C_FUNC(funcs, I2C_FUNC_SMBUS_WRITE_WORD_DATA);
	// 2 设置从设备地址
	ret = ioctl(fd, I2C_SLAVE, ADDRESS);
	if (ret<0) {
		perror("ioctl:");
		printf("i2c_slave=%d\n", I2C_SLAVE);
		goto CLOSE;
	}
	// *****************  读取 ********************** //
	//读 = 写地址,读内容
	buf[0] = 0x10;	//register,对于eeprom就是地址
	//先写地址
	args.read_write = I2C_SMBUS_WRITE;     //写
	args.command = 0x10;     //地址
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = NULL;     //没有数据
	r = ioctl(fd, I2C_SMBUS, &args);
	if (r<0) {
		printf("r<0 r=%d\n", r);
		perror("ioctl");
	}
	//再访问,读
	args.read_write = I2C_SMBUS_READ;     //读
	args.command = 0;     //命令?
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = &data;     //保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		printf("读取ioctl错误");
		goto CLOSE;
	}
	//正确读取,数据保存在data中
	printf("0x%x读取结果: 0x%x\n", 0x10, data.byte);
	// ******************* 写入 **********************  /
	//写
	args.read_write = I2C_SMBUS_WRITE;     //写
	args.command = 0x10;     //地址
	args.size = I2C_SMBUS_BYTE_DATA;     //多个字节
	data.byte = 0xab;     //数据
	args.data = &data;     //数据联合体
	r = ioctl(fd, I2C_SMBUS, &args);
	if (r<0) {
		printf("r<0 r=%d\n", r);
	}
	printf("在 0x%x 处写入 0x%x \n", args.command, data.byte);
	// ******  再次读取 *************************** ////
	//读=写地址,读内容
	buf[0] = 0x10;	//register,对于eeprom就是地址
	//先写地址
	args.read_write = I2C_SMBUS_WRITE;     //写
	args.command = 0x10;     //地址
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = NULL;     //没有数据
	r = ioctl(fd, I2C_SMBUS, &args);
	if (r<0) {
		printf("r<0 r=%d\n", r);
	}
	//再访问,读
	args.read_write = I2C_SMBUS_READ;     //读
	args.command = 0;     //命令?
	args.size = I2C_SMBUS_BYTE;     //大小一比特
	args.data = &data;     //保存数据
	ret = ioctl(fd, I2C_SMBUS, &args);
	if (ret!=0) {     //非零错误
		printf("再次读取ioctl错误");
		goto CLOSE;
	}
	//正确读取,数据保存在data中
	printf("再次读取 0x%x 结果: 0x%x\n", 0x10, data.byte);
	CLOSE:
	close(fd);
	return 0;
}

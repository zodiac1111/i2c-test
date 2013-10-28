//通过i2c-stub调试应用层的驱动程序
#define ADDRESS 0x1D // stub测试桩地址,动过insmod的参数指定
#define DEV "/dev/i2c/1"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

int main(int argc, char* argv[])
{
	int fd;
	union i2c_smbus_data data;
	struct i2c_smbus_ioctl_data args;
	unsigned char reg =0x10; //读取(寄存器)地址
	int funcs;
	// 1  打开 ,从设备文件打开
	fd = open(DEV, O_RDWR);
	if (fd<0) {
		perror("open");
		return -1;
	}
	// 2 获得支持函数列表<不必须>但是可以知道很多有用的信息
	// 支持的函数返回,并保存在 funcs 中.
	if (ioctl(fd, I2C_FUNCS, &funcs)<0) {
		perror("获得支持函数列表");
		goto CLOSE;
	}
	// 2.1 check for req funcs 通过 & 操作得到是不是支持这些函数.
	// 应用层可以判断什么函数能用,什么不能用.类似向下兼容的作用
	// 各种功能宏在<linux/i2c.h>中定义
	if ((funcs&I2C_FUNC_SMBUS_READ_BYTE)==0) {
		printf("Error:I2C_FUNC_SMBUS_READ_BYTE "
				"function is required.\n");
	}
	// 3 设置从设备地址
	if (ioctl(fd, I2C_SLAVE, ADDRESS)!=0) {
		perror("ioctl");
		goto CLOSE;
	}
	//读
	args.read_write = I2C_SMBUS_READ;	//读
	args.command = reg;			//地址
	args.size = I2C_SMBUS_BYTE_DATA;	//1个字节的数据
	args.data = &data;			//保存数据
	if (ioctl(fd, I2C_SMBUS, &args)!=0) {
		perror("ioctl");
		goto CLOSE;
	}
	printf("0x%x 读取结果: 0x%x\n", reg, data.byte);
	//写
	args.read_write = I2C_SMBUS_WRITE;	//写
	args.command = reg;			//地址
	args.size = I2C_SMBUS_BYTE_DATA;	//1个字节数据
	data.byte = 0xab;			//数据
	args.data = &data;			//数据联合体
	if (ioctl(fd, I2C_SMBUS, &args)!=0) {
		perror("ioctl");
	}
	printf("在 0x%x 处写入 0x%x \n", args.command, data.byte);
	//读
	args.read_write = I2C_SMBUS_READ;
	args.command = reg;
	args.size = I2C_SMBUS_BYTE_DATA;
	args.data = &data;
	if (ioctl(fd, I2C_SMBUS, &args)!=0) {
		printf("再次读取ioctl错误");
		goto CLOSE;
	}
	printf("再次读取 0x%x 结果: 0x%x\n", reg, data.byte);
	CLOSE:
	close(fd);
	return 0;
}

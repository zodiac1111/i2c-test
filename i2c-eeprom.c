// i2c 应用层协议,读写挂载在i2c总线上的eeprom
// 设置从设备,mini2440 参考其给的例子,完全在应用层
// 驱动层到 /dev/i2c/0 这个设备的过程完全没有涉及,
// 配合原始的i2c -w 和 i2c -r 检查结果
#define ADDRESS 0x50 // eeprom的地址

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
// i2c
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
int main(int argc,char* argv[])
{
	int fd;
	int ret;
	char buf[10];
	//打开 ,从设备打开
	if(argc==2){
		fd = open(argv[1], O_RDWR );
	}else{
		fd = open( "/dev/i2c/0", O_RDWR );
	}
	if(fd<0){
		perror("打开:");
		printf("fd=%d\n",fd);
		return -1;
	}
	ret=ioctl( fd, I2C_SLAVE, ADDRESS );
	if(ret<0){
		perror("ioctl:");
		printf("i2c_slave=%d\n",I2C_SLAVE);
		goto CLOSE;
	}
	//读
	buf[0]=0x10;//register,对于eeprom就是地址
	buf[1]=0;
	ret=read(fd,buf,1);
	if(ret!=1){
		perror("读取");
		printf("ret=%d\n",ret);
		goto CLOSE;
	}
	printf("读取结果: buf0 %x, buf1 %x \n",buf[0],buf[1]);
	//写
	buf[0]=0x10;//register
	buf[1]=0xAB;//
	ret=write(fd,buf,2);
	if(ret!=2){
		perror("write i2c");
		printf("ret=%d\n",ret);
		goto CLOSE;
	}
	printf("在 %x 处写入 %x \n",buf[0],buf[1]);
	//再读,检查
	buf[0]=0x10;//register,对于eeprom就是地址
	buf[1]=0;
	ret=read(fd,buf,1);
	if(ret!=1){
		perror("再次读取");
		printf("ret=%d\n",ret);
		goto CLOSE;
	}
	printf("再次读取: buf0 %x ,buf1 %x \n",buf[0],buf[1]);
CLOSE:
	close(fd);
	return 0;
}

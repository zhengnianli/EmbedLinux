
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 * ./led_drv_test /dev/led on
 * ./led_drv_test /dev/led off
 */
int main(int argc, char **argv)
{
	int fd;
	char buf[1024];
	int len;
	char status;
	
	/* 1. 判断参数 */
	if (argc < 2) 
	{
		printf("Usage: %s <dev> <on | off>\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open("/dev/led", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/led\n");
		return -1;
	}

	/* 3. 写文件或读文件 */
	if (0 == strcmp(argv[2], "on"))
	{
		status = 0;
		write(fd, &status, 1);
	}
	else
	{
		status = 1;
		write(fd, &status, 1);
	}
	
	close(fd);
	
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "btn.h"

void all_ledoff();

int main(void)
{
	int fd;
	fd = open("/dev/btn",O_RDWR);
	if(fd<0)
	{
		perror("open");
		exit(-1);
	}	
	char val;
	/*复位*/
	ioctl(fd,TINY4412_BTN_IORESET);
	/*复位后，获取并打印*/
	ioctl(fd,TINY4412_BTN_IOGET,&val);
	printf("app: reset and get val %d\n",val);	
	sleep(1);

	/*设置值为10*/
	val = 10;
	ioctl(fd,TINY4412_BTN_IOSET,&val);
	sleep(1);
	/*设置值后，再获取打印值*/
	ioctl(fd,TINY4412_BTN_IOGET,&val);
	sleep(1);	
	printf("app:set and get val %d\n",val);

	int i=0;
	while(1)
	{
	  	all_ledoff(fd);
	  	ioctl(fd,TINY4412_LED_ON,&(i));
		i=++i%4;
		sleep(1);
	}

	return 0;
}

void all_ledoff(int fd_f)
{

	int k=0;
	while(k!=4)
	{
		ioctl(fd_f,TINY4412_LED_OFF,&(k));
		k++;
	}

}

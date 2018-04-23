#ifndef __BTN_H
#define __BTN_H
#include <linux/ioctl.h>
//定义一个幻数（type）
#define TINY4412_BTN_TYPE	'f'
#define TINY4412_LED_TYPE	'l' 
    //分别定义reset，get，set命令
#define TINY4412_BTN_IORESET  _IO(TINY4412_BTN_TYPE,0)
#define TINY4412_BTN_IOGET 	 _IOR(TINY4412_BTN_TYPE, 1, char)
#define TINY4412_BTN_IOSET 	 _IOW(TINY4412_BTN_TYPE, 2, char)

#define TINY4412_LED_ON 	 _IOW(TINY4412_LED_TYPE, 0, char)
#define TINY4412_LED_OFF 	 _IOW(TINY4412_LED_TYPE, 1, char)

#define TINY4412_BTN_MAXNUM 	3

#endif

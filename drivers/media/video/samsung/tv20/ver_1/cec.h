/* linux/drivers/media/video/samsung/tv20/ver_1/cec.h
 *
 * Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsung.com/
 *
 * cec ftn header for Samsung TVOut driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _LINUX_CEC_H_
#define _LINUX_CEC_H_

#define CEC_IOC_MAGIC        'c'

#define CEC_IOC_SETLADDR     _IOW(CEC_IOC_MAGIC, 0, unsigned int)

#define S5P_HDMI_FIN	24000000
#define CEC_DIV_RATIO	187500

#endif /* _LINUX_CEC_H_ */
/*
 *this is for spi flash command
 * 
 *
 */

#ifndef __MY_UDP_H__
#define __MY_UDP_H__
#include "sockets.h"
#include "upgrade.h"

#define UDP_LOCAL_PORT 1200
#define UDP_DATA_LEN 100

void ota_upgrade_task(void *pvParameters);

#endif //__MY_UDP_H__


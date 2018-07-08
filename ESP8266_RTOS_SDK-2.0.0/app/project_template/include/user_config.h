/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#include "esp_common.h"
#include "uart.h"
// config wifi station
#define CONFIG_WIFI_STATION_V01
// config wifi AP mode
//#define CONFIG_WIFI_SOFT_AP_V01
//use spi flash read/write
#define CONFIG_USE_SPI_FLASH
//use time RTC
//#define CONFIG_USE_TIME_RTC
//use wifi udp
//#define CONFIG_USE_UDP
//use wifi TCP
#define CONFIG_USE_TCP
#define CONFIG_USE_TCP_CLIENT
#define CONFIG_USE_TCP_SERVER


#if defined(CONFIG_USE_UDP)
#include "m_udp.h"
#endif//CONFIG_USE_UDP

#if defined(CONFIG_USE_TCP)
#include "m_tcp.h"
#endif//CONFIG_USE_TCP

#if defined(CONFIG_USE_TIME_RTC)
#include "m_time.h"
#endif//CONDIG_USE_TIME_RTC

#if defined(CONFIG_USE_SPI_FLASH)
#include "m_spi_flash.h"
#endif//CONFIG_USE_SPI_FLASH

#if defined(CONFIG_WIFI_STATION_V01)||defined(CONFIG_WIFI_SOFT_AP_V01)
#include "m_wifi_config.h"
#endif

#endif//__USER_CONFIG_H__


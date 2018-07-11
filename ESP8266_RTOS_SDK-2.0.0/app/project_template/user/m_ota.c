/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 *
 *
 */

#include "user_config.h"
#if defined(CONFIG_USE_OTA)

#define OTA_UPGRADE_BIN1_ADDR	0x1000
#define OTA_UPGRADE_BIN2_ADDR	0x81000
#define OTA_UPGRADE_BIN1_NAME	"user1.4096.new.6.bin"
#define OTA_UPGRADE_BIN2_NAME	"user2.4096.new.6.bin"
#define OTA_SERVER_URL_IP		"http://192.168.2.244/"

void ota_upgrade_time_task(void *pvParameters){
	bool is_timeout = 0;
	while(1){

		/* 允许其它发送任务执行。 taskYIELD()通知调度器现在就切换到其它任务，而不必等到本任务的时
间片耗尽 */
		taskYIELD();
	}
	//upgrade timeout
	if(is_timeout){
		// set upgrade flag to idle
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
	}
	vTaskDelete(NULL);
}

#define UPGRADE_BUF_LEN 128
void ota_upgrade_task(void *pvParameters){
	int sock_fd = 0,ret = 0;
	u32_t upgrade_addr = 0,erease_addr = 0;
	SpiFlashOpResult err_flash =SPI_FLASH_RESULT_ERR;
	char upgrade_buf[UPGRADE_BUF_LEN] ={"fadfafdafd"};
	char upgrade_url[128] = {0};
	char *file_name = 0 ;
	char *pHeader = 0;
	long file_size = 0 , file_rec = 0;
	struct sockaddr_in server_ip;

	printf("--lx %s enter\n", __PRETTY_FUNCTION__);
	// 0---- is get ip addr
	while(is_get_ip_addr() !=0);
	
	// http Header process web URL
    bzero(upgrade_url,sizeof(upgrade_url));
	sprintf(upgrade_url,\
            "GET %s HTTP/1.1\r\n"\
            "Host: %s\r\n"\
            "Connection: Keep-Alive\r\n"\
            "Content-Type: application/octet-stream\r\n"\
            "\r\n",file_name,OTA_SERVER_URL_IP);
	
	// create timeout detectd task.
	xTaskCreate(ota_upgrade_time_task, "ota timeout detect", 200, NULL, 2, NULL);
	
	// 1----Create a tcp socket
	sock_fd = mWifi_tcp_socket_create();

	// 2----Create a TCP connection
	ret = mWifi_tcp_client_connect_server(sock_fd,&server_ip);

	// check which userbin is used.0x00 : UPGRADE_FW_BIN1, i.e. user1.bin @return 0x01 : UPGRADE_FW_BIN2, i.e. user2.bin
	if(system_upgrade_userbin_check()){
		upgrade_addr = OTA_UPGRADE_BIN2_ADDR;
		file_name = OTA_UPGRADE_BIN1_NAME;
	}else{
		upgrade_addr = OTA_UPGRADE_BIN1_ADDR;
		file_name = OTA_UPGRADE_BIN1_NAME;
	}
	// erase the upgrade bin sector.
	erease_addr = upgrade_addr/SPI_FLASH_SEC_SIZE; 
	err_flash = spi_flash_erase_sector(erease_addr);//erase flash sector(N*4K/4096) 
	if(err_flash != SPI_FLASH_RESULT_OK){
		printf("---lx spi_flash_erase_sector faild\n");
	}

	// system upgrade init
	system_upgrade_init();

	// setting the upgrade flag   - UPGRADE_FLAG_IDLE - UPGRADE_FLAG_START - UPGRADE_FLAG_FINISH    0x02
	system_upgrade_flag_set(UPGRADE_FLAG_START);

	//Send the downloading request to the server.
	if(mWifi_tcp_client_send_data(sock_fd,upgrade_url,strlen(upgrade_url)+1) < 0) {
		printf("Send the downloading request to the server failed!\n");
	}
	// Parse the HTTP header
	bzero(upgrade_url,sizeof(upgrade_url));
	pHeader = upgrade_url;
	//i = 0;
	while((ret = read(sock_fd,pHeader,1)) != 0)
	{
		if (*pHeader == '\n'){
			if((pHeader - upgrade_url) < 2)//if( i < 2)
				continue;
			if (*(pHeader-1) == '\r' && *pHeader == '\n' && *(pHeader-2) == '\n' )
				break;
		}
		//i ++;
		pHeader++;
	}
	//get file size 
	pHeader = strstr(upgrade_url,"Content-Length:");
	if (pHeader != NULL)
	{
		pHeader = strchr(pHeader,':');
		pHeader++;
		file_size = strtol(pHeader,NULL,10);
	}
	//printf("----lx Parse the HTTP header::%d byte\n%s\n",file_size,upgrade_url);
	printf("----lx Parse the HTTP header::%d byte\n",file_size);
	while((ret = read(sock_fd ,upgrade_buf,UPGRADE_BUF_LEN)) > 0) {
		
	}
   for(file_rec = 0;;){
	    int rec;
	    rec = recv(sock_fd,upgrade_buf,UPGRADE_BUF_LEN,0);
	    if(rec <= 0)
	        break;
		else
			file_rec += rec;
	   // save the recive code int dst addr write(fd,text,rec);
	   //// write the new firmware into flash by spi_flash_write 	err_flash = mSpi_flash_write(upgrade_addr,upgrade_buf,rec);
		// TODO: fit in the data
		if((upgrade_addr + file_rec)/SPI_FLASH_SEC_SIZE != erease_addr){
			erease_addr =(upgrade_addr + file_rec)/SPI_FLASH_SEC_SIZE;
			err_flash = spi_flash_erase_sector(erease_addr);//erase flash sector(N*4K/4096)
			if(err_flash != SPI_FLASH_RESULT_OK){
				printf("---lx spi_flash_erase_sector faild\n");
			}
		}
		err_flash = spi_flash_write(upgrade_addr, (uint32 *)upgrade_buf,rec);
		if(err_flash != SPI_FLASH_RESULT_OK){
			printf("---lx write flash faild\n");
		}
		upgrade_addr += file_rec;
	    //printf("receive success Message:%d\n",rec);
		if(file_size == file_rec)
			break;
	}
   
	// upgrade successfull ,set the flag
	if(file_size == file_rec)
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
	else{
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
		printf("----lx update addr:%s--:%s failed!\n",upgrade_addr,file_name);
	}
	// reboot device
	system_upgrade_reboot();
	while(1){
		// 3---client send data packets via TCP communication
		ret = mWifi_tcp_client_send_data(sock_fd,upgrade_buf,UPGRADE_BUF_LEN);

		// 4---TCP client Receive data packets via TCP communication
		ret = mWifi_tcp_client_receive_data(sock_fd,upgrade_buf,UPGRADE_BUF_LEN);
		upgrade_buf[ret] = 0;
		printf("receive bytes:%d %s \n",ret,upgrade_buf);

			/* 允许其它发送任务执行。 taskYIELD()通知调度器现在就切换到其它任务，而不必等到本任务的时
间片耗尽 */
		//taskYIELD();
	}
	close(sock_fd);
	vTaskDelete(NULL);

}

#endif//CONFIG_USE_TIME_RTC


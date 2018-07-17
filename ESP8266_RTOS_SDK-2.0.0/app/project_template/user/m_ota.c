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
#define OTA_UPGRADE_BIN1_NAME	"user1.4096.new.4.bin"
#define OTA_UPGRADE_BIN2_NAME	"user2.4096.new.4.bin"
#define OTA_SERVER_URL_IP		"192.168.2.244"
#define OTA_HTTP_HEADER  "GET /user2.4096.new.4.bin HTTP/1.1\r\n"\
			            "Host: 192.168.2.244\r\n"\
			            "Connection: Keep-Alive\r\n"\
			            "Content-Type: application/octet-stream\r\n"\
			            "\r\n"
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

#define UPGRADE_BUF_LEN 1024
void ota_upgrade_task(void *pvParameters){
	int sock_fd = 0,ret = 0 ,retry = 0;
	volatile u32_t upgrade_addr = 0,i=0;
	u32_t erease_addr = 0,updated_addr = 0;
	SpiFlashOpResult err_flash =SPI_FLASH_RESULT_ERR;
	//char upgrade_buf[UPGRADE_BUF_LEN] ={0};
	char *upgrade_url;
	char *file_name = 0 ;
	char *pHeader = 0;
	long file_size = 0 , file_rec = 0;
	char save[4]={0};
	char extra = 0;
	struct sockaddr_in server_ip;

	printf("--lx %s enter\n", __PRETTY_FUNCTION__);
	// 0---- is get ip addr
	while(is_get_ip_addr() != 1);
	upgrade_url = zalloc(UPGRADE_BUF_LEN);
	// http Header process web URL
    //bzero(upgrade_url,sizeof(upgrade_url));
    printf("--lx 1  enter\n");
	/*sprintf(upgrade_url,\
            "GET %s HTTP/1.1\r\n"\
            "Host: %s\r\n"\
            "Connection: Keep-Alive\r\n"\
            "Content-Type: application/octet-stream\r\n"\
            "\r\n",file_name,OTA_SERVER_URL_IP);*/
    strcpy(upgrade_url,OTA_HTTP_HEADER);
	printf("--lx 2  enter\n");
	// create timeout detectd task.
	xTaskCreate(ota_upgrade_time_task, "ota timeout detect", 200, NULL, 2, NULL);
	
	// 1----Create a tcp socket
	sock_fd = mWifi_tcp_socket_create();
	printf("--lx 3  enter\n");

	// 2----Create a TCP connection
	//ret = mWifi_tcp_client_connect_server(sock_fd,&server_ip);
		//SET server addr/port
#if 1
	bzero(&server_ip, sizeof(struct sockaddr_in));
	//bzero(&server_ip, sizeof(struct sockaddr_in));
	server_ip.sin_family = AF_INET;
	server_ip.sin_addr.s_addr = inet_addr(OTA_SERVER_URL_IP);
	server_ip.sin_port = htons(80);

	do{
		ret = connect(sock_fd, (struct sockaddr *)&server_ip, sizeof(struct	sockaddr));
		if (ret != 0) {
			printf("ESP8266 TCP client task > connect fail! retry:%d\n",retry);
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP client task > connect ok!\n");
			break ;
		}
		retry++;
	}while((ret != 0)&&(retry <=3));// retry 3 times
#endif
	if(ret !=0){
		printf("connect failed");
		goto failed;
	}
	printf("--lx 4  enter\n");

	// check which userbin is used.0x00 : UPGRADE_FW_BIN1, i.e. user1.bin @return 0x01 : UPGRADE_FW_BIN2, i.e. user2.bin
	if(!system_upgrade_userbin_check()){
		upgrade_addr = OTA_UPGRADE_BIN2_ADDR;
		file_name = OTA_UPGRADE_BIN2_NAME;
	}else{
		upgrade_addr = OTA_UPGRADE_BIN1_ADDR;
		file_name = OTA_UPGRADE_BIN1_NAME;
	}
	// erase the upgrade bin sector.
	printf("ugrade addr %x \n",upgrade_addr);
	erease_addr = upgrade_addr/SPI_FLASH_SEC_SIZE; 
	err_flash = spi_flash_erase_sector(erease_addr);//erase flash sector(N*4K/4096) 
	if(err_flash != SPI_FLASH_RESULT_OK){
		printf("---lx spi_flash_erase_sector faild\n");
	}
	printf("--lx 5  enter\n");

	// system upgrade init
	////system_upgrade_init();

	// setting the upgrade flag   - UPGRADE_FLAG_IDLE - UPGRADE_FLAG_START - UPGRADE_FLAG_FINISH    0x02
	system_upgrade_flag_set(UPGRADE_FLAG_START);

	printf("----lx Parse the HTTP header::%d byte\n%s\n",file_size,upgrade_url);
	//Send the downloading request to the server.
	if(mWifi_tcp_client_send_data(sock_fd,upgrade_url,strlen(upgrade_url)+1) < 0) {
		printf("Send the downloading request to the server failed!\n");
	}
	// Parse the HTTP header
	bzero(upgrade_url,UPGRADE_BUF_LEN);
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
	printf("recive ret=%d http header=%d \n",pHeader-upgrade_url,strstr(upgrade_url,"\r\n\r\n")-upgrade_url+4);
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
	updated_addr =upgrade_addr;
#if 1  
	for(file_rec = 0;;){
	    int rec;
		// clear buf
		bzero(upgrade_url,sizeof(upgrade_url));
   		// aglin 4 bytes
		memcpy(upgrade_url, save, extra);
	    rec = read(sock_fd,upgrade_url + extra,UPGRADE_BUF_LEN - extra);
	    if(rec <= 0)
	        break;
		else
			file_rec += rec;
	   // save the recive code int dst addr write(fd,text,rec);
	   //// write the new firmware into flash by spi_flash_write 	err_flash = mSpi_flash_write(upgrade_addr,upgrade_buf,rec);
		// TODO: fit in the data
		if((upgrade_addr + file_rec)/SPI_FLASH_SEC_SIZE != erease_addr){
			erease_addr =(upgrade_addr + file_rec)/SPI_FLASH_SEC_SIZE;
			printf("erase sector %x \n",erease_addr);
			err_flash = spi_flash_erase_sector(erease_addr);//erase flash sector(N*4K/4096)
			if(err_flash != SPI_FLASH_RESULT_OK){
				printf("---lx spi_flash_erase_sector faild\n");
			}
		}

		rec += extra;
		extra = rec & 0x03;
		rec -= extra;

		if(extra<=4)
		   memcpy(save, upgrade_url + rec, extra);
		else
		   printf("ERR3:arr_overflow,%u,%d\n",__LINE__,extra);

		err_flash = spi_flash_write(updated_addr, (uint32 *)upgrade_url,rec);
		if(err_flash != SPI_FLASH_RESULT_OK){
			printf("---lx write flash faild\n");
		}
		updated_addr = upgrade_addr + file_rec - extra;
	    printf("receive success Message:%d upgrade_addr=%x file_rec=%x\n",rec,updated_addr,file_rec);
		if(0){
			printf("--receive Message:%s\n",upgrade_url);
		}
		if(file_size == file_rec){
			printf("successful rec %d bytes\n",file_rec);
			break;
		}
	}
#else // read data from flash just for test
	updated_addr =upgrade_addr;
	for(file_rec = 0;;){
		int rec;
		printf("---lx read data from flash!\n");
		rec = spi_flash_read(updated_addr,(uint32 *)upgrade_url,UPGRADE_BUF_LEN);
		for(i=0;i<UPGRADE_BUF_LEN;i++){
			if(i%16 ==0)
				printf("\n");
			printf("%02x",*(upgrade_url+i));
		}
		updated_addr += UPGRADE_BUF_LEN;
		
		break;
	}
#endif   
	// upgrade successfull ,set the flag
	if(file_size == file_rec){
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
		printf("----lx update addr:%x--:%s ok!\n",upgrade_addr,file_name);
	}else{
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
		printf("----lx update addr:%dx--:%s failed!\n",upgrade_addr,file_name);
	}
	// reboot device
	system_upgrade_reboot();
	/* 允许其它发送任务执行。 taskYIELD()通知调度器现在就切换到其它任务，而不必等到本任务的时
间片耗尽 */
		//taskYIELD();
failed:
	free(upgrade_url);
	close(sock_fd);
	vTaskDelete(NULL);

}

#endif//CONFIG_USE_TIME_RTC


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
	u32_t upgrade_addr = 0;
	SpiFlashOpResult err_flash =SPI_FLASH_RESULT_ERR;
	char upgrade_buf[UPGRADE_BUF_LEN] ={"fadfafdafd"};
	char upgrade_url[32] = "http://192.168.2.244/";
	struct sockaddr_in server_ip;

	printf("--lx %s enter\n", __PRETTY_FUNCTION__);
	// 0---- is get ip addr
	while(is_get_ip_addr() !=0);

	// create timeout detectd task.
	xTaskCreate(ota_upgrade_time_task, "ota timeout detect", 200, NULL, 3, NULL);
	
	// 1----Create a tcp socket
	sock_fd = mWifi_tcp_socket_create();

	// 2----Create a TCP connection
	ret = mWifi_tcp_client_connect_server(sock_fd,&server_ip);

	// check which userbin is used.0x00 : UPGRADE_FW_BIN1, i.e. user1.bin @return 0x01 : UPGRADE_FW_BIN2, i.e. user2.bin
	if(system_upgrade_userbin_check()){
		upgrade_addr = OTA_UPGRADE_BIN2_ADDR;
	}else{
		upgrade_addr = OTA_UPGRADE_BIN1_ADDR;
	}
	// erase the upgrade bin sector.
	err_flash = spi_flash_erase_sector(upgrade_addr/SPI_FLASH_SEC_SIZE);//erase flash sector(N*4K/4096) 
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
	while((ret = read(sock_fd ,upgrade_buf,UPGRADE_BUF_LEN)) > 0) {
		// write the new firmware into flash by spi_flash_write
	}

	// upgrade successfull ,set the flag
	system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
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


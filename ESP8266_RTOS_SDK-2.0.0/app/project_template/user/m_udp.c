/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 *
 *
 */

#include "user_config.h"
#if defined(CONFIG_USE_UDP)


// create udp socket
int mWifi_udp_socket_create(struct sockaddr_in * server_addr,socklen_t addr_len){
//	struct sockaddr_in server_addr;
//	memset(server_addr, 0, sizeof(server_addr));
	int retry = 0;
	int sock_fd;
	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(UDP_LOCAL_PORT);
	server_addr->sin_len = addr_len;
	do{
		sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock_fd == -1) {
			printf("ESP8266 UDP task > failed to create sock!\n");
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 UDP task > socket OK!\n");
		}
		retry++;
	}while((sock_fd == -1)&&(retry <=5));// retry 5 times
	return sock_fd;
}

//Bind a local port
int mWifi_udp_socket_bind(int sock_fd, const struct sockaddr *name, socklen_t namelen){
	int ret=0;
	int retry = 0;
	do{
		ret = bind(sock_fd, name, namelen);
		if (ret != 0) {
			printf("ESP8266 UDP task > captdns_task failed to bind sock,retry times:%d!\n",retry);
			vTaskDelay(1000/portTICK_RATE_MS);// delay 1000ms
		}else{
			printf("ESP8266 UDP task > bind OK!\n");
		}
		retry++;
	}while((ret != 0)&&(retry <=5));// retry 5 times

	return ret;
}

//Receive  the UDP data
int mWifi_udp_data_receive(int sock_fd,void *udp_msg ,size_t msg_len,
						struct sockaddr *from, socklen_t *fromlen){
	int ret = 0;
	int nNetTimeout = 10000;// 100s unit:ms
	
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int));//set sockt
	*fromlen = sizeof(struct sockaddr_in); // **** must setting ths fromlen
	ret = recvfrom(sock_fd, (uint8 *)udp_msg, msg_len, 0,from,fromlen);
	if (ret > 0) {
		printf("ESP8266 UDP task > recv %d Bytes \n",ret);
	}

	return ret;

}
//transmit the UDP data
int mWifi_udp_data_send(int sock_fd,void *udp_msg ,size_t msg_len,struct sockaddr *to, socklen_t tolen){
	// **** must setting ths tolen
	if(tolen == 0){
		tolen = sizeof(struct sockaddr_in);
		printf("mWifi_udp_data_send modify tolen from 0 to %d\n",tolen);
	}
	return sendto(sock_fd,(uint8*)udp_msg, msg_len, 0, to,tolen);
}

#endif//CONFIG_USE_UDP


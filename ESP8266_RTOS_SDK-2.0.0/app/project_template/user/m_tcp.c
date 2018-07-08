/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 *
 *
 */

#include "user_config.h"
#if defined(CONFIG_USE_TCP)

//Create a tcp socket
int mWifi_tcp_socket_create(void){
	int sock_fd = 0,retry = 0;

	do{
		sock_fd = socket(PF_INET, SOCK_STREAM, 0);
		if (sock_fd == -1) {
			printf("ESP8266 TCP client task > socket fail! retry:%d\n",retry);
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP client task > socket ok!\n");
			return sock_fd;
		}
		retry++;
	}while((sock_fd == -1)&&(retry <=5));// retry 5 times
	return sock_fd;
}

//Create a TCP connection
int mWifi_tcp_client_connect_server(int sock_fd,struct sockaddr_in *remote_ip){
	int ret = 0,retry = 0;	

	//SET server addr/port
	bzero(remote_ip, sizeof(struct sockaddr_in));
	remote_ip->sin_family = AF_INET;
	remote_ip->sin_addr.s_addr = inet_addr(REMOTE_TCP_SERVER_IP);
	remote_ip->sin_port = htons(REMOTE_TCP_SERVER_PORT);

	do{
		ret = connect(sock_fd, (struct sockaddr *)remote_ip, sizeof(struct	sockaddr));
		if (ret != 0) {
			printf("ESP8266 TCP client task > connect fail! retry:%d\n",retry);
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP client task > connect ok!\n");
			return ret;
		}
		retry++;
	}while((ret != 0)&&(retry <=3));// retry 3 times
	
	return ret;
}
//client send data packets via TCP communication
int mWifi_tcp_client_send_data(int sock_fd,char *buf,int buf_len){
	int ret = 0,retry = 0;	

	do{
		ret = write(sock_fd, buf, buf_len);
		if (ret < 0) {
			printf("ESP8266 TCP client task > send fail retry:%d\n",retry);;
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP client task > send ok!\n");
			return ret;
		}
		retry++;
	}while((ret < 0)&&(retry <=3));// retry 3 times
	
	return ret;
}
//TCP client Receive data packets via TCP communication
int mWifi_tcp_client_receive_data(int sock_fd,char *buf,int buf_len){
	int ret = 0,retry = 0;
	//char *recv_buf = (char *)zalloc(128);
	do{
		ret = read(sock_fd , buf, buf_len);
		if (ret <= 0) {
			printf("ESP8266 TCP client task > read data fail! retry:%d\n",retry);;
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP client task > read data ok!recbytes:%d\n",ret);
			return ret;
		}
		retry++;
	}while((ret <= 0)&&(retry <=3));// retry 3 times
	
	return ret;

}



#endif//CONFIG_USE_TCP


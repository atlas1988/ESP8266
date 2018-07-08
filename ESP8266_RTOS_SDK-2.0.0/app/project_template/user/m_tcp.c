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
			printf("ESP8266 TCP client task > send fail retry:%d\n",retry);
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
			printf("ESP8266 TCP client task > read data fail! retry:%d\n",retry);
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP client task > read data ok!recbytes:%d\n",ret);
			return ret;
		}
		retry++;
	}while((ret <= 0)&&(retry <=3));// retry 3 times
	
	return ret;

}

//Establish a TCP server
int mWifi_tcp_server_socket_create(void){
	int server_fd;
	int retry = 0;
	
	/* Create socket for incoming connections */
	do{
		server_fd = socket(PF_INET, SOCK_STREAM, 0);
		if (server_fd == -1) {
			printf("ESP8266 TCP server task > socket error\n");
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP server task > create socket: %d\n", server_fd);
			return server_fd;
		}
		retry++;
	}while((server_fd == -1)&&(retry <=5));// retry 5 times

	return server_fd;

}

//bind it with the local port.
int mWifi_tcp_server_socket_bind(int sock_fd, struct sockaddr_in  *server_addr){
	int ret=0,retry = 0;
		
	/* Construct local address structure */
	memset(server_addr, 0, sizeof(struct sockaddr_in)); /* Zero out structure */
	server_addr->sin_family = AF_INET; /* Internet address family */
	server_addr->sin_addr.s_addr = INADDR_ANY; /* Any incoming interface */
	server_addr->sin_len = sizeof(struct sockaddr_in);
	server_addr->sin_port = htons(TCP_SERVER_PORT); /* Local port */
	/* Bind to the local port */
	do{
		ret = bind(sock_fd, (struct sockaddr *)server_addr,sizeof(struct sockaddr_in));
		if (ret != 0) {
			printf("ESP8266 TCP server task > bind fail\n");
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP server task > port:%d\n",ntohs(server_addr->sin_port));
			return ret;
		}
		retry++;
	}while((ret != 0)&&(retry <=3));// retry 3 times
	return ret;
}
//Listen to	the	local connection Establish TCP server interception
int mWifi_tcp_server_listen(int sock_fd,int max_connect){
	int ret=0,retry = 0;
	do{
		/* Listen to the local connection */
		ret = listen(sock_fd, max_connect);
		if (ret != 0) {
			printf("ESP8266 TCP server task > failed to set listen queue!\n");
			vTaskDelay(1000/portTICK_RATE_MS);
		}else{
			printf("ESP8266 TCP server task > listen ok\n");
			return ret;
		}
		retry++;
	}while((ret != 0)&&(retry <=5));// retry 5 times
	return ret;
}

//TCP server wait for connect read data from connected client
int mWifi_tcp_server_accept(int server_fd,struct sockaddr_in *remote_addr,socklen_t *len){
	int client_fd,retry = 0;
	*len = sizeof(struct sockaddr_in);
	printf("ESP8266 TCP server task > wait client\n");
	do{
		/*block here waiting remote connect request*/
		if ((client_fd = accept(server_fd, (struct sockaddr *)remote_addr,len)) < 0) {
			printf("ESP8266 TCP server task > accept fail\n");
		}else{
			printf("ESP8266 TCP server task > Client from %s %d\n",	inet_ntoa(remote_addr->sin_addr), htons(remote_addr->sin_port));
			return client_fd;
		}
		retry++;
	}while((client_fd < 0)&&(retry <=3));// retry 3 times
	return client_fd;
}
//TCP client Receive data packets via TCP communication
int mWifi_tcp_server_receive_data(int sock_fd,char *buf,int buf_len){
	int ret = 0,retry = 0;
	//char *recv_buf = (char *)zalloc(128);
	do{
		ret = read(sock_fd , buf, buf_len);
		if (ret <= 0) {
			printf("ESP8266 TCP client task > read data fail! retry:%d\n",retry);
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


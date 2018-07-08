#include "user_config.h"

/* 
	config defined in user_config.h
*/
//wifi mode setting
#if defined(CONFIG_WIFI_STATION_V01)&&defined(CONFIG_WIFI_SOFT_AP_V01)
#define WIFI_MODE_CONFIG STATIONAP_MODE
#elif defined(CONFIG_WIFI_STATION_V01)
#define WIFI_MODE_CONFIG STATION_MODE
#else
#define WIFI_MODE_CONFIG SOFTAP_MODE
#endif//wifi mode setting

// setting station wifi router connect info
#define WIFI_AP_SSID_DEFUALT "atlas_2"
#define WIFI_AP_SSID_PASSWORD_DEFUALT "18068306040"
//connect esp8266 to wifi router
void mWifi_connect_ap(void){
	struct station_config * config = (struct station_config *)zalloc(sizeof(struct station_config));
	sprintf(config->ssid, WIFI_AP_SSID_DEFUALT);
	sprintf(config->password, WIFI_AP_SSID_PASSWORD_DEFUALT);
	wifi_station_set_config(config);
	free(config);
	wifi_station_connect();
}

//setting soft ap mode config
#define SOFT_AP_SSID_DEFUALT "ESP_AP"
#define SOFT_AP_SSID_PASSWORD_DEFUALT "12345678"
//config soft ap mode 
void mWifi_soft_ap_init(void){
	struct softap_config *config = (struct softap_config *)zalloc(sizeof(struct	softap_config)); // initialization
	wifi_softap_get_config(config); // Get soft-AP config first.
	sprintf(config->ssid, SOFT_AP_SSID_DEFUALT);
	sprintf(config->password, SOFT_AP_SSID_PASSWORD_DEFUALT);
	config->authmode = AUTH_WPA_WPA2_PSK;
	config->ssid_len = 0; // or its actual SSID length
	config->max_connection = 4;
	wifi_softap_set_config(config); // Set ESP8266 soft-AP config
	free(config);
	
	wifi_softap_dhcps_stop(); // disable soft-AP DHCP server
	//config the soft ap IP gatway netmask start
	struct ip_info info;
	IP4_ADDR(&info.ip, 192, 168, 5, 1); // set IP
	IP4_ADDR(&info.gw, 192, 168, 5, 1); // set gateway
	IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
	wifi_set_ip_info(SOFTAP_IF, &info);
	//config the soft ap IP gatway netmask end
	
	// config the ip pool range
	struct dhcps_lease dhcp_lease;
	IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 5, 100);
	IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 5, 105);
	wifi_softap_set_dhcps_lease(&dhcp_lease);
	wifi_softap_dhcps_start(); // enable soft-AP DHCP server
	
}
void mWifi_read_connected_station_info(void){
		struct station_info * station = wifi_softap_get_station_info();
	while(station){
		printf("bssid : "MACSTR," ip : "IPSTR"/n",MAC2STR(station->bssid), IP2STR(&station->ip));
		station = STAILQ_NEXT(station, next);
	}
	wifi_softap_free_station_info(); // Free it by calling functions
}
// get the device mac addr
#if defined(CONFIG_GET_WIFI_MAC_ADDR)
void mWifi_get_macaddr(void){
	char wifi_mac[6]={0};
	printf("----lx mWifi_get_macaddr----");
	wifi_set_opmode(STATIONAP_MODE);
	wifi_get_macaddr(SOFTAP_IF, wifi_mac);
	printf("SoftAP mac addr:"MACSTR"\n",MAC2STR(wifi_mac));
	wifi_get_macaddr(STATION_IF, wifi_mac);
	printf("Station mac addr:"MACSTR"\n",MAC2STR(wifi_mac));
}
#endif
// set the device mac addr
#if defined(CONFIG_SET_WIFI_MAC_ADDR)
void mWifi_set_macaddr(void){
	char sofap_mac[6] = {0x5e, 0xcf, 0x7f, 0x1a, 0xbf, 0x59};
	char sta_mac[6] = {0x5c, 0xcf, 0x7f, 0x1a, 0xbf, 0xab};
	wifi_set_opmode(STATIONAP_MODE);

	wifi_set_macaddr(SOFTAP_IF, sofap_mac);
	wifi_set_macaddr(STATION_IF, sta_mac);

}
#endif

/*
config wifi wifi station scan hook/callback 
//Before use the function must setting  TODO: add user’s own code here....
wifi_station_scan(NULL,mWifi_station_scan_router_done);

get the specify wifiAP info/RSSI
#define DEMO_AP_SSID “DEMO_AP"
struct scan_config config;
memset(&config, 0, sizeof(config));
config.ssid = DEMO_AP_SSID;//specify the wifiap name/SSID
wifi_station_scan(&config,scan_done);

*/
void mWifi_station_scan_router_done(void *arg, STATUS status){
	uint8 ssid[33];
	char temp[128];
	printf("----lx mWifi_station_scan_done----\n");
	if (status == OK){
		struct bss_info *bss_link = (struct bss_info *)arg;
		while (bss_link != NULL)
		{
			memset(ssid, 0, 33);
			if (strlen(bss_link->ssid) <= 32)
				memcpy(ssid, bss_link->ssid, strlen(bss_link->ssid));
			else
				memcpy(ssid, bss_link->ssid, 32);
			
			printf("(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",bss_link->authmode, ssid, bss_link->rssi,\
					MAC2STR(bss_link->bssid),bss_link->channel);
			bss_link = bss_link->next.stqe_next;
		}
	}else{
		printf("scan fail !!!\r\n");
	}

}


//create udp task to deal with udp
#if defined(CONFIG_USE_UDP)
void udp_task(void *pvParameters){
	int8 i = 0,ret = 0;
	uint8 udp_msg[UDP_DATA_LEN];
	LOCAL int32 sSock_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addrlen;

	printf("--lx %s enter\n", __PRETTY_FUNCTION__);
	vTaskDelay(1000/portTICK_RATE_MS);// delay 1000ms

	memset(&server_addr, 0, sizeof(server_addr));
	//create udp socket
	sSock_fd = mWifi_udp_socket_create(&server_addr, sizeof(server_addr));
	//Bind a local port
	mWifi_udp_socket_bind(sSock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	while(1){
		i++;
		//clear old data
		memset(udp_msg, 0, UDP_DATA_LEN);
		memset(&client_addr, 0, sizeof(client_addr));

		//Receive  the UDP data
		ret = mWifi_udp_data_receive(sSock_fd,udp_msg ,UDP_DATA_LEN,(struct sockaddr *)&client_addr, &client_addrlen);
		if (ret > 0) {
			printf("ESP8266 UDP task > recv %d Bytes from %s, Port %d\n",ret,inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			printf("recv data udp_msg:%s\n",udp_msg);
			//send the UDP data back this just for test,you can change the client_addr client_addrlen--this must be setted
			mWifi_udp_data_send(sSock_fd,udp_msg ,ret,(struct sockaddr *)&client_addr, client_addrlen);
		}

		/* 允许其它发送任务执行。 taskYIELD()通知调度器现在就切换到其它任务，而不必等到本任务的时
间片耗尽 */
		//taskYIELD();
	}

	close(sSock_fd);
	vTaskDelete(NULL);
}
#endif//CONFIG_USE_UDP

#if defined(CONFIG_USE_TCP_CLIENT)
#define TCP_CLIEN_BUF_LEN 100
void tcp_client_task(void *pvParameters){
	int sock_fd = 0,ret = 0;
	char buf[TCP_CLIEN_BUF_LEN] ={"fadfafdafd"};
	struct sockaddr_in server_ip;

	printf("--lx %s enter\n", __PRETTY_FUNCTION__);
	vTaskDelay(5000/portTICK_RATE_MS);// delay 1000ms
	// 1----Create a tcp socket
	sock_fd = mWifi_tcp_socket_create();

	// 2----Create a TCP connection
	ret = mWifi_tcp_client_connect_server(sock_fd,&server_ip);
	while(1){
		// 3---client send data packets via TCP communication
		ret = mWifi_tcp_client_send_data(sock_fd,buf,TCP_CLIEN_BUF_LEN);

		// 4---TCP client Receive data packets via TCP communication
		ret = mWifi_tcp_client_receive_data(sock_fd,buf,TCP_CLIEN_BUF_LEN);
		buf[ret] = 0;
		printf("receive bytes:%d %s \n",ret,buf);

			/* 允许其它发送任务执行。 taskYIELD()通知调度器现在就切换到其它任务，而不必等到本任务的时
间片耗尽 */
		//taskYIELD();
	}
	close(sock_fd);
	vTaskDelete(NULL);

}
#endif//CONFIG_USE_TCP_CLIENT

/*
config wifi connect event hook/callback 
//Before use the function must setting  TODO: add user’s own code here....
wifi_set_event_handler_cb(wifi_handle_event_cb);

*/
void wifi_handle_event_cb(System_Event_t *evt)
{
	printf("event %x\n", evt->event_id);
	switch (evt->event_id) {
		case EVENT_STAMODE_CONNECTED:
			//wifi_station_scan(NULL,mWifi_station_scan_router_done);
			#if defined(CONFIG_USE_UDP)
			xTaskCreate(udp_task, "udp", 256, NULL, 2, NULL);
			#endif
			#if defined(CONFIG_USE_TCP_CLIENT)
			xTaskCreate(tcp_client_task, "tcp client", 256, NULL, 2, NULL);
			#endif
			printf("connect to ssid %s, channel %d\n",evt->event_info.connected.ssid,evt->event_info.connected.channel);
			break;
		case EVENT_STAMODE_DISCONNECTED:
			printf("disconnect from ssid %s, reason %d\n",evt->event_info.disconnected.ssid,evt->event_info.disconnected.reason);
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			printf("mode: %d -> %d\n",evt->event_info.auth_change.old_mode,evt->event_info.auth_change.new_mode);
			break;
		case EVENT_STAMODE_GOT_IP:
			printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR"\n",	IP2STR(&evt->event_info.got_ip.ip),\
					IP2STR(&evt->event_info.got_ip.mask),IP2STR(&evt->event_info.got_ip.gw));
			break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			printf("station: " MACSTR "join, AID = %d\n",MAC2STR(evt->event_info.sta_connected.mac),\
					evt->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			printf("station: " MACSTR "leave, AID = %d\n",MAC2STR(evt->event_info.sta_disconnected.mac),evt->event_info.sta_disconnected.aid);
			break;
		default:
			break;
	}
}

//wifi init
void mWifi_mode_init(void){
	printf("-----lx mWifi_mode_init -----\n");
	/* station + soft-AP mode */
	wifi_set_opmode(WIFI_MODE_CONFIG);
	//config soft ap mode 
#if defined(CONFIG_WIFI_SOFT_AP_V01)
	mWifi_soft_ap_init();
#endif//CONFIG_WIFI_SOFT_AP_V01

	mWifi_connect_ap();

#if defined(CONFIG_GET_WIFI_MAC_ADDR)
	mWifi_get_macaddr();
#endif
#if defined(CONFIG_USE_WIFI_EVENT_CALLBACK)
	// TODO: add user’s own code here....
	wifi_set_event_handler_cb(wifi_handle_event_cb);
#endif//

}


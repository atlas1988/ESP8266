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
	mWifi_soft_ap_init();
	
	mWifi_connect_ap();

#if defined(CONFIG_GET_WIFI_MAC_ADDR)
	mWifi_get_macaddr();
#endif
#if defined(CONFIG_USE_WIFI_EVENT_CALLBACK)
	// TODO: add user’s own code here....
	wifi_set_event_handler_cb(wifi_handle_event_cb);
#endif//

}


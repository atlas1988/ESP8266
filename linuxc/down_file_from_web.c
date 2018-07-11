#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>

#define BUF_SIZE 0xF000

void get_url(char* url){
    int sock_fd;
    struct sockaddr_in server_addr;
    struct hostent *pURL = NULL;
    char myurl[BUF_SIZE];
    char *pHost = 0,*pHeader = 0,*pTemp = 0;
    char host[BUF_SIZE],GET[BUF_SIZE];
    char request[BUF_SIZE];
    static char text[BUF_SIZE];
    int i,j,ret = 0;
	unsigned int file_size = 0,file_rec = 0;
    int fd = open("user1.bin",O_CREAT | O_WRONLY | O_TRUNC,0777);
    if(fd == -1){
        printf("OPen error\n");
        exit(1);
    }
    // http server addr analysis
    memset(myurl,0,BUF_SIZE);
    memset(host,0,BUF_SIZE);
    memset(GET,0,BUF_SIZE);
    strcpy(myurl,url);
	// Separate the address(host) and the file name(GET)
	for(pHost = myurl; *pHost != '\0' && *pHost != '/' ;++pHost){
		if(*(pHost+1) != '\0' && *(pHost+1)  == '/'){
			if(*(pHost+2) != '\0' && *(pHost+2)  == '/'){
				pHost += 2;//skip "//"
				pTemp = pHost + 1;
			}
		}
	}
	// save the file name in GET
	if((int)(pHost-myurl) == strlen(myurl))
	{
		strcpy(GET,"/");//if the url do not include the file name,we should add it on the end of url"/"
	}
	else
	{
		strcpy(GET,pHost);//addr from pHost to strlen(myurl) save the file name
	}
	
	//Separate the address(host) and the file name(GET)
	*pHost = '\0';
	// save the server ip in host skip https:// and http://
	if(pTemp != NULL)
		strcpy(host,pTemp);
	else
		strcpy(host,myurl);
	
    // http Header process web URL
    bzero(request,sizeof(request));
    sprintf(request,\
            "GET %s HTTP/1.1\r\n"\
            "Host: %s\r\n"\
            "Connection: Keep-Alive\r\n"\
            "Content-Type: application/octet-stream\r\n"\
            "\r\n",GET,host);
	printf("----lx prepare the HTTP header::\n%s\n",request);
	// get the ip addr of server http
	pURL = gethostbyname(host);
    // create tcp socket
    if(-1 == (sock_fd = socket(AF_INET,SOCK_STREAM,0))){
        printf("----lx create socket failed of client!\n");
        return ;
    }
    // connect tcp server
    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = *((unsigned long*)pURL->h_addr_list[0]);
    //server_addr.sin_addr.s_addr = inet_addr("192.168.2.244");
    server_addr.sin_port = htons(80);

    if(-1 == (ret = connect(sock_fd,(struct sockaddr*)&server_addr,(socklen_t)sizeof(struct sockaddr_in)))){
        printf("----lx connect failed of client!\n");
        return ;
    }else{
        printf("connect success!\n");
    }

    // send the HTTP request
    if(-1 == (ret = send(sock_fd,request,strlen(request),0))){
        printf("----lx send request failed!\n");
        return ;
    }else{
        printf("send success send %d bytes!\n",ret);
    }
	// Parse the HTTP header
	bzero(request,sizeof(request));
	pHeader = request;
	i = 0;
	while((ret = read(sock_fd,pHeader,1)) != 0)
	{
		if (*pHeader == '\n'){
			if((pHeader - request) < 2)//if( i < 2)
				continue;
			if (*(pHeader-1) == '\r' && *pHeader == '\n' && *(pHeader-2) == '\n' )
				break;
		}
		//i ++;
		pHeader++;
	}
	//get file size 
	pHeader = strstr(request,"Content-Length:");
	if (pHeader != NULL)
	{
		pHeader = strchr(pHeader,':');
		pHeader++;
		file_size = strtoul(pHeader,NULL,10);
	}
	printf("----lx Parse the HTTP header::%d byte\n%s\n",file_size,request);
    // client receive data from server
    memset(text,0,BUF_SIZE);
    for(file_rec = 0;;){
        int rec;
        rec = recv(sock_fd,text,BUF_SIZE,0);
        if(rec <= 0)
            break;
		else
			file_rec += rec;
        write(fd,text,rec);
        printf("receive success Message:%d\n",rec);
		if(file_size == file_rec)
			break;
    }

    close(fd);
    close(sock_fd);
}

int main(int argc,char* argv[]){
    if(argc < 2){
        printf("用法:%s url网页网址\n",argv[0]);
        exit(1);
    }
    get_url(argv[1]);
    return 0;

}



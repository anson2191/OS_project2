#include<stdio.h>
#include <string>
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/socket.h> 
#include<pthread.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

int fd;

int client_sock;

char path[100];

char port[100];

//send thread
void *sendsocket(void *arg)
{
	int st = *(int *)arg;
	char sendbuffer[100]; //store sending message
	char writebuffer[100]; //store message writing into file
	while(1){
		memset(sendbuffer, 0, sizeof(sendbuffer)); //initialize sendbuffer
		printf("請輸入訊息：\n");
		scanf("%s",sendbuffer); //get message
		memset(writebuffer, 0, sizeof(writebuffer)); //initialize writebuffer
		strcat(writebuffer, sendbuffer);
		strcat(writebuffer,"\n");
		//寫入檔案
		write(fd, writebuffer, sizeof(writebuffer));
		//傳送訊息
		send(st, sendbuffer, strlen(sendbuffer), 0); 
	}

	return NULL;
}

//accept thread
void *recvsocket(void *arg)
{
	int st = *(int *)arg;
	char receivebuffer[100]; 
	char writebuffer[100];
	int n; 
	while(1){
		//read return data from server
		memset(receivebuffer, 0, sizeof(receivebuffer));
		n = recv(st, receivebuffer, sizeof(receivebuffer), 0);

		//whether communicate is end
		if(n <= 0)
			break;
		memset(writebuffer, 0, sizeof(writebuffer));
		strcat(writebuffer, receivebuffer);
		strcat(writebuffer, "\n");

		//write file
		write(fd, writebuffer, sizeof(writebuffer));

		//print message
		printf("%s\n", receivebuffer);
	}

	return NULL;
}

int main()
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	//AF_INET: use IPv4
	//SOCK_STREAM:TCP protocol
	//0:default, let kernel choose the protocol type of socket	
	
	struct sockaddr_in serv_addr; //get server socket address
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //IPv4 structure
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //turn IP into decimal
	serv_addr.sin_port = htons(6666); //host to network	
	
	//connet to server. if success, return 0
	client_sock = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(client_sock ==0 ){
		printf("Connect to server sucessfully\n");
	}

	//create a new structure
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t len = sizeof(client_addr); //socklen
	int ti = getsockname(sock, (struct sockaddr*)&client_addr, &len);
	sprintf(port, "%d", client_addr.sin_port);
	strcat(path, "./usernote/");
	strcat(path, port);

	//opent file
	//char*  path_s = "./usernote/";
	//const char* path_c = path_s.c_str();
	mkdir("./usernote",0777);
	fd = open(path, O_CREAT|O_EXCL|O_WRONLY|O_APPEND|O_NONBLOCK);
	if(fd == -1){
		printf("fail to open file\n");
	}

	//create send,receive two thread
	pthread_t thrd1, thrd2;
	pthread_create(&thrd1, NULL, sendsocket, &sock);
	pthread_create(&thrd2, NULL, recvsocket, &sock);
	pthread_join(thrd1, NULL);
	pthread_join(thrd2, NULL);

	//close fill
	close(fd);

	close(sock);

	return 0;
}

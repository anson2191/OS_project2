#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<arpa/inet.h> 
#include<sys/socket.h> 
#include<netinet/in.h> 
#include<unistd.h> 
#include<pthread.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<cstdlib>
#include<signal.h>

//stuct a new data structure for sending info to thread
struct threadinfo{
	int my_sock; //save socket
	int my_port; //save port numver
};

int usernum = 0; //number of users
int sock_array[10]; //socket array, max num = 10

int fd; //open file

//sending thread
void *recvsocket(void *arg) //err1
{
	//printf("err\n");
	//receive structure
	struct threadinfo *thread = (threadinfo *)arg; //create a threadinfo ptr to use the info
	int st = thread->my_sock;
	int port = thread->my_port;
	char receivebuffer[100]; //save message from client
	int i, n;
	int length = sizeof(sock_array)/sizeof(sock_array[0]);
	char sendbuffer[100];
	char writebuffer[100];
	while(1){
		//receive return message from client
		memset(receivebuffer, 0, sizeof(receivebuffer));
		n = recv(st, receivebuffer, sizeof(receivebuffer), 0); //receive message from client

		//whether communication is over
		if(n <= 0)
			break;
		printf("Message from%d:%s \n", port,receivebuffer);

		//save the chat record to file
		memset(writebuffer, 0, sizeof(writebuffer));
		sprintf(writebuffer, "%d", port);
		strcat(writebuffer, ":");
		strcat(writebuffer, receivebuffer);
		strcat(writebuffer, "\n");
		write(fd, writebuffer, sizeof(writebuffer));

		//send message to client except sending guy
		memset(sendbuffer, 0, sizeof(sendbuffer));
		sprintf(sendbuffer, "User %d", port);
		strcat(sendbuffer, " say : ");
		strcat(sendbuffer, receivebuffer);
		for(i=0; i<usernum; i++) {
			if(sock_array[i] != st) {
				send(sock_array[i], sendbuffer, strlen(sendbuffer), 0);
			}	
		}
	}
	return NULL;
}

int main()
{
	//prevent SIGPIPE to terminate program
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error\n");
	}


	//get server socket
	int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(6666);	

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
		perror("bind failed");
		exit(1);
	}

	//waiting for user require
	printf("listening on port:6666\n");

	//cread a new sockaddr_int for accept client socket
	struct sockaddr_in client_addr;
	pthread_t thrd[10]; //max user = 10

	int client_sock;
	socklen_t len = sizeof(client_addr);

	struct threadinfo my_thread;
	struct threadinfo thread_array[10];

	int i;
	char portbuffer[100];

	//start listening to 10 users
	listen(serv_sock, 10);

	fd = open("./note.txt", O_CREAT|O_WRONLY|O_APPEND);
	while(1) {
		client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &len);
		if(client_sock < 0) {
			perror("connet failed!");
			exit(1);
		}

		printf("Hello %d, Welcome to the chatroom ~\n", client_addr.sin_port);

		//send message to all users
		memset(portbuffer, 0, sizeof(portbuffer));
		sprintf(portbuffer, "%d", client_addr.sin_port);
		strcat(portbuffer, " enters the chatroom ~");
		
		for(i=0; i<usernum; i++){
			send(sock_array[i], portbuffer, strlen(portbuffer), 0);
		}
	
		my_thread.my_sock = client_sock;
		my_thread.my_port = client_addr.sin_port;
		thread_array[usernum] = my_thread;
	
		sock_array[usernum] = client_sock;
	
		//create sending thread
		pthread_create(&thrd[usernum], NULL, recvsocket, &thread_array[usernum]);
		usernum++;

		//printf("usernum: %d\n", usernum);
	}

	close(fd);
	close(client_sock);
	close(serv_sock);

	return 0;
}	

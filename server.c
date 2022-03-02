//server
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include <signal.h>

#define MAXCLIENTNUM 20
#define BUFFERSIZE 300

#define NAME 20
#define TEXT 300
#define SV_NAME "server"

void error(const char * msg);
void * send_all_client(void * arg);//서버가 모든 클라이언트에게 메세지 뿌리기

int clientSocket[MAXCLIENTNUM];
pthread_mutex_t mutex;
int clientNum;

struct texts {
   char name[NAME];
   char texts[TEXT];
};

void signal_int(int sig){
	struct texts msg_box;

	strcpy(msg_box.name,SV_NAME);
	strcpy(msg_box.texts,"exit");
	for (int i = 0; i < clientNum; i++)
         write(clientSocket[i], &msg_box, sizeof(msg_box));
	exit(1);
}

int main(int argc, char * argv[])
{

   if (argc < 2)
   {
      fprintf(stderr, "port not provieded\n");
      exit(1);
   }

   int sockfd, newsockfd, portno, n;
   char buffer[255];

   struct sockaddr_in serv_addr, cli_addr;
   socklen_t clilen;
   pthread_t threadId;

   pthread_mutex_init(&mutex, NULL);
   sockfd = socket(AF_INET, SOCK_STREAM, 0);//소켓 생성
   if (sockfd < 0)
   {
      error("error opening socket.");
   }
   bzero((char*)&serv_addr, sizeof(serv_addr));//초기화
   portno = atoi(argv[1]);

   serv_addr.sin_family = AF_INET;//set socket type
   serv_addr.sin_addr.s_addr = INADDR_ANY;//ip address provide
   serv_addr.sin_port = htons(portno);//set portnum

   if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)//set socket ip address and portnum
      error("Binding fail");

   listen(sockfd, 5);//check client access

	signal(SIGINT,signal_int);

   while (1)
   {
      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);//accept client

      pthread_mutex_lock(&mutex);//임계영역 들어가기전 락 걸기
      clientSocket[clientNum++] = newsockfd;
      pthread_mutex_unlock(&mutex);//락 해제

      pthread_create(&threadId, NULL, send_all_client, (void*)&newsockfd);//쓰레드 생성 (서버가 모든 클라이언트에게 메세지뿌리기)
      pthread_detach(threadId);//쓰레드 반환

      printf("chatNum : %d MaxNum : 20\n", clientNum);
   }
   close(sockfd);
   return 0;



}

void * send_all_client(void * arg)
{
   int clientSockfd = *((int*)arg);
   char message[BUFFERSIZE];
   int i;
   int readLength = 0;
	struct texts msg_box;

   while ((readLength = read(clientSockfd, &msg_box, sizeof(msg_box))) != 0)
   {
      int i;
      pthread_mutex_lock(&mutex);//임계영역 들어가기전 락 걸기
      //임계영역
      for (i = 0; i < clientNum; i++)
         write(clientSocket[i], &msg_box, sizeof(msg_box));

      pthread_mutex_unlock(&mutex);// 락 풀기
   }

   pthread_mutex_lock(&mutex);//임계영역 들어가기전 락 걸기
   for (i = 0; i < clientNum; i++)
   {
      if (clientSockfd == clientSocket[i])
      {
         for (; i < clientNum - 1; i++)
            clientSocket[i] = clientSocket[i + 1];

         break;
      }
   }
   clientNum--;
   pthread_mutex_unlock(&mutex);// 락 풀기
   close(clientSockfd);
   return NULL;
}

void error(const char * msg)
{
   perror(msg);
   exit(1);
}

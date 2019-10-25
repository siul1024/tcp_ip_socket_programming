#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>

#define BUF_SIZE 100
#define MAX_CLNT 100
#define MAX_IP 30
#define GREEN "\x1b[32m"  //connect
#define YELLOW "\x1b[33m"	//disconnect
#define RED "\x1b[31m"	//error
#define COLORRESET "\x1b[0m"

void* handle_clnt(void *arg);
void send_msg(char *msg, int len);
void menu();

//접속정보 저장을 위한 구조체
typedef struct clientinfo {
	int sockNUM; //클라이언트가 접속한 소켓 넘버
	char *connected_time;  //접속시간
	struct sockaddr_in serv_adr, clnt_adr;
} clientINFO;

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
//로컬타임
struct tm *t;
char *clientip;
int port = 5000;

int main() {
	char *start = "\x1b[36m Welcome! Connected server \n\x1b[0m";
	int serv_sock;
	int clnt_adr_sz;
	pthread_t t_id;
	clientINFO *CLIENT;

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&CLIENT->serv_adr, 0, sizeof(CLIENT->serv_adr));
	CLIENT->serv_adr.sin_family = AF_INET;
	CLIENT->serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	CLIENT->serv_adr.sin_port = htons(port);

	if (bind(serv_sock, (struct sockaddr*) &CLIENT->serv_adr,
			sizeof(CLIENT->serv_adr)) == -1) {
		printf(RED "bind() error\n"COLORRESET);
		exit(1);
	}
	if (listen(serv_sock, 5) == -1) {
		printf(RED"listen() error\n"COLORRESET);
		exit(1);
	}
	//서버 메뉴 호출
	menu();
	while (1) {
		time_t timer = time(NULL);
		t = localtime(&timer);
		clnt_adr_sz = sizeof(CLIENT->clnt_adr);
		//구조체에 소켓정보 저장
		CLIENT->sockNUM = accept(serv_sock,
				(struct sockaddr*) &CLIENT->clnt_adr, &clnt_adr_sz);
		//accept time 저장
		sprintf(CLIENT->connected_time, "%d-%d-%d %d:%d", t->tm_year + 1900,
				t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);

		//client ip 변수 생성(사용하기 편하게하려고 지정)
		clientip = inet_ntoa(CLIENT->clnt_adr.sin_addr);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = CLIENT->sockNUM;
		send(CLIENT->sockNUM, start, strlen(start), 0);
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*) &CLIENT->sockNUM);
		pthread_detach(t_id);
		printf(GREEN" Connected client IP(socket) : %s(%d)", clientip,
				CLIENT->sockNUM);
		printf("(%s)\n"COLORRESET, CLIENT->connected_time);
		//접속중인 소켓 정보 출력
		printf("connected socket number(total:%2d) :", clnt_cnt);
		for (int i = 0; i < clnt_cnt; i++)
			printf("%d, ", clnt_socks[i]);
		printf("\n");
	}
	close(serv_sock);
	return 0;
}

void* handle_clnt(void *arg) {
	int clnt_sock = *((int*) arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];

	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
		send_msg(msg, str_len);

	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) {
		if (clnt_sock == clnt_socks[i]) {
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	printf(YELLOW" Disconnected client IP(socket) : %s(%d)", clientip,
			clnt_sock);
	printf("(%d-%d-%d %d:%d)\n"COLORRESET, t->tm_year + 1900, t->tm_mon + 1,
			t->tm_mday, t->tm_hour, t->tm_min);
	printf("connected sockets number(total:%2d) :", clnt_cnt);
	for (int i = 0; i < clnt_cnt; i++)
		printf("%d, ", clnt_socks[i]);
	printf("\n");
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void send_msg(char *msg, int len) {
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void menu() {
	printf(" ------------- chat server -------------\n");
	printf(" server port    : %d\n", port);
	printf(" max connection : %d\n", MAX_CLNT);
	printf(" ----------------- log -----------------\n\n");
}

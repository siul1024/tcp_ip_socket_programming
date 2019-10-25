/*	멀티스레드사용
 * 	명령 메뉴 옵션 -> 1닉네임 변경 2채팅창 클리어
 * 	접속정보 전송
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>

#define YELLOW "\x1b[33m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define CYAN "\x1b[36m" //name
#define COLORRESET "\x1b[0m"
#define BUF_SIZE 100
#define NORMAL_SIZE 20

void* send_msg(void *arg);
void* recv_msg(void *arg);
void getServerIP();
void menu();
void changeName();
void menuoptions();

char name[NORMAL_SIZE] = "[unnamed]";
char msg_form[NORMAL_SIZE];
char serv_time[NORMAL_SIZE];
char msg[BUF_SIZE];
char serv_port[NORMAL_SIZE];
char clnt_ip[20];


int main(int argc, char *argv[]) {
	getServerIP();
	int sock;
	struct sockaddr_in serv_addr, clnt_addr;
	pthread_t snd_thread, rcv_thread;
	void *thread_return;

	if (argc != 4) {
		printf(RED" Usage : %s <ip> <port> <name>\n"COLORRESET, argv[0]);
		exit(1);
	}
	//시간
	struct tm *t;
	time_t timer = time(NULL);
	t = localtime(&timer);


	//clnt_ip = getServerIP();

	sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year + 1900, t->tm_mon + 1,
			t->tm_mday, t->tm_hour, t->tm_min);

	sprintf(name, "[%s]", argv[3]);

	sprintf(serv_port, "%s", argv[2]);
	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));


	//clnt_ip = getServerIP();

	if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		printf(RED " connect() error\n"COLORRESET);
		exit(1);
	}
	//메뉴호출
	menu();
	//멀티스레드
	pthread_create(&snd_thread, NULL, send_msg, (void*) &sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*) &sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}

void* send_msg(void *arg) {
	int sock = *((int*) arg);
	char name_msg[NORMAL_SIZE + BUF_SIZE];
	char myInfo[BUF_SIZE];

	//접속정보 전송
	sprintf(myInfo, CYAN"%s"GREEN"님이 입장하셨습니다. (%s)\n"COLORRESET, name, clnt_ip);
	write(sock, myInfo, strlen(myInfo));

	while (1) {
		fgets(msg, BUF_SIZE, stdin);

		//명령모드 호출 !menu
		if (!strcmp(msg, "!menu\n")) {
			menuoptions();
			continue;
		}

		else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
			sprintf(name_msg, CYAN"%s"YELLOW"님이 퇴장하셨습니다. (%s)\n"COLORRESET, name, clnt_ip);
			write(sock, name_msg, strlen(name_msg));
			close(sock);
			exit(0);
		}

		// 메세지 전송
		sprintf(name_msg, CYAN "%s " COLORRESET "%s", name, msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}

void* recv_msg(void *arg) {
	int sock = *((int*) arg);
	char name_msg[NORMAL_SIZE + BUF_SIZE];
	int str_len;

	while (1) {
		str_len = read(sock, name_msg, NORMAL_SIZE + BUF_SIZE - 1);
		if (str_len == -1)
			return (void*) -1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
	}
	return NULL;
}

void menuoptions() {
	int select;
	printf(YELLOW" ----------------- menu ----------------\n");
	printf(" 1. 닉네임 변경\n");
	printf(" 2. 채팅로그 지우기\n");
	printf(" 취소하려면 아무 키나 누르세요\n");
	printf(" ---------------------------------------\n"COLORRESET);
	printf("\n >> ");
	scanf("%d", &select);
	getchar();
	switch (select) {
	//닉네임 변경
	case 1:
		changeName();
		break;
	//clear log
	case 2:
		menu();
		break;


	default:
		printf(YELLOW" cancel.\n"COLORRESET);
		break;
	}
}

//닉네임 변경
void changeName() {
	char nameTemp[100];
	printf(" new name >>");
	scanf("%s", nameTemp);
	sprintf(name, "[%s]", nameTemp);
	printf(GREEN"\tComplete.\n"COLORRESET);
	getchar();
}

void menu() {
	system("clear");
	printf(YELLOW" ------------- chat client -------------\n");
	printf(" server port : %s \n", serv_port);
	printf(" chat name   : %s \n", name);
	printf(" accept time : %s \n", serv_time);
	printf(" ----------------- menu ----------------\n");
	printf(" 명령모드\"!menu\"\n");
	printf(" 1. 닉네임 변경\n");
	printf(" 2. 채팅로그 지우기\n");
	printf(" exit(q/Q)\n\n");
	printf(" ---------------------------------------\n"COLORRESET);
}

void getServerIP()
{
    char myip[20];
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    const char* kGoogleDnsIp = "8.8.8.8";
    int kDnsPort = 53;

    struct sockaddr_in serv;
    struct sockaddr_in host_name;

    memset(&serv, 0, sizeof(serv));

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    if( connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0 ) printf("[-] sock connect for get ipaddr faild!\n");

    socklen_t host_len = sizeof(host_name);
    if( getsockname(sockfd, (struct sockaddr *)&host_name, &host_len) < 0 ) printf("[-] getsockname faild!\n");

    inet_ntop(AF_INET, &host_name.sin_addr, myip, sizeof(myip));
    close(sockfd);
    strncpy(clnt_ip, myip, sizeof(myip));

}


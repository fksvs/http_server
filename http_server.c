#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFF_SIZE 4096
#define BACKLOG 10

char *listen_address = "127.0.0.1";
int listen_port = 8080;
int sockfd;

void close_server()
{
	int status = 0;
	pid_t wpid;

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);

	while ((wpid = wait(&status)) > 0);
	exit(0);
}

void die(const char *err){
        perror(err);
        close_server();
}

void close_connection(int client_sock)
{
	shutdown(client_sock, SHUT_RDWR);
	close(client_sock);
}

void send_data(int client_sock)
{
	char *data =
		"HTTP/1.1 200 OK\r\n\r\n<html><body><h1> HTTP Server</h1></body></html>";
	if (send(client_sock, data, strlen(data), 0) < 0)
		close_connection(client_sock);
}

void listen_data(int client_sock)
{
	char buffer[BUFF_SIZE];

	while (1) {
		memset(buffer, 0, BUFF_SIZE);
		if (recv(client_sock, buffer, BUFF_SIZE, 0) <= 0)
			break;
		else {
			printf("%s\n", buffer);
			send_data(client_sock);
			break;
		}
	}

	close_connection(client_sock);
	exit(0);
}

void listen_connection()
{
	struct sockaddr_in conn;
	int client_sock, len = sizeof(struct sockaddr_in);

	while (1) {
		client_sock = accept(sockfd, (struct sockaddr *)&conn,
				     (socklen_t *)&len);
		if (client_sock < 0)
			close(client_sock);
		else if (fork() == 0)
			listen_data(client_sock);
		close(client_sock);
	}
}

int init_server()
{
	signal(SIGINT, close_server);
	signal(SIGPIPE, SIG_IGN);

	struct sockaddr_in server;
	int enable = 1;

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(listen_address);
	server.sin_port = htons(listen_port);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("[ERROR] [socket] ");
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		die("[ERROR] [setsockopt] ");
	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		die("[ERROR] [bind] ");
	if (listen(sockfd, BACKLOG) < 0)
		die("[ERROR] [listen] ");

	return 1;
}

void parser(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "a:p:h")) != -1) {
		switch (opt) {
		case 'a':
			listen_address = optarg;
			break;
		case 'p':
			listen_port = atoi(optarg);
			break;
		case 'h':
			printf("usage : https (-a listen address)\
 (-p listen port)\n");
			exit(0);
		}
	}
}

int main(int argc, char *argv[])
{
	parser(argc, argv);

	if (init_server() != 1)
		return 0;
	listen_connection();

	return 0;
}

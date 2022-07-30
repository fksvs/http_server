#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BUFF_SIZE 4096
#define BACKLOG 10

char data[BUFF_SIZE], html_data[BUFF_SIZE], *file_path = NULL;

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

void die(const char *err)
{
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

void accept_connection()
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

void init_server()
{
	signal(SIGINT, close_server);
	signal(SIGPIPE, SIG_IGN);

	struct sockaddr_in server;
	int fd,enable = 1;

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

        if ((fd = open(file_path, O_RDONLY)) == -1)
                die("[ERROR] [open] ");
        if (read(fd, html_data, BUFF_SIZE) == -1)
                die("[ERROR] [read] ");
        close(fd);

        sprintf(data, "HTTP/1.1 200 OK\r\n\r\n%s",html_data);

        accept_connection();
}

void parser(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "a:p:d:h")) != -1) {
		switch (opt) {
		case 'a':
			listen_address = optarg;
			break;
		case 'p':
			listen_port = atoi(optarg);
			break;
                case 'd':
                        file_path = optarg;
                        break;
		case 'h':
			printf("usage : https -a (host address) -p (host port) -d (html file)\n");
			exit(0);
		}
	}
}

int main(int argc, char *argv[])
{
	parser(argc, argv);
	init_server();

	return 0;
}

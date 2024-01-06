#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 12345

static struct sockaddr_in serv_addr;
static int sock = 0;
static struct sockaddr_in address;
static int server_fd, new_socket, opt = 1, addrlen = sizeof(address);

void server();
void readClient(int *value);
void sendClient(int *value);
void sendClientll(long long int *value);
void sendClientull(unsigned long long int *value);
void sendClientFile(char* FILE_PATH);

void client();
void readServer(int *value);
void sendServer(int *value);
void readServerll(long long int *value);
void readServerull(unsigned long long int *value);
void readServerFile(char* FILE_PATH);

#endif
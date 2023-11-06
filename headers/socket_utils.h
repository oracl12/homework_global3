#pragma once 

#ifdef __WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#endif

#include <iostream>
#include <ctime>
#include "other.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 46
#endif

#define SOCKET_ERROR (-1)
#define IP_SERVER "127.0.0.1"
#define PROXY_PORT 8088
#define SERVER_PORT 5555
#define MAX_BUFFER_SIZE 32

#pragma pack(1)
struct DataPackage {
    unsigned long number;
    unsigned int checksum;
    char buffer[MAX_BUFFER_SIZE];
};
#pragma pack()

class SocketUtil
{
public:
    virtual void forceCleanUpProgram();

    void WSAStartUp();

    void cleanupWinsock();

    int initsSocket();

    sockaddr_in getServerAddr(int port);

    void bindSocket(int socket, int port);

    void listenToSocket(int socket);

    void connectToSocket(int socket, int port);

    void closeSocket(int socket);

    unsigned int calculateXORChecksum(const char *data, size_t dataLength, unsigned long number);
};

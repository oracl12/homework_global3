#pragma once 

#include <winsock2.h>
#include <windows.h>
#include <iostream>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 46
#endif

#define SOCKET_ERROR (-1)
#define IP_SERVER "127.0.0.1"
#define PROXY_PORT 5656
#define SERVER_PORT 5555

#pragma pack(1)
struct DataPackage {
    unsigned long number;
    unsigned int checksum;
    char buffer[32];
};
#pragma pack()

class SocketUtil
{
public:
    void WSAStartUp();

    void cleanupWinsock();

    int initsSocket();

    sockaddr_in getServerAddr(int port);

    void bindSocket(int socket, int port);

    void listenToSocket(int socket);

    void connectToSocket(int socket, int port);

    unsigned int calculateXORChecksum(const char *data, size_t dataLength, unsigned long number);
};

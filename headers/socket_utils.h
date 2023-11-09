#pragma once 

#ifdef __WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#endif

#include <iostream>
#include <ctime>
#include <vector>
#include <thread>
#include <functional>
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
    void WSAStartUp();

    void cleanupWinsock();

    int initsSocket();

    sockaddr_in getServerAddr(int port);

    void bindSocket(int socket, int port);

    void listenToSocket(int socket);

    int acceptConnection(int socket);

    void connectToSocket(int socket, int port);

    static void closeSocket(int socket);

    unsigned int calculateXORChecksum(const char *data, size_t dataLength, unsigned long number);

    static const char* SOCKET_ERRORS_TEXT[7];

    enum SOCKET_ERRORS {
        WINSOCK_INIT_FAILURE,
        INIT_FAILURE,
        BINDING_ERROR,
        LISTENING_ERROR,
        ACCEPTANCE_FAULURE,
        CONNECTION_FAILURE,
        CLEANUP_WINSOCK_ERROR
    };
};

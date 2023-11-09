#include "../headers/socket_utils.h"

const char* SocketUtil::SOCKET_ERRORS_TEXT[] = {
        "WINSOCK_INIT_FAILURE",
        "INIT_FAILURE",
        "BINDING_ERROR",
        "LISTENING_ERROR",
        "ACCEPTANCE_FAILURE",
        "CONNECTION_FAILURE",
        "CLEANUP_WINSOCK_ERROR"

};
void SocketUtil::WSAStartUp()
{
    #ifdef __WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        throw SOCKET_ERRORS::WINSOCK_INIT_FAILURE;
    }
    #endif
}

int SocketUtil::initsSocket()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        throw SOCKET_ERRORS::INIT_FAILURE;
    }
    return clientSocket;
};

sockaddr_in SocketUtil::getServerAddr(int port)
{
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(IP_SERVER);
    return serverAddr;
}

void SocketUtil::bindSocket(int socket, int port)
{
    sockaddr_in serverAddr = getServerAddr(port);

    if (bind(socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        throw SOCKET_ERRORS::BINDING_ERROR;
    }
};

void SocketUtil::listenToSocket(int socket)
{
    if (listen(socket, 1) == SOCKET_ERROR)
    {
        throw SOCKET_ERRORS::LISTENING_ERROR;
    }
};

int SocketUtil::acceptConnection(int socket)
{
    sockaddr_in clientAddr;

    #ifdef __WIN32
    int clientAddrLen = sizeof(clientAddr);
    #else
    socklen_t clientAddrLen = sizeof(clientAddr);
    #endif
    int clientSocket = accept(socket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET)
    {
        throw SOCKET_ERRORS::ACCEPTANCE_FAULURE;
    }
    return clientSocket;
}

void SocketUtil::connectToSocket(int socket, int port)
{
    sockaddr_in serverAddr = getServerAddr(port);
    int tryiesCount = 0;
    while (connect(socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        if (tryiesCount > 6) {
            throw SOCKET_ERRORS::CONNECTION_FAILURE;
        }
        
        std::cout << "SocketUtils: Try connecting to server again..." << std::endl;
        SleepS(1500);
        tryiesCount++;
    }
};

void SocketUtil::closeSocket(int socket)
{
#ifdef __WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

void SocketUtil::cleanupWinsock() {
    #ifdef __WIN32
    if (WSACleanup() == SOCKET_ERROR) {
        throw SOCKET_ERRORS::CLEANUP_WINSOCK_ERROR;
    }
    #endif
}

unsigned int SocketUtil::calculateXORChecksum(const char *data, size_t dataLength, unsigned long number) {
    unsigned int checksum = 0;

    for (size_t i = 0; i < dataLength; ++i) {
        checksum ^= data[i];
    }

    unsigned char *numberBytes = reinterpret_cast<unsigned char *>(&number);
    for (size_t i = 0; i < sizeof(unsigned long); ++i) {
        checksum ^= numberBytes[i];
    }

    return checksum;
}
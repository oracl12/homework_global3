#include "../headers/socket_utils.h"

void SocketUtil::WSAStartUp()
{
    #ifdef __WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        exit(1);
    }
    #endif
}

int SocketUtil::initsSocket()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        exit(1);
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
        std::cerr << "Error binding socket." << std::endl;
        closesocket(socket);
        WSACleanup();
        exit(1);
    }
};

void SocketUtil::listenToSocket(int socket)
{
    if (listen(socket, 1) == SOCKET_ERROR)
    {
        std::cerr << "Error listening for connections." << std::endl;
        closesocket(socket);
        WSACleanup();
        exit(1);
    }
};

void SocketUtil::connectToSocket(int socket, int port)
{
    sockaddr_in serverAddr = getServerAddr(port);
    int tryiesCount = 0;
    while (connect(socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        if (tryiesCount > 5) {
            std::cerr << "Error connecting to the server." << std::endl;
            closesocket(socket);
            WSACleanup();
            exit(1);
        }
        
        std::cout << "Try connecting to server again..." << std::endl;
        Sleep(1000);
    }
};

void SocketUtil::cleanupWinsock() {
    #ifdef __WIN32
    if (WSACleanup() == SOCKET_ERROR) {
        std::cerr << "WSACleanup failed: " << WSAGetLastError() << std::endl;
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
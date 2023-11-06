#include "headers/socket_utils.h"

class Server : private SocketUtil
{
public:
    Server() {
        std::clog << "STARTING SERVER" << std::endl;

        WSAStartUp();

        serverSocket = initsSocket();

        bindSocket(serverSocket, SERVER_PORT);

        listenToSocket(serverSocket);

        std::clog << "Server is listening on port " << SERVER_PORT << "..." << std::endl;
    };

    void runServer() {
        std::cout << "WE ARE IN RUNSERVER" << std::endl;
        sockaddr_in clientAddr;

        #ifdef __WIN32
        int clientAddrLen = sizeof(clientAddr);
        #else
        socklen_t clientAddrLen = sizeof(clientAddr);
        #endif
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Error accepting connection." << std::endl;
            SleepS(3000);
            return;
        }

        std::clog << "Client connected." << std::endl;
    }

    void receiveAndCheckData() {
        std::cout << "WE ARE RECEIVING" << std::endl;
        while (1)
        {
            char buffer[sizeof(DataPackage)];
            DataPackage receivedData;

            int bytesReceived = recv(clientSocket, buffer, sizeof(DataPackage), 0);
            
            if (bytesReceived == -1) {
                std::cerr << "Failed to receive data package." << std::endl;
                closeSocket(clientSocket);
                return;
            }

            memcpy(&receivedData, buffer, sizeof(DataPackage));

            std::cout << "Received buffer: " << receivedData.buffer << std::endl;
            std::cout << "Received number: " << receivedData.number << std::endl;
            std::cout << "Received checksum: " << receivedData.checksum << std::endl;

            char responceBuffer[sizeof(bool) + 1];

            // check if matches
            if (calculateXORChecksum(receivedData.buffer, strlen(receivedData.buffer), receivedData.number) != receivedData.checksum) {
                strcpy(responceBuffer, "0");
                std::cout << "Checksum doesnt match" << std::endl;
            } else {
                strcpy(responceBuffer, "1");
            }

            if (send(clientSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "Successfully sent responce back" << std::endl;
            } else {
                std::cout << "Client has no responce" << std::endl;
                break;
            };

        }
    };

    void forceCleanUpProgram() override
    {
        std::cout << "SERVER: SHUTTING DOWN FORCEFULLY" << std::endl;
        closeSocket(serverSocket);
        closeSocket(clientSocket);
        cleanupWinsock();
        exit(1);
    }

    ~Server(){
        std::cout << "SERVER: SHUTTING DOWN NORMALLY" << std::endl;
        closeSocket(serverSocket);
        closeSocket(clientSocket);
        cleanupWinsock();
    };

	Server(const Server &) = delete;

	Server &operator=(const Server &) = delete;

private:
    int serverSocket;
    int clientSocket;

    bool checkIfCheckSumIsCorrect(){
        return true;
    }
};

int main(){
    Server server;
    server.runServer();
    server.receiveAndCheckData();
    return 0;
}
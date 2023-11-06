#include "headers/socket_utils.h"

class Proxy : private SocketUtil
{
public:
    Proxy()
    {
        WSAStartUp();
        proxySocket = initsSocket();
        bindSocket(proxySocket, PROXY_PORT);
    }

    void listenClient()
    {
        std::clog << "STARTING PROXY SERVER" << std::endl;

        listenToSocket(proxySocket);

        std::clog << "Proxy is listening on port " << PROXY_PORT << "..." << std::endl;
        sockaddr_in clientAddr;
        #ifdef __WIN32
        int clientAddrLen = sizeof(clientAddr);
        #else
        socklen_t clientAddrLen = sizeof(clientAddr);
        #endif
        clientSocket = accept(proxySocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Error accepting connection." << std::endl;
            return;
        }

        std::clog << "Client connected." << std::endl;
    }

    void connectToServer()
    {
        std::cout << "STARTING CLIENT SIDE" << std::endl;

        serverSocket = initsSocket();
        connectToSocket(serverSocket, SERVER_PORT);

        std::cout << "CLIENT-SIDE: Connected to the server." << std::endl;
    }

    void passingData(){
        char buffer[sizeof(DataPackage)];
        char responceBuffer[sizeof(bool) + 1];
        int bytesRead;
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        {
            std::cout << "Has received data from client" << std::endl;

            if (shouldIChangeVariable()) {
                makeRandomChanges(buffer, sizeof(DataPackage));
            }

            if (send(serverSocket, buffer, bytesRead, 0) > 0)
            {
                std::cout << "Success sent package to server" << std::endl;
            } else {
                std::cout << "Server has no responce" << std::endl;
                break;
            };

            // start receiving from server
            if (recv(serverSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "Has received responce from server" << std::endl;
            } else {
                std::cout << "Server has no responce" << std::endl;
            }

            // start sending to client
            if (send(clientSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "Successfully pass server responce to client" << std::endl;
            } else {
                std::cout << "Client has no responce" << std::endl;
            }

            std::cout << "------------ Iteration end ------------" << std::endl;
        }
    }

    void forceCleanUpProgram() override
    {
        std::cout << "SERVER: SHUTTING DOWN FORCEFULLY" << std::endl;
        closeSocket(clientSocket);
        closeSocket(serverSocket);
        closeSocket(proxySocket);
        cleanupWinsock();
        exit(1);
    }

    ~Proxy()
    {
        std::cout << "SERVER: SHUTTING DOWN NORMALLY" << std::endl;
        closeSocket(clientSocket);
        closeSocket(serverSocket);
        closeSocket(proxySocket);
        cleanupWinsock();
    }
private:
    int clientSocket;
    int serverSocket;
    int proxySocket;

    void makeRandomChanges(char* buffer, int length) {
        srand(time(0));
        int index = rand() % length;
        buffer[index] = static_cast<char>((buffer[index] + rand() % 26) % 127);
        std::cout << "Making changes: Changed buffer at index " << index << std::endl;
    }

    bool shouldIChangeVariable(){
        srand(time(0));
        return (rand() % 5 + 1) == 3; // probability 1/5
    }
};

int main()
{
    Proxy proxy;
    proxy.listenClient();

    proxy.connectToServer();

    proxy.passingData();

    return 0;
}

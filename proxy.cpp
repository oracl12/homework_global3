#include "headers/socket_utils.h"

#include <ctime>

// make thread, one which will receive packages and pushes them to queue, second which sends to server and pops first out

class Proxy : private SocketUtil
{
public:
    Proxy()
    {
        WSAStartUp();
    }

    void setUpProxy()
    {
        proxySocket = socket(AF_INET, SOCK_STREAM, 0);
        if (proxySocket == -1)
        {
            std::cerr << "Proxy server socket creation failed." << std::endl;
            exit(1);// TODO: remove exits and handle end of program better
        }

        sockaddr_in proxyAddr;
        proxyAddr.sin_family = AF_INET;
        proxyAddr.sin_addr.s_addr = INADDR_ANY;
        proxyAddr.sin_port = htons(PROXY_PORT);

        std::cout << "Binding proxy server" << std::endl;
        if (bind(proxySocket, (struct sockaddr *)&proxyAddr, sizeof(proxyAddr)) == -1)
        {
            std::cerr << "Proxy server binding failed." << std::endl;
            exit(1);// TODO: remove exits and handle end of program better
        }
    }

    void listenClient()
    {
        std::clog << "STARTING PROXY SERVER" << std::endl;

        listenToSocket(proxySocket);

        std::clog << "Proxy is listening on port " << PROXY_PORT << "..." << std::endl;
        sockaddr_in clientAddr;
        // place this to seperate function
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
        static int counterRandom = 0;
        int bytesRead;
        std::cout << "Tryinjg to receive data from client" << std::endl;

        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        {
            if (++counterRandom > 25) {
                makeRandomChanges(buffer, sizeof(DataPackage));
            }

            std::cout << "passing data" << std::endl;
            if (send(serverSocket, buffer, bytesRead, 0) > 0)
            {
                std::cout << "Success" << std::endl;
            } else {
                std::cout << "Server has no responce" << std::endl;
                break;
            };
        }
    }

    ~Proxy()
    {
        closeSocket(clientSocket);
        closeSocket(serverSocket);
        closeSocket(proxySocket);
    }
private:
    int clientSocket;
    int serverSocket;
    int proxySocket;

    void makeRandomChanges(char* buffer, int length) {
        srand(static_cast<unsigned int>(time(0)));
        int index = rand() % length;
        buffer[index] = static_cast<char>((buffer[index] + rand() % 26) % 127);
        std::cout << "Making changes: Changed buffer at index " << index << std::endl;
    }

};

int main()
{
    Proxy proxy;
    proxy.setUpProxy();

    proxy.listenClient();

    proxy.connectToServer();

    proxy.passingData();

    return 0;
}

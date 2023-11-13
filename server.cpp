#include "headers/socket_utils.h"
#include "headers/ctrl_handler.h"

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

        clients = new std::vector<std::pair<std::thread, int>>();
    };

    void runServer() {
        while (!ctrlCClicked.load()) {
            std::cout << "WE ARE IN RUNSERVER" << std::endl;

            int clientSocket = acceptConnection(serverSocket);

            std::clog << "Client connected." << std::endl;

            clients->push_back({ std::thread([this, &clientSocket]() { receiveAndCheckData(clientSocket); }), clientSocket });
        }
    }

    void receiveAndCheckData(int& clientSocket) {
        std::cout << "WE ARE RECEIVING" << std::endl;
        while (!ctrlCClicked.load())
        {
            char buffer[sizeof(DataPackage)];
            DataPackage receivedData;

            if (recv(clientSocket, buffer, sizeof(DataPackage), 0) == -1) {
                std::cerr << "Failed to receive data package." << std::endl;
                closeSocket(clientSocket);
                break;
            }

            memcpy(&receivedData, buffer, sizeof(DataPackage));

            std::cout << "Received buffer: " << receivedData.buffer << std::endl;
            std::cout << "Received number: " << receivedData.number << std::endl;
            std::cout << "Received checksum: " << receivedData.checksum << std::endl;

            char responceBuffer[sizeof(bool) + 1];

            if (calculateXORChecksum(receivedData.buffer, strlen(receivedData.buffer), receivedData.number) != receivedData.checksum) {
                // if failed then we are sending 0 - false
                strcpy(responceBuffer, "0");
                std::cout << "Checksum doesnt match" << std::endl;
            } else {
                // else 1 - true
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

    ~Server(){
        std::cout << "SERVER: SHUTTING DOWN NORMALLY" << std::endl;

        for (auto& pair: *clients) {
            closeSocket(pair.second);
            if (pair.first.joinable()) {
                pair.first.join();
            }
        }

        closeSocket(serverSocket);
        cleanupWinsock();
        delete clients;
    };

	Server(const Server &) = delete;

	Server &operator=(const Server &) = delete;

    inline int getSocket() const
    {
        return serverSocket;
    }
private:
    int serverSocket;
    std::vector<std::pair<std::thread, int>> *clients;
};

int main()
{
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler::ctrlHandler, TRUE);
#else
    signal(SIGINT, CtrlHandler::ctrlHandler);
#endif

    Server* server;
    try {
        server = new Server();
        supportThread = new std::thread(CtrlHandler::closeSocketThread, server->getSocket());
        SleepS(150);
        server->runServer();
    } catch(SocketUtil::SOCKET_ERRORS err) {
        std::cout << "An error occurred: " << SocketUtil::SOCKET_ERRORS_TEXT[err] << std::endl;
    }

    if (server) {
        delete server;
    }

    shouldSupportThreadStop.store(true);
    supportThread->join();
    delete supportThread;
    return 0;
}
#include "headers/socket_utils.h"
#include "headers/ctrl_handler.h"

class Proxy : private SocketUtil
{
public:
    Proxy()
    {
        WSAStartUp();
        proxySocket = initsSocket();
        bindSocket(proxySocket, PROXY_PORT);
        workers = new std::vector<ThreadStructure>();
    }

    void startUp()
    {
        std::clog << "PROXY: STARTING" << std::endl;

        while (!ctrlCClicked.load()) {
            listenToSocket(proxySocket);

            std::clog << "Proxy: listening on port " << PROXY_PORT << "..." << std::endl;

            int clientSocket = acceptConnection(proxySocket);

            if (clientSocket == INVALID_SOCKET)
            {
                std::cerr << "Proxy: Error accepting connection." << std::endl;
                return;
            }

            std::clog << "Proxy: client connected." << std::endl;

            int serverSocket = connectToServer();

            workers->push_back({ std::thread([this, &clientSocket, &serverSocket]() { passingData(clientSocket, serverSocket); }), clientSocket, serverSocket });
        }
    }

    int connectToServer()
    {
        std::cout << "STARTING CLIENT SIDE" << std::endl;

        int serverSocket = initsSocket();
        connectToSocket(serverSocket, SERVER_PORT);

        std::cout << "CLIENT-SIDE: Connected to the server." << std::endl;

        return serverSocket;
    }

    void passingData(int& clientSocket, int& serverSocket){
        char buffer[sizeof(DataPackage)];
        char responceBuffer[sizeof(bool) + 1];
        int bytesRead;
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        {
            if (ctrlCClicked.load()) {
                break;
            }
            std::cout << "Proxy: has received data from client" << std::endl;

            if (shouldIChangeVariable()) {
                makeRandomChanges(buffer, sizeof(DataPackage));
            }

            if (send(serverSocket, buffer, bytesRead, 0) > 0)
            {
                std::cout << "Proxy: Success sent package to server" << std::endl;
            } else {
                std::cout << "Proxy: Server has no responce" << std::endl;
                break;
            };

            // start receiving from server
            if (recv(serverSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "Proxy: Has received responce from server" << std::endl;
            } else {
                std::cout << "Proxy: Server has no responce" << std::endl;
            }

            // start sending to client
            if (send(clientSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "Proxy: Successfully pass server responce to client" << std::endl;
            } else {
                std::cout << "Proxy: Client has no responce" << std::endl;
            }

            std::cout << "------------ Package life ending ------------" << std::endl;
        }
    }

    ~Proxy()
    {
        std::cout << "PROXY: SHUTTING DOWN NORMALLY" << std::endl;

        for (auto& threadStruct: *workers) {
            closeSocket(threadStruct.clientSocket);
            closeSocket(threadStruct.serverSocket); // place outside
            if (threadStruct.thread.joinable()) {
                threadStruct.thread.join();
            }
        }

        delete workers;
        closeSocket(proxySocket);
        cleanupWinsock();
    }

    inline int getSocket() const
    {
        return proxySocket;
    }
private:
    int proxySocket;

    struct ThreadStructure {
        std::thread thread;
        int clientSocket;
        int serverSocket;
    };

    std::vector<ThreadStructure>* workers;

    void makeRandomChanges(char* buffer, int length) {
        srand(time(0));
        int index = rand() % length;
        buffer[index] = static_cast<char>((buffer[index] + rand() % 26) % 127);
        std::cout << "PROXY: Making changes -> Changed buffer at index " << index << std::endl;
    }

    bool shouldIChangeVariable(){
        srand(time(0));
        return (rand() % 5 + 1) == 3; // probability 1/5
    }
};

int main()
{
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler::ctrlHandler, TRUE);
#else
    signal(SIGINT, CtrlHandler::signalHandler);
#endif
    Proxy* proxy;
    try {
        proxy = new Proxy();
        supportThread = new std::thread(CtrlHandler::closeSocketThread, proxy->getSocket());
        proxy->startUp();
    } catch(SocketUtil::SOCKET_ERRORS err) {
        std::cout << "An error occurred: " << SocketUtil::SOCKET_ERRORS_TEXT[err] << std::endl;
    }

    if (proxy) {
        delete proxy;
    }

    shouldSupportThreadStop.store(true);
    supportThread->join();
    delete supportThread;
    return 0;
}

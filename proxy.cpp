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

        while (!ctrlCClicked.load())
        {
            if (ctrlCClicked.load()) {
                break;
            }

            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                std::cout << "Proxy: has received data from client" << std::endl;
            } else {
                std::cout << "Proxy: Client has no responce" << std::endl;
                break;
            }

            if (shouldIChangeVariable()) {
                DataPackage* dataPackageInterface = reinterpret_cast<DataPackage*>(buffer);
                makeRandomChanges(dataPackageInterface);
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
            shutdown(threadStruct.clientSocket, DISALLOW_BOTH);
            shutdown(threadStruct.serverSocket, DISALLOW_BOTH);
            closeSocket(threadStruct.clientSocket);
            closeSocket(threadStruct.serverSocket);

            if (threadStruct.thread.joinable()) {
                threadStruct.thread.join();
            }
        }
        if (!proxySocket) // if not 0(not initialized)
        {
            closeSocket(proxySocket);
        }
        cleanupWinsock();
        delete workers;
    }

    inline int getSocket() const
    {
        return proxySocket;
    }
private:
    int proxySocket = 0;

    struct ThreadStructure {
        std::thread thread;
        int clientSocket;
        int serverSocket;
    };

    std::vector<ThreadStructure>* workers;

    void makeRandomChanges(DataPackage* dataPackage) {
        srand(time(0)); // setting seed to be current time in seconds

        dataPackage->checksum += rand() % 10;
        dataPackage->number += rand() % 10;
        int index = rand() % strlen(dataPackage->buffer); // selecting random index in buffer
        dataPackage->buffer[index] = rand() % 128; // changing char at this index
        std::cout << "PROXY: Making changes" << std::endl;
    }

    bool shouldIChangeVariable(){
        srand(time(0));
        return (rand() % 5 + 1) == 3; // probability 1/5 -> selecting one number of 5(in our case its 3)
    }
};

int main()
{
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler::ctrlHandler, TRUE);
#else
    signal(SIGINT, CtrlHandler::ctrlHandler);
#endif
    
    std::unique_ptr<Proxy> proxy;
    try {
        proxy = std::make_unique<Proxy>();
        supportThread = new std::thread(CtrlHandler::closeSocketThread, proxy->getSocket());
        SleepS(150);
        proxy->startUp();
    } catch(const SocketUtil::SOCKET_ERRORS& err) {
        std::cout << "Socket error occurred: " << SocketUtil::SOCKET_ERRORS_TEXT[err] << std::endl;
    } catch(const std::bad_alloc& e) {
        std::cerr << "Failed to create object: " << e.what() << std::endl;
    }  catch(const std::exception& e) {
        std::cerr << "Undefined error occurred: " << e.what() << std::endl;
    }

    if (supportThread) {
        shouldSupportThreadStop.store(true);
        supportThread->join();
        delete supportThread;
    }
    return 0;
}

#include "headers/socket_utils.h"
#include "headers/ctrl_handler.h"
#include "headers/other.h"

class Client : private SocketUtil
{
public:
    Client(){
        std::cout << "CLIENT: STARTING" << std::endl;

        WSAStartUp();

        clientSocket = initsSocket();

        connectToSocket(clientSocket, PROXY_PORT);

        std::cout << "CLIENT: Connected to the server." << std::endl;
    };

    void sendDataWithCheckSum(){
        char responceBuffer[sizeof(bool) + 1];

        while (!ctrlCClicked.load()) {
            DataPackage dataToSend;

            std::cout << "CLIENT: Please enter string to pass: " << std::endl;
            // cannot terminate on linux without entering some text
            std::cin.getline(dataToSend.buffer, MAX_BUFFER_SIZE);

            if (std::cin.fail()) // forgot to check overflow 
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "CLIENT: Input overflow -> please enter lesser number of symbols" << std::endl;
                continue;
            }

            dataToSend.number = 1234567890;
            dataToSend.checksum = calculateXORChecksum(dataToSend.buffer, strlen(dataToSend.buffer), dataToSend.number);

            char buffer[sizeof(dataToSend)];

            memcpy(buffer, &dataToSend, sizeof(dataToSend));

            if (send(clientSocket, buffer, sizeof(buffer), 0) > 0)
            {
                std::cout << "CLIENT: SUCCESSfULLY SENT" << std::endl;
            } else {
                std::cerr << "CLIENT: SERVER: dead" << std::endl;
                break;
            }

            if (recv(clientSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "CLIENT: Has received responce from server" << std::endl;
            } else {
                std::cout << "CLIENT: Server has no responce" << std::endl;
                break;
            }

            if (strcmp(responceBuffer, "1") == 0)
            {
                std::cout << "CLIENT: Message wasnt changed on his way" << std::endl;
            } else if (strcmp(responceBuffer, "0") == 0) {
                std::cout << "CLIENT: Message was changed -> Checksum hadned matched" << std::endl;
            } else {
                std::cout << "CLIENT: Undefined message" << std::endl;
                break;
            }
        }
    };

    ~Client(){
        std::cout << "CLIENT: SHUTTING DOWN" << std::endl;
        if (clientSocket > 0)
        {
            closeSocket(clientSocket);
        }
        cleanupWinsock();
    };

    Client(const Client &) = delete;

	Client &operator=(const Client &) = delete;

    inline const int getSocket(){
        return clientSocket;
    }
private:
    int clientSocket;
};

int main()
{
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler::ctrlHandler, TRUE);
#else
    signal(SIGINT, CtrlHandler::ctrlHandler);
#endif

    Client* client;
    try {
        client = new Client();
        supportThread = new std::thread(CtrlHandler::closeSocketThread, client->getSocket());
        SleepS(150);
        client->sendDataWithCheckSum();
    } catch(SocketUtil::SOCKET_ERRORS err) {
        std::cout << "An error occurred: " << SocketUtil::SOCKET_ERRORS_TEXT[err] << std::endl;
    }

    if (client) {
        delete client;
    }

    shouldSupportThreadStop.store(true);
    supportThread->join();
    delete supportThread;
    
    return 0;
}
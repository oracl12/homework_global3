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
            // cannot terminate without entering some text
            std::cin.getline(dataToSend.buffer, MAX_BUFFER_SIZE);

            if (std::cin.fail())
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
        if (!clientSocket) // if not 0(not initialized)
        {
            shutdown(clientSocket, DISALLOW_BOTH);
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
    int clientSocket = 0;
};

int main()
{
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler::ctrlHandler, TRUE);
#else
    signal(SIGINT, CtrlHandler::ctrlHandler);
#endif

    std::unique_ptr<Client> client;
    try {
        client = std::make_unique<Client>();
        supportThread = new std::thread(CtrlHandler::closeSocketThread, client->getSocket());
        SleepS(150);
        client->sendDataWithCheckSum();
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
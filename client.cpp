#include "headers/socket_utils.h"
#include "headers/other.h"

class Client : private SocketUtil
{
public:
    Client(){
        std::cout << "STARTING CLIENT" << std::endl;

        WSAStartUp();

        clientSocket = initsSocket();

        connectToSocket(clientSocket, PROXY_PORT);

        std::cout << "CLIENT: Connected to the server." << std::endl;
    };

    void sendDataWithCheckSum(){
        char responceBuffer[sizeof(bool) + 1];

        while (true) {
            DataPackage dataToSend;

            std::cout << "Please enter string to pass: " << std::endl;
            std::cin.getline(dataToSend.buffer, MAX_BUFFER_SIZE);

            dataToSend.number = 1234567890;
            dataToSend.checksum = calculateXORChecksum(dataToSend.buffer, strlen(dataToSend.buffer), dataToSend.number);

            char buffer[sizeof(dataToSend)];
            memcpy(buffer, &dataToSend, sizeof(dataToSend));

            if (send(clientSocket, buffer, sizeof(buffer), 0) > 0)
            {
                std::cout << "SUCCESSfULLY SENT" << std::endl;
            } else {
                std::cerr << "SERVER: dead" << std::endl;
                break;
            }

            // start receiving from server
            if (recv(clientSocket, responceBuffer, sizeof(responceBuffer), 0) > 0)
            {
                std::cout << "Has received responce from server" << std::endl;
            } else {
                std::cout << "Server has no responce" << std::endl;
                break;
            }

            std::cout << responceBuffer<< std::endl;
            if (strcmp(responceBuffer, "1") == 0)
            {
                std::cout << "Message wasnt changed on his way" << std::endl;
            } else if (strcmp(responceBuffer, "0") == 0) {
                std::cout << "Message was changed; Checksum hadned matched" << std::endl;
            } else {
                std::cout << "Undefined message" << std::endl;
                break;
            }
        }
    };

    void forceCleanUpProgram() override
    {
        std::cout << "CLIENT: SHUTTING DOWN FORCEFULLY" << std::endl;
        closeSocket(clientSocket);
        cleanupWinsock();
        exit(1);
    }

    ~Client(){
        std::cout << "CLIENT: SHUTTING DOWN NORMALLY" << std::endl;
        closeSocket(clientSocket);
        cleanupWinsock();
    };

    Client(const Client &) = delete;

	Client &operator=(const Client &) = delete;
private:
    int clientSocket;
    bool shouldStop;
};

int main(){
    Client client;
    client.sendDataWithCheckSum();
    return 0;
}
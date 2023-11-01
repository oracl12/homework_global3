// should sent data, lets say buffer on 32 elements, + unsindned long int - checksum in bytes??

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
        while (true) {
            DataPackage dataToSend;
            strcpy(dataToSend.buffer, "This is a 32-byte buffer!");
            dataToSend.number = 1234567890;
            dataToSend.checksum = calculateXORChecksum(dataToSend.buffer, strlen(dataToSend.buffer), dataToSend.number);

            char buffer[sizeof(dataToSend)];
            memcpy(buffer, &dataToSend, sizeof(dataToSend));

            int bytesSent = send(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesSent <= 0)
            {
                SleepS(3000);
                std::cerr << "SERVER: dead" << std::endl;
                break;
            }

            SleepS(200);
            std::cout << "SUCCESSfULLY SENT" << std::endl;
        }
    };

    ~Client(){
        std::cout << "CLIENT: SHUTTING DOWN" << std::endl;
        closeSocket(clientSocket);
        cleanupWinsock();
    };

    Client(const Client &) = delete;

	Client &operator=(const Client &) = delete;
private:
    int clientSocket;
};

int main(){
    Client client;
    client.sendDataWithCheckSum();
    return 0;
}
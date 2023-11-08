#pragma once

#include <atomic>
#include "socket_utils.h"

std::atomic<bool> ctrlCClicked(false);
std::atomic<bool> shouldSupportThreadStop(false);
std::thread* supportThread;

class CtrlHandler
{
public:
    static BOOL ctrlHandler(DWORD fdwCtrlType) {
        switch (fdwCtrlType) {
            case CTRL_C_EVENT:
                std::cout << "CtrlHandler: Ctrl+C received!" << std::endl;
                ctrlCClicked.store(true);
                return TRUE;
            default:
                return FALSE;
        }
    }

    static void closeSocketThread(int socket) {
        SleepS(150);
        std::cout << "SUPPORT THREAD: thread start" << std::endl;
        while (!shouldSupportThreadStop.load()) {
            if (ctrlCClicked.load()) {
                std::cout << "SUPPORT THREAD: Closing socket..." << std::endl;
                SocketUtil::closeSocket(socket);
                break;
            }
        
            SleepS(100);
        }
        std::cout << "SUPPORT THREAD: end" << std::endl;
        return;
    }
};
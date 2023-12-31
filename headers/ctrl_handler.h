#pragma once

#include <atomic>
#include "socket_utils.h"

std::atomic<bool> ctrlCClicked(false);
std::atomic<bool> shouldSupportThreadStop(false);
std::thread* supportThread;

class CtrlHandler
{
public:
#ifdef __WIN32
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
#else
    static void ctrlHandler(int signum) {
        if (signum == SIGINT) {
            std::cout << "CtrlHandler: Ctrl+C received!" << std::endl;
            ctrlCClicked.store(true);
        }
    }
#endif

    static void closeSocketThread(int socket) {
        std::cout << "SUPPORT THREAD: thread start" << std::endl;
        while (!shouldSupportThreadStop.load()) {
            if (ctrlCClicked.load()) {
                // closing socket here terminates threads that are on wait cause of send/recv/accept...etc
                std::cout << "SUPPORT THREAD: Closing socket..." << std::endl;
                shutdown(socket, DISALLOW_BOTH);
                SocketUtil::closeSocket(socket);
                break;
            }
        
            SleepS(100);
        }
        std::cout << "SUPPORT THREAD: end" << std::endl;
        return;
    }
};
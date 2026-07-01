#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_MESSAGE_LEN 1024

class RedisServer {
    public:
        RedisServer(int port);
        ~RedisServer();
        void start();
        void stop();

    private:
        int server_fd;
        struct sockaddr_in address;
        int addrlen;
        bool running;

        void handleClient(int client_socket);
};

#endif

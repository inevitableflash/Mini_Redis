#include "server.h"
#include "resp.h"
#include <algorithm>
#include <sstream>

RedisServer::RedisServer(int port) {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    std::cerr << "Socket could not be created\n";
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  addrlen = sizeof(address);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Unable to bind socket to given port\n";
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    std::cerr << "Listen failed\n";
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  running = false;
}

RedisServer::~RedisServer() { stop(); }

void RedisServer::start() {
  running = true;
  std::cout << "Server started, waiting for connections..." << std::endl;

  while (running) {
    int client_socket =
        accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_socket < 0) {
      perror("accept failed");
      continue;
    }
    std::cout << "Connection accepted" << std::endl;

    bool clientConnected = true;
    while (running && clientConnected) {
      clientConnected = false;
      handleClient(client_socket, clientConnected);
    }
    close(client_socket);
    std::cout << "Connection closed, waiting for next client..." << std::endl;
  }
  stop();
}

void RedisServer::stop() {
  if (running) {
    running = false;
    close(server_fd);
    std::cout << "Server stopped" << std::endl;
  }
}

void RedisServer::handleClient(int client_socket, bool &keepGoing) {
  char buffer[MAX_MESSAGE_LEN] = {0};
  int bytesRead = read(client_socket, buffer, MAX_MESSAGE_LEN);

  if (bytesRead <= 0) {
    std::cout << "Client disconnected" << std::endl;
    return;
  }

  std::string response;
  try {
    std::stringstream inputStream(std::string(buffer, bytesRead));
    Resp resp(inputStream);
    Value command = resp.Read();

    if (command.array.empty()) {
      throw std::runtime_error("Empty command");
    }

    std::string commandName = command.array[0].bulk;
    std::transform(commandName.begin(), commandName.end(),
                    commandName.begin(), ::toupper);

    std::vector<Value> args(command.array.begin() + 1, command.array.end());

    Value result = store.executeCommand(commandName, args);
    response = result.translateToResp();
  } catch (const std::exception &e) {
    response = std::string("-ERR ") + e.what() + "\r\n";
  }

  send(client_socket, response.c_str(), response.length(), 0);
  std::cout << "Response sent " << response << std::endl;
  keepGoing = true;
}
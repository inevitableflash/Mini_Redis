#include "server.h"
#include "resp.h"

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

  int client_socket =
      accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
  if (client_socket < 0) {
    perror("accept failed");
    stop();
    exit(EXIT_FAILURE);
  }
  std::cout << "Connection accepted" << std::endl;
  // TODO: Keep server running
  // handleClient(client_socket);
  while (running) {
    handleClient(client_socket);
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

void RedisServer::handleClient(int client_socket) {
  char buffer[MAX_MESSAGE_LEN] = {0};
  read(client_socket, buffer, MAX_MESSAGE_LEN);
  std::cout << "Received: " << buffer << std::endl;

  std::stringstream inputStream(buffer);
  Resp resp(inputStream);
  Value command = resp.Read();

  // TODO: Handle command in Redis simulated database:::::::
  // Need command sanitizer/executor for this as well as an in-memory database
  // simulation
  std::string response = "+OK\r\n";
  send(client_socket, response.c_str(), response.length(), 0);
  std::cout << "Response sent " << response << std::endl;
}

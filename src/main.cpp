#include "server.h"

int main() {
  std::cout << "Hello World\n";
  RedisServer *redis = new RedisServer(6379);
  redis->start();
  return 0;
}

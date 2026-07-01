#ifndef KVSTORE_H
#define KVSTORE_H

#include <mutex>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include "resp.h"

const std::string PING = "PING";
const std::string GET = "GET";
const std::string SET = "SET";
const std::string HGET = "HGET";
const std::string HSET = "HSET";
const std::string DEL = "DEL";
const std::string HDEL = "HDEL";

class KVStore{
public:
    KVStore();
    Value executeCommand(std::string command, std::vector<Value> arguments);

private:
    // Map and mutex for redis store serving key>value pairs
    std::unordered_map<std::string, std::string> redisStore;
    std::mutex storeMutex;

    // Map and mutex for redis store serving hash->map pairs
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hashRedisStore;
    std::mutex hashStoreMutex;

    // Map to register "callback" to invoke based on the command name
    std::unordered_map<std::string, std::function<Value(KVStore&, std::vector<Value>)>> handlers;

    // Private method handlers hidden based on principle of encapsulation
    Value ping(std::vector<Value> arguments);
    Value set(std::vector<Value> arguments);
    Value get(std::vector<Value> arguments);
    Value hset(std::vector<Value> arguments);
    Value hget(std::vector<Value> arguments);
    Value del(std::vector<Value> arguments);
    Value hdel(std::vector<Value> arguments);
};

#endif // KVSTORE_H

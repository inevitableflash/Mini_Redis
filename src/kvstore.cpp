#include "kvstore.h"

KVStore::KVStore() {
  handlers[PING] = [](KVStore &store, std::vector<Value> args) {
    return store.ping(args);
  };
  handlers[GET] = [](KVStore &store, std::vector<Value> args) {
    return store.get(args);
  };
  handlers[SET] = [](KVStore &store, std::vector<Value> args) {
    return store.set(args);
  };
  handlers[HGET] = [](KVStore &store, std::vector<Value> args) {
    return store.hget(args);
  };
  handlers[HSET] = [](KVStore &store, std::vector<Value> args) {
    return store.hset(args);
  };
  handlers[DEL] = [](KVStore &store, std::vector<Value> args) {
    return store.del(args);
  };
  handlers[HDEL] = [](KVStore &store, std::vector<Value> args) {
    return store.hdel(args);
  };
}

Value KVStore::executeCommand(std::string command,
                              std::vector<Value> arguments) {
  if (handlers.find(command) == handlers.end()) {
    throw std::runtime_error("Invalid command provided");
  }
  // Invoke command from handler mapping
  return handlers[command](*this, arguments);
}

Value KVStore::ping(std::vector<Value> arguments) {
  // Need unused arg to allow map handler to register PING response
  (void)arguments;

  return Value("string", "PONG", 0, "");
}

Value KVStore::set(std::vector<Value> arguments) {
  if (arguments.size() != 2) {
    throw std::runtime_error("ERR wrong number of args for 'SET' command");
  }

  std::string key = arguments[0].bulk;
  std::string val = arguments[1].bulk;

  std::scoped_lock lock(this->storeMutex);

  redisStore[key] = val;

  return Value("string", "OK", 0, "");
}

Value KVStore::get(std::vector<Value> arguments) {
  if (arguments.size() != 1) {
    throw std::runtime_error("ERR wrong number of args for 'GET' command");
  }

  std::string key = arguments[0].bulk;

  std::scoped_lock lock(this->storeMutex);

  if (redisStore.find(key) == redisStore.end()) {
    return Value("null", "", 0, "");
  } else {
    return Value("bulk", "", 0, redisStore[key]);
  }
}

Value KVStore::hset(std::vector<Value> arguments) {
  if (arguments.size() != 3) {
    throw std::runtime_error("ERR wrong number of args for 'HSET' command");
  }

  std::string hashKey = arguments[0].bulk;
  std::string field = arguments[1].bulk;
  std::string value = arguments[2].bulk;

  std::scoped_lock lock(this->hashStoreMutex);

  hashRedisStore[hashKey][field] = value;

  return Value("string", "OK", 0, "");
}

Value KVStore::hget(std::vector<Value> arguments) {
  if (arguments.size() != 2) {
    throw std::runtime_error("ERR wrong number of args for 'HGET' command");
  }

  std::string hashKey = arguments[0].bulk;
  std::string field = arguments[1].bulk;

  std::scoped_lock lock(this->hashStoreMutex);

  if (hashRedisStore.find(hashKey) != hashRedisStore.end() &&
      hashRedisStore[hashKey].find(field) != hashRedisStore[hashKey].end()) {
    return Value("bulk", "", 0, hashRedisStore[hashKey][field]);
  } else {
    return Value("null", "", 0, "");
  }
}

Value KVStore::del(std::vector<Value> arguments) {
  if (arguments.size() < 1) {
    throw std::runtime_error("ERR wrong number of args for 'DEL' command");
  }

  std::scoped_lock lock(this->storeMutex);

  size_t count = 0;
  for (Value &arg : arguments) {
    std::string key = arg.bulk;
    if (redisStore.find(key) != redisStore.end()) {
      count += redisStore.erase(key);
    }
  }

  return Value("integer", "", count, "");
}

Value KVStore::hdel(std::vector<Value> arguments) {
  if (arguments.size() < 2) {
    throw std::runtime_error("ERR wrong number of args for 'HDEL' command");
  }

  std::scoped_lock lock(this->hashStoreMutex);

  size_t count = 0;
  std::string hashKey = arguments[0].bulk;
  for (size_t i = 1; i < arguments.size(); i++) {
    std::string field = arguments[i].bulk;
    if (hashRedisStore.find(hashKey) != hashRedisStore.end() &&
        hashRedisStore[hashKey].find(field) != hashRedisStore[hashKey].end()) {
      count += hashRedisStore[hashKey].erase(field);
    }
  }
  // Remove hashkey mapping if all fields were emptied
  if (hashRedisStore[hashKey].empty()) {
    hashRedisStore.erase(hashKey);
  }
  
  return Value("integer", "", count, "");
}

#include "kvstore.h"
using namespace std;


/*
std::unordered_map<std::string, std::string> redisStore;                                     // for SET/GET
std::unordered_map<std::string, std::unordered_map<std::string,std::string>> hashRedisStore; // for HSET/HGET

HSET user:1 name Divy stores {"user:1": {"name": "Divy"}}

*/
KVStore::KVStore() {
  handlers[PING] = [](KVStore &store, vector<Value> args) {
    return store.ping(args);
  };
  handlers[GET] = [](KVStore &store, vector<Value> args) {
    return store.get(args);
  };
  handlers[SET] = [](KVStore &store, vector<Value> args) {
    return store.set(args);
  };
  handlers[HGET] = [](KVStore &store, vector<Value> args) {
    return store.hget(args);
  };
  handlers[HSET] = [](KVStore &store, vector<Value> args) {
    return store.hset(args);
  };
  handlers[DEL] = [](KVStore &store, vector<Value> args) {
    return store.del(args);
  };
  handlers[HDEL] = [](KVStore &store, vector<Value> args) {
    return store.hdel(args);
  };
}

Value KVStore::executeCommand(string command,
                              vector<Value> arguments) {
  if (handlers.find(command) == handlers.end()) {
    throw runtime_error("Invalid command provided");
  }
  // Invoke command from handler mapping
  return handlers[command](*this, arguments);
}

Value KVStore::ping(vector<Value> arguments) {
  // Need unused arg to allow map handler to register PING response
  (void)arguments;

  return Value("string", "PONG", 0, "");
}

Value KVStore::set(vector<Value> arguments) {
  if (arguments.size() != 2) {
    throw runtime_error("ERR wrong number of args for 'SET' command");
  }

  string key = arguments[0].bulk;
  string val = arguments[1].bulk;

  scoped_lock lock(this->storeMutex);

  redisStore[key] = val;

  return Value("string", "OK", 0, "");
}

Value KVStore::get(vector<Value> arguments) {
  if (arguments.size() != 1) {
    throw runtime_error("ERR wrong number of args for 'GET' command");
  }

  string key = arguments[0].bulk;

  scoped_lock lock(this->storeMutex);

  if (redisStore.find(key) == redisStore.end()) {
    return Value("null", "", 0, "");
  } else {
    return Value("bulk", "", 0, redisStore[key]);
  }
}

Value KVStore::hset(vector<Value> arguments) {
  if (arguments.size() != 3) {
    throw runtime_error("ERR wrong number of args for 'HSET' command");
  }

  string hashKey = arguments[0].bulk;
  string field = arguments[1].bulk;
  string value = arguments[2].bulk;

  scoped_lock lock(this->hashStoreMutex);

  hashRedisStore[hashKey][field] = value;

  return Value("string", "OK", 0, "");
}

Value KVStore::hget(vector<Value> arguments) {
  if (arguments.size() != 2) {
    throw runtime_error("ERR wrong number of args for 'HGET' command");
  }

  string hashKey = arguments[0].bulk;
  string field = arguments[1].bulk;

  scoped_lock lock(this->hashStoreMutex);

  if (hashRedisStore.find(hashKey) != hashRedisStore.end() &&
      hashRedisStore[hashKey].find(field) != hashRedisStore[hashKey].end()) {
    return Value("bulk", "", 0, hashRedisStore[hashKey][field]);
  } else {
    return Value("null", "", 0, "");
  }
}

Value KVStore::del(vector<Value> arguments) {
  if (arguments.size() < 1) {
    throw runtime_error("ERR wrong number of args for 'DEL' command");
  }

  scoped_lock lock(this->storeMutex);

  size_t count = 0;
  for (Value &arg : arguments) {
    string key = arg.bulk;
    if (redisStore.find(key) != redisStore.end()) {
      count += redisStore.erase(key);
    }
  }

  return Value("integer", "", count, "");
}

Value KVStore::hdel(vector<Value> arguments) {
  if (arguments.size() < 2) {
    throw runtime_error("ERR wrong number of args for 'HDEL' command");
  }

  scoped_lock lock(this->hashStoreMutex);

  size_t count = 0;
  string hashKey = arguments[0].bulk;
  for (size_t i = 1; i < arguments.size(); i++) {
    string field = arguments[i].bulk;
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

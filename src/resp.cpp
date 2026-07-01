#include "resp.h"

std::string Value::translateToResp() {
  if (this->typ == "array") {
    return this->valueArray();
  } else if (this->typ == "bulk") {
    return this->valueBulk();
  } else if (this->typ == "string") {
    return this->valueString();
  } else if (this->typ == "null") {
    return this->valueNull();
  } else if (this->typ == "error") {
    return this->valuError();
  } else {
    return std::string();
  }
}

std::string Value::valueString() {
  std::string result = std::string(1, STRING) + this->str + '\r' + '\n';
  return result;
}

std::string Value::valueBulk() {
  int bulkLen = this->bulk.length();
  std::string result = std::string(1, BULK) + std::to_string(bulkLen) + '\r' +
                       '\n' + this->bulk + '\r' + '\n';
  return result;
}

std::string Value::valueArray() {
  int arrLen = this->array.size();
  std::string result =
      std::string(1, ARRAY) + std::to_string(arrLen) + '\r' + '\n';
  // Recursively translate all sub values inside the array
  for (int i = 0; i < arrLen; i++) {
    result += this->array[i].translateToResp();
  }
}

std::string Value::valueInteger()
{
  std::string result = std::string(1, INTEGER) + std::to_string(this->num) + '\r' + '\n';
  return result;
}

std::string Value::valuError() {
  std::string result = std::string(1, ERROR) + this->str + '\r' + '\n';
  return result;
}

std::string Value::valueNull() {
  std::string result = "$-1\r\n";
  return result;
}

Resp::Resp(std::istream &input) : reader(input) {}

std::string Resp::readLine() {
  std::string line;
  char c;

  while (reader.get(c)) {
    line += c;
    if (line.length() >= 2 && line[line.length() - 2] == '\r') {
      break;
    }
  }
  return line.substr(0, line.length() - 2); // Remove the trailing CRLF
}

int Resp::readInteger() {
  std::string line = readLine();
  try {
    return std::stoi(line);
  } catch (const std::invalid_argument &) {
    throw std::runtime_error("Invalid integer format");
  }
}

Value Resp::Read() {
  // b"*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n"
  char type;
  if (!reader.get(type)) {
    throw std::runtime_error("Failed to read type");
  }
  switch (type) {
  case ARRAY:
    return readArray();
  case BULK:
    return readBulk();
  default:
    throw std::runtime_error("Unknown type");
  }
}

Value Resp::readArray() {
  Value v;
  v.typ = "array";

  int len = readInteger();
  v.array.resize(len);

  for (int i = 0; i < len; i++) {
    v.array[i] = Read();
  }
  return v;
}

Value Resp::readBulk() {
  Value v;
  v.typ = "bulk";
  std::string bulk = "";
  char c;

  int len = readInteger();

  for (int i = 0; i < len; i++) {
    reader.get(c);
    bulk += c;
  }
  std::cout << "Bulk read: " << bulk << std::endl;
  v.bulk = bulk;
  readLine(); // Read the trailing CRLF

  return v;
}

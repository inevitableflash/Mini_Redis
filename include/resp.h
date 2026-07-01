#ifndef RESP_H
#define RESP_H

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

const char STRING  = '+';
const char ERROR   = '-';
const char INTEGER = ':';
const char BULK    = '$';
const char ARRAY   = '*';

class Value {
public:
    std::string typ;
    std::string str;
    int num;
    std::string bulk;
    std::vector<Value> array;

    Value() : num(0) {}
    Value(const std::string& typ_, const std::string& str_, int num_, 
        const std::string& bulk_)
        : typ(typ_), str(str_), num(num_), bulk(bulk_) {}
    std::string translateToResp();

private:
    std::string valueString();
    std::string valueBulk();
    std::string valueArray();
    std::string valueInteger();
    std::string valuError();
    std::string valueNull();
};

class Resp {
public:
    Resp(std::istream& input);
    Value Read();

private:
    std::istream& reader;

    std::string readLine();
    int readInteger();
    Value readArray();
    Value readBulk();
};

#endif // RESP_H

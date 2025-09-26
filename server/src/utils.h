#ifndef UTILS_H
#define UTILS_H

#include "noncopyable.h"
#include "client/RedisClient.h"
#include <string>

using std::string;

string Cookie2User_id(RedisClient &redis, const string &cookie);
std::string generate_uuid();

template <class T>
struct deleteOfstream {  // ofstream的智能指针删除器
    void operator()(T* p) { p->close(); }
};

#endif
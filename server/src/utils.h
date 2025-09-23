#ifndef UTILS_H
#define UTILS_H

#include "noncopyable.h"


template <class T>
struct deleteOfstream {  // ofstream的智能指针删除器
    void operator()(T* p) { p->close(); }
};

#endif
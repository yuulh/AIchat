/* 
* 继承以实现对象语义
*/

#ifndef _NONCOPYABLE_H
#define _NONCOPYABLE_H

class Noncopyable{
protected:
    Noncopyable() {}

    ~Noncopyable() {}

    Noncopyable(const Noncopyable &rhs) = delete;
    Noncopyable(const Noncopyable &&rhs) = delete;

    Noncopyable &operator=(const Noncopyable &rhs) = delete;
    Noncopyable &operator=(const Noncopyable &&rhs) = delete;

};

#endif
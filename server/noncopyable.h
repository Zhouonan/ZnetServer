#pragma once 

namespace ZnetServer
{
class Noncopyable {
public: 

    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator= (const Noncopyable&) = delete;

};
} // namespace ZnetServer

#pragma once

template <class T>
class LockGuard
{
public:
    LockGuard(T &v) : m_v(v) { v.lock(); }
    ~LockGuard() { m_v.unlock(); }

private:
    T &m_v;
};
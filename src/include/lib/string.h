#pragma once
#include <types.h>
#include <lib/vector.h>

class String : public Vector<char>
{
private:
public:
    String() {}
    String(const char *str);
    String(const char *str, uint len);
    String(const String &str);

    const char *c_str() const;

    bool operator==(const String &other) const;
    bool operator==(const char *other) const;
    bool operator!=(const String &other) const;
    bool operator!=(const char *other) const;
    void operator+=(const String &other);
    void operator+=(const char c);
    String operator+(const char c) const;
    String operator+(const String& str) const;
    String operator+(const char* str) const;

    Vector<String> split(const char delim) const;
    String slice(uint start, uint end) const;
    int64 findLastIndex(const char c) const;
};
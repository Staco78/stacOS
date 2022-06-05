#include <lib/string.h>
#include <debug.h>
#include <lib/mem.h>
#include <memory.h>

String::String(const char *str)
{
    uint len = strlen(str);
    realloc(len);
    memcpy(begin(), str, len);
    _size = len;
}

String::String(const char *str, uint len)
{
    realloc(len);
    memcpy(begin(), str, len);
    _size = len;
}

String::String(const String &str)
{
    realloc(str.size());
    memcpy(begin(), str.begin(), str.size());
    _size = str.size();
}

const char *String::c_str() const
{
    if (dataSize <= _size)
        realloc(_size + 1);
    _data[_size] = 0;

    return begin();
}

bool String::operator==(const String &other) const
{
    if (size() != other.size())
        return false;
    return memcmp(begin(), other.begin(), size());
}

bool String::operator==(const char *other) const
{
    if (strlen(other) != size())
        return false;

    return memcmp(begin(), other, size());
}

bool String::operator!=(const String &other) const
{
    return !operator==(other);
}

bool String::operator!=(const char *other) const
{
    return !operator==(other);
}

void String::operator+=(const String &other)
{
    realloc(size() + other.size());
    memcpy(end(), other.begin(), other.size());
    _size += other.size();
}

void String::operator+=(const char c)
{
    push(c);
}

String String::operator+(const char c) const
{
    String str(*this);
    str += c;
    return str;
}

String String::operator+(const String &str) const
{
    String s(*this);
    s.realloc(size() + str.size());
    memcpy(s.begin() + size(), str.begin(), str.size());
    return s;
}

String String::operator+(const char *str) const
{
    String s(*this);
    uint len = strlen(str);
    s.realloc(size() + len);
    memcpy(s.begin(), str, len);
    return s;
}

Vector<String> String::split(const char delim) const
{
    Vector<String> result;

    int start = 0;
    unsigned i = 0;
    for (; i < size(); i++)
    {
        if (operator[](i) == delim)
        {
            if ((i - start) > 0)
            { // Do not add empty strings
                String str((begin() + start), (i - start));
                result.push(str);
            }
            start = i + 1;
        }
    }

    if ((i - start) > 0)
    { // Do not add empty strings
        result.push(String((begin() + start), (i - start)));
    }
    return result;
}

String String::slice(uint start, uint end) const
{
    if (end - start == 0)
        return String("");
    assert(start < size());
    assert(end > start);
    return String(begin() + start, end - start);
}

int64 String::findLastIndex(const char c) const
{
    uint i = size() - 1;
    for (; i >= 0; i--)
    {
        if (_data[i] == c)
            return i;
    }

    return -1;
}
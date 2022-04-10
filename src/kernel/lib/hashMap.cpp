#include <lib/hashMap.h>

uint stringHash(const String &key)
{
    unsigned int hash = 0;
    for (uint i = 0; i < key.size(); i++)
    {
        hash = key[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
#include <types.h>
#include <memory.h>
#include <debug.h>
#include <log.h>
#include <lib/list.h>
#include <lib/string.h>
#include <terminal.h>

template <typename K, typename T>
class HashMap
{
protected:
    static constexpr uint defaultSize = 10;

private:
    struct Node
    {
        K key;
        T data;
    };

    uint m_size;
    List<Node> *entries = nullptr;

    void allocIfNecessary()
    {
        if (entries)
            return;
        entries = (List<Node> *)kmalloc(sizeof(List<Node>) * m_size);
        for (uint i = 0; i < m_size; i++)
        {
            new (&entries[i]) List<Node>();
        }
    }

public:
    HashMap()
    {
        m_size = defaultSize;
        allocIfNecessary();
    }

    HashMap(uint size)
    {
        m_size = size;
        allocIfNecessary();
    }

    ~HashMap()
    {
        if (entries)
            kfree(entries);
    }

    T *get(const K &key)
    {
        allocIfNecessary();
        uint i = hash(key) % m_size;
        if (entries[i].size() == 0)
            return nullptr;
        if (entries[i].size() == 1)
        {
            if (entries[i][0]->key == key)
                return &entries[i][0]->data;
            return nullptr;
        }
        for (auto it = entries[i].begin(); it != entries[i].end(); it++)
        {
            if (it->key == key)
                return &it->data;
        }
        return nullptr;
    }

    T *set(const K &key, const T &value)
    {
        allocIfNecessary();
        uint i = hash(key) % m_size;
        entries[i].push({.key = key, .data = value});
        return &entries[i][entries[i].size() - 1]->data;
    }

    void remove(const K &key)
    {
        allocIfNecessary();
        uint i = hash(key) % m_size;
        assert(entries[i].size() > 0);
        if (entries[i].size() == 1)
            entries[i].clear();
        else
        {
            uint y = 0;
            for (auto it = entries[i].begin(); it != entries[i].end(); it++)
            {
                if (it->key == key)
                {
                    entries[i].remove(y);
                    return;
                }
                y++;
            }
            assert(false);
        }
    }

    uint hash(const String &key)
    {
        unsigned int hash = 0;
        for (uint i = 0; i < key.size(); i++)
        {
            hash = key[i] + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    uint hash(const uint32 &key)
    {
        return key;
    }
};

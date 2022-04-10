#include <types.h>
#include <memory.h>
#include <debug.h>
#include <log.h>
#include <lib/list.h>
#include <lib/string.h>
#include <terminal.h>

template <typename T, typename K>
class HashMap
{
protected:
    static constexpr uint defaultSize = 10;

private:
    typedef uint (*Hash)(const K &);

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

    Hash hash = nullptr;

public:
    HashMap(Hash hashFunc)
    {
        m_size = defaultSize;
        hash = hashFunc;
        allocIfNecessary();
    }

    HashMap(uint size, Hash hashFunc)
    {
        m_size = size;
        hash = hashFunc;
        allocIfNecessary();
    }

    ~HashMap()
    {
        if (entries)
            kfree(entries);
    }

    T *get(const K &key)
    {
        assert(hash);
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

    void set(const K &key, const T &value)
    {
        assert(hash);
        allocIfNecessary();
        uint i = hash(key) % m_size;
        entries[i].push({.key = key, .data = value});
    }

    void remove(const K &key)
    {
        assert(hash);
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
};

uint stringHash(const String &key);

template <typename T>
class StringHashMap : public HashMap<T, String>
{

public:
    StringHashMap(uint size = HashMap<T, String>::defaultSize) : HashMap<T, String>::HashMap(size, &stringHash)
    {
    }
};
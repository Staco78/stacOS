#pragma once
#include <memory.h>
#include <debug.h>

template <typename T>
class List
{

private:
    typedef struct Node
    {
        T data;
        Node *next;
    } Node;

    Node *head = nullptr;
    Node *last = nullptr;
    uint length = 0;

public:
    struct iterator
    {
        iterator(Node *n) : node(n) {}

        iterator &operator++(int)
        {
            assert(this->node);
            this->node = this->node->next;
            return *this;
        }

        T &operator*()
        {
            return this->node->data;
        }

        const T &operator*() const
        {
            return this->node->data;
        }

        T *operator->()
        {
            return &this->node->data;
        }

        const T *operator->() const
        {
            return &this->node->data;
        }

        bool operator==(const iterator &other) const
        {
            return other.node == this->node;
        }
        bool operator!=(const iterator &other) const { return other.node != this->node; }

    private:
        Node *node;
    };

    inline bool empty()
    {
        return length == 0;
    }

    inline uint size()
    {
        return length;
    }

    void push(const T &data)
    {
        if (head == nullptr)
        {
            head = (Node *)kmalloc(sizeof(Node));
            new (head) Node();
            head->data = data;
            head->next = nullptr;
            last = head;
        }
        else
        {
            assert(last);
            last->next = (Node *)kmalloc(sizeof(Node));
            new (last->next) Node();
            last->next->data = data;
            last->next->next = nullptr;
            last = last->next;
        }

        length++;
    }

    T *get(uint index)
    {
        if (index >= length)
            return nullptr;

        Node *current = head;
        for (uint i = 0; i < index; i++)
        {
            current = current->next;
        }
        return &current->data;
    }

    void insert(uint index, T data)
    {
        assert(index <= length);

        if (index == 0)
        {
            Node *newNode = (Node *)kmalloc(sizeof(Node));
            newNode->data = data;
            newNode->next = head;
            head = newNode;
        }
        else
        {
            Node *current = head;
            for (unsigned int i = 0; i < index - 1; i++)
            {
                current = current->next;
            }
            Node *newNode = (Node *)kmalloc(sizeof(Node));
            newNode->data = data;
            newNode->next = current->next;
            current->next = newNode;

            if (current == last)
                last = newNode;
        }

        length++;
    }

    void remove(uint index)
    {
        assert(index < length);

        Node *temp;
        if (index == 0)
        {
            temp = head;
            head = head->next;
            if (length == 1)
                last = head;
        }
        else
        {
            Node *current = head;
            for (uint i = 0; i < index - 1; i++)
            {
                current = current->next;
            }
            temp = current->next;
            current->next = current->next->next;

            if (index + 1 == length)
                last = current;
        }
        kfree(temp);

        length--;
    }

    void clear()
    {
        Node *current = head;
        head = nullptr;
        for (uint i = 0; i < length; i++)
        {
            Node *next = current->next;
            kfree(current);
            current = next;
        }

        length = 0;
    }

    inline T *operator[](uint index)
    {
        return get(index);
    }

    inline iterator begin()
    {
        return iterator(head);
    }

    inline iterator end()
    {
        if (last == nullptr)
            return iterator(last);
        return iterator(last->next);
    }

    ~List()
    {
        clear();
    }
};

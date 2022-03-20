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
    unsigned int length = 0;

public:
    struct iterator
    {
        iterator(Node *n) : node(n) {}

        iterator &operator++()
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

    inline unsigned int size()
    {
        return length;
    }

    void push(T data)
    {
        if (head == nullptr)
        {
            head = (Node *)kmalloc(sizeof(Node));
            head->data = data;
            head->next = nullptr;
            last = head;
        }
        else
        {
            assert(last);
            last->next = (Node *)kmalloc(sizeof(Node));
            last->next->data = data;
            last->next->next = nullptr;
            last = last->next;
        }

        length++;
    }

    T &get(unsigned int index)
    {
        if (index < 0 || index >= length)
        {
            panic("List: Index out of range");
        }

        return last->data;
    }

    void insert(unsigned int index, T data)
    {
        if (index < 0 || index > length)
        {
            panic("List: Index out of range");
        }

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

    void remove(unsigned int index)
    {
        if (index < 0 || index >= length)
        {
            panic("List: Index out of range");
        }

        Node *temp;
        if (index == 0)
        {
            temp = head;
            head = head->next;
        }
        else
        {
            Node *current = head;
            for (unsigned int i = 0; i < index - 1; i++)
            {
                current = current->next;
            }
            temp = current->next;
            current->next = current->next->next;
        }
        kfree(temp);

        length--;
    }

    void clear()
    {
        Node *current = head;
        head = nullptr;
        for (unsigned int i = 0; i < length; i++)
        {
            Node *next = current->next;
            kfree(current);
            current = next;
        }

        length = 0;
    }

    inline T &operator[](unsigned int index)
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

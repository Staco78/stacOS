#pragma once
#include <memory.h>
#include <panic.h>

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
    unsigned int length = 0;

public:
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
        }
        else
        {
            Node *current = head;
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->next = (Node *)kmalloc(sizeof(Node));
            current->next->data = data;
            current->next->next = nullptr;
        }

        length++;
    }

    T &get(unsigned int index)
    {
        if (index < 0 || index >= length)
        {
            panic("List: Index out of range");
        }

        Node *current = head;
        for (unsigned int i = 0; i < index; i++)
        {
            current = current->next;
        }
        return current->data;
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

    ~List()
    {
        clear();
    }
};

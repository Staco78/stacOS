#pragma once
#include <memory.h>

template <typename K, typename V>
class Tree
{
private:
    struct Node
    {
        Node(K k, V v) : key(k), value(v), left(nullptr), right(nullptr) {}
        K key;
        V value;
        Node *left;
        Node *right;
    };

    Node *root = nullptr;

public:
    void insert(K key, V value)
    {
        if (root == nullptr)
        {
            root = new Node(key, value);
            return;
        }
        Node *current = root;
        while (true)
        {
            if (key == current->key)
            {
                Terminal::kprintf("Warn: tree insert: element with same key already exist\n");
                current->value = value;
                return;
            }

            Node *parent = current;
            bool goLeft = key < current->key;
            current = goLeft ? current->left : current->right;

            if (current == nullptr)
            {
                Node *newNode = new Node(key, value);
                if (goLeft)
                    parent->left = newNode;
                else
                    parent->right = newNode;
                return;
            }
        }
    }

    V *find(K key)
    {
        Node *current = root;
        while (current != nullptr)
        {
            if (key == current->key)
            {
                return &current->value;
            }

            if (key < current->key)
                current = current->left;
            else
                current = current->right;
        }

        return nullptr;
    }
};
#pragma once
#include <lib/list.h>
#include <memory.h>
#include <debug.h>

template <typename T>
class Tree
{
public:
    struct Node
    {
        T data;
        Node *parent;
        List<Node *> children;
    };

private:
    Node *root = nullptr;

public:
    inline void setRoot(T data)
    {
        root = (Node *)kmalloc(sizeof(Node));
        root->data = data;
        root->parent = nullptr;
    }

    inline Node *getRoot()
    {
        return root;
    }

    Node *addChild(Node *parent, T data)
    {
        assert(parent);
        Node *node = (Node *)kmalloc(sizeof(Node));
        node->data = data;
        node->parent = parent;
        parent->children.push(node);
        return node;
    }

    void deleteNode(Node *node)
    {
        assert(node);
        for (Node *child : node->children)
        {
            assert(child);
            deleteNode(child);
        }

        uint i = 0;
        for (auto it = node->children.begin(); it != node->children.end(); it++)
        {
            if (*it == node)
            {
                node->children.remove(i);
                return;
            }
            i++;
        }
        panic("Tree: unable to delete node: not found");
    }
};
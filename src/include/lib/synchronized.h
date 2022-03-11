#pragma once
#include <lib/queue.h>
#include <synchronization/spinlock.h>

template <typename T>
class Synchronized : public T, public Spinlock
{
};
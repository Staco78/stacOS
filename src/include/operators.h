#pragma once

#include <stddef.h>

[[nodiscard]] void *operator new(size_t count);
[[nodiscard]] void *operator new[](size_t count);
[[nodiscard]] void *operator new(size_t count, void *ptr);
[[nodiscard]] void *operator new[](size_t count, void *ptr);

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;
void operator delete(void *ptr, size_t al);
void operator delete[](void *ptr, size_t al);
#pragma once
#include <types.h>

void memcpy(void *destination, const void *source, uint64 size);
bool memcmp(const void *ptr1, const void *ptr2, uint64 size);
void memset(const void *ptr, uint8 value, uint64 size);
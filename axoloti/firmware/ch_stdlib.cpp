#include "ch_stdlib.hpp"
#include "hal.h"  // NOLINT

#include <cstdarg>
#include <cstdlib>

#include "ch.h"
#include "chprintf.h"

void* operator new(size_t size) {
    return chHeapAllocAligned(nullptr, size, 1);
}

void* operator new[](size_t size) {
    return chHeapAllocAligned(nullptr, size, 1);
}

void operator delete(void* ptr) {
    chHeapFree(ptr);
}

void operator delete(void* ptr, size_t size) {
    static_cast<void>(size);
    chHeapFree(ptr);
}

void operator delete[](void* ptr) {
    chHeapFree(ptr);
}

void operator delete[](void* ptr, size_t size) {
    static_cast<void>(size);
    chHeapFree(ptr);
}

// int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
// void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
// void __cxa_guard_abort (__guard *) {};

// void __cxa_pure_virtual(void) {};

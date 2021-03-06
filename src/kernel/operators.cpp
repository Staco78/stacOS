#include <operators.h>
#include <memory.h>
#include <debug.h>

[[nodiscard]] void *operator new(size_t count) { return kmalloc(count); }
[[nodiscard]] void *operator new[](size_t count) { return (void *)kmalloc(count); }
[[nodiscard]] void *operator new(size_t count, void *ptr) { return ptr; }
[[nodiscard]] void *operator new[](size_t count, void *ptr) { return ptr; }

void operator delete(void *ptr) noexcept { kfree(ptr); }
void operator delete[](void *ptr) noexcept { kfree(ptr); }
void operator delete(void *ptr, size_t al) { kfree(ptr); }
void operator delete[](void *ptr, size_t al) { kfree(ptr); }

void *__gxx_personality_v0;
void *_Unwind_Resume;

extern "C"
{
    void __cxa_pure_virtual()
    {
        panic("tried to call pure virtual function!");
    }

    struct atexit_func_entry_t
    {
        /*
         * Each member is at least 4 bytes large. Such that each entry is 12bytes.
         * 128 * 12 = 1.5KB exact.
         **/
        void (*destructor_func)(void *);
        void *obj_ptr;
        void *dso_handle;
    };

    volatile void *__dso_handle;
    atexit_func_entry_t __atexit_funcs[128];
    unsigned __atexit_func_count = 0;

    int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
    {
        if (__atexit_func_count >= 128)
            return -1;

        __atexit_funcs[__atexit_func_count].destructor_func = f;
        __atexit_funcs[__atexit_func_count].obj_ptr = objptr;
        __atexit_funcs[__atexit_func_count].dso_handle = dso;
        __atexit_func_count++;

        return 0;
    }

    void __cxa_finalize(void *f)
    {
        auto i = __atexit_func_count;
        if (!f)
        {
            while (i--)
            {
                if (__atexit_funcs[i].destructor_func)
                    (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            }

            return;
        }

        while (i--)
        {
            if ((void *)__atexit_funcs[i].destructor_func == f)
            {
                (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
                __atexit_funcs[i].destructor_func = 0;
            }
        }
    }
}
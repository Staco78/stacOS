#define DEFN_SYSCALL0(fn, num)      \
    long syscall_##fn()             \
    {                               \
        long a = num;               \
        __asm__ volatile("syscall"  \
                         : "=a"(a)  \
                         : "a"(a)); \
        return a;                   \
    }

#define DEFN_SYSCALL1(fn, num, P1)                           \
    long syscall_##fn(P1 p1)                                 \
    {                                                        \
        long __res = num;                                    \
        __asm__ __volatile__("syscall"                       \
                             : "=a"(__res)                   \
                             : "a"(__res), "D"((long)(p1))); \
        return __res;                                        \
    }

#define DEFN_SYSCALL2(fn, num, P1, P2)                                        \
    long syscall_##fn(P1 p1, P2 p2)                                           \
    {                                                                         \
        long __res = num;                                                     \
        __asm__ __volatile__("syscall"                                        \
                             : "=a"(__res)                                    \
                             : "a"(__res), "D"((long)(p1)), "S"((long)(p2))); \
        return __res;                                                         \
    }

#define DEFN_SYSCALL3(fn, num, P1, P2, P3)                                                     \
    long syscall_##fn(P1 p1, P2 p2, P3 p3)                                                     \
    {                                                                                          \
        long __res = num;                                                                      \
        __asm__ __volatile__("syscall"                                                         \
                             : "=a"(__res)                                                     \
                             : "a"(__res), "D"((long)(p1)), "S"((long)(p2)), "d"((long)(p3))); \
        return __res;                                                                          \
    }

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4)                                                           \
    long syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4)                                                        \
    {                                                                                                    \
        long __res = num;                                                                                \
        register long r10 asm("r10") = p4;                                                               \
        __asm__ __volatile__("syscall"                                                                   \
                             : "=a"(__res)                                                               \
                             : "a"(__res), "D"((long)(p1)), "S"((long)(p2)), "d"((long)(p3)), "r"(r10)); \
        return __res;                                                                                    \
    }

#define DEFN_SYSCALL5(fn, num, P1, P2, P3, P4, P5)                                                                \
    long syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)                                                          \
    {                                                                                                             \
        long __res = num;                                                                                         \
        register long r10 asm("r10") = p4;                                                                        \
        register long r10 asm("r8") = p5;                                                                         \
        __asm__ __volatile__("syscall"                                                                            \
                             : "=a"(__res)                                                                        \
                             : "a"(__res), "D"((long)(p1)), "S"((long)(p2)), "d"((long)(p3)), "r"(r10), "r"(r8)); \
        return __res;                                                                                             \
    }

#define DEFN_SYSCALL6(fn, num, P1, P2, P3, P4, P5, P6)                                                                     \
    long syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)                                                            \
    {                                                                                                                      \
        long __res = num;                                                                                                  \
        register long r10 asm("r10") = p4;                                                                                 \
        register long r10 asm("r8") = p5;                                                                                  \
        register long r10 asm("r9") = p6;                                                                                  \
        __asm__ __volatile__("syscall"                                                                                     \
                             : "=a"(__res)                                                                                 \
                             : "a"(__res), "D"((long)(p1)), "S"((long)(p2)), "d"((long)(p3)), "r"(r10), "r"(r8), "r"(r9)); \
        return __res;                                                                                                      \
    }

DEFN_SYSCALL1(exit, 0, int);
DEFN_SYSCALL2(open, 1, const char *, unsigned int);
DEFN_SYSCALL4(write, 2, unsigned int, void *, unsigned int, unsigned long int);
DEFN_SYSCALL4(read, 3, unsigned int, void *, unsigned int, unsigned long int);

void error(const char *e)
{
    unsigned int len = 0;
    while (e[len++])
        ;
    syscall_write(1, e, len, 0);
    syscall_exit(-1);
}

int _start()
{
    long int fd = syscall_open("/file", 1);
    if (fd < 0) error("fd < 0");
    char buffer[100];
    long e = syscall_read(fd, buffer, 100, 0);
    if (e < 0) error("e < 0");
    syscall_write(1, buffer, 100, 0);
    syscall_exit(0);
    return 0;
}
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>

#include "CurrentThread.h"

namespace
{
int g_failures = 0;

pid_t getTidSyscall()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void expectTrue(const char *name, bool condition)
{
    if (condition)
    {
        std::cout << "[OK]   " << name << std::endl;
    }
    else
    {
        std::cout << "[FAIL] " << name << std::endl;
        ++g_failures;
    }
}

void testMainThreadCache()
{
    std::cout << "=== Test 1: main thread cache ===" << std::endl;
    expectTrue("t_cachedTid starts at 0", CurrentThread::t_cachedTid == 0);

    pid_t syscall_tid = getTidSyscall();
    int tid1 = CurrentThread::tid();
    int tid2 = CurrentThread::tid();

    expectTrue("tid() matches syscall", tid1 == syscall_tid);
    expectTrue("tid() cached value stable", tid1 == tid2);
    expectTrue("t_cachedTid is non-zero", CurrentThread::t_cachedTid != 0);
    std::cout << std::endl;
}

void testWorkerThreadCache()
{
    std::cout << "=== Test 2: worker thread cache ===" << std::endl;
    struct ThreadResult
    {
        bool initial_zero = false;
        bool tid_matches = false;
        bool cached_same = false;
        bool cached_nonzero = false;
    } result;

    std::thread worker([&result]() {
        result.initial_zero = (CurrentThread::t_cachedTid == 0);
        pid_t syscall_tid = getTidSyscall();
        int tid1 = CurrentThread::tid();
        int tid2 = CurrentThread::tid();

        result.tid_matches = (tid1 == syscall_tid);
        result.cached_same = (tid1 == tid2);
        result.cached_nonzero = (CurrentThread::t_cachedTid != 0);
    });
    worker.join();

    expectTrue("thread-local cache starts at 0", result.initial_zero);
    expectTrue("tid() matches syscall in worker", result.tid_matches);
    expectTrue("tid() cached value stable in worker", result.cached_same);
    expectTrue("t_cachedTid is non-zero in worker", result.cached_nonzero);
    std::cout << std::endl;
}
} // namespace

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CurrentThread 测试程序" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    testMainThreadCache();
    testWorkerThreadCache();

    std::cout << "========================================" << std::endl;
    if (g_failures == 0)
    {
        std::cout << "   测试通过" << std::endl;
    }
    else
    {
        std::cout << "   测试失败: " << g_failures << " 项" << std::endl;
    }
    std::cout << "========================================" << std::endl;

    return g_failures == 0 ? 0 : 1;
}

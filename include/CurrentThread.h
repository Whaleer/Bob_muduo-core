#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // 线程局部存储（Thread-Local Storage, TLS）变量
    // __thread 是 GCC 扩展关键字，表示每个线程都拥有独立的 t_cachedTid 副本
    // 不同线程之间互不干扰，各自维护自己的缓存值
    // 初始化为 0，表示还未获取线程 ID
    extern __thread int t_cachedTid;

    // 缓存线程 ID 的函数
    // 在 .cc 文件中实现，通过系统调用 syscall(SYS_gettid) 获取真实的线程 ID
    // 并将结果存储到 t_cachedTid 中
    void cacheTid();

    // 获取当前线程 ID 的内联函数
    // inline 关键字建议编译器将函数内联展开，避免函数调用开销
    // 适合这种简单但可能被频繁调用的函数
    inline int tid()
    {
        // __builtin_expect 是 GCC 的内置函数，用于分支预测优化
        // 语法：__builtin_expect(exp, c)
        //   - exp: 要判断的表达式
        //   - c: 预期结果（0 表示预期为假，非 0 表示预期为真）
        // __builtin_expect(t_cachedTid == 0, 0) 的含义：
        //   告诉编译器"预期 t_cachedTid != 0"（即 t_cachedTid == 0 为假）
        //   因为大多数情况下，缓存已经被获取，不需要再次调用系统调用
        // 编译器会据此优化生成的汇编代码，将"缓存已获取"的路径作为热点路径
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            // 首次调用或缓存未设置时，调用 cacheTid() 获取线程 ID
            cacheTid();
        }
        // 返回缓存的线程 ID
        // 如果已经缓存，直接返回，无需系统调用，性能极高
        return t_cachedTid;
    }
}

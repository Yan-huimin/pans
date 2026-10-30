#ifndef PANS_INCLUDE_PANS_MUTEX_H
#define PANS_INCLUDE_PANS_MUTEX_H

#include <pthread.h>

// 64位x86（AMD64、Intel64） 或者 32 位x86
#if defined (__x86_64) || defined (__i386__)
#include <immintrin.h>  // 用于访问 SIMD 指令
// 64位arm或者32位arm
#elif defined(__aarch64__) || defined(__arm__)
#include <arm_acle.h>   // ARM C Language Extensions 的头文件，可以使用 ARM 提供的一些底层指令接口
#endif


// inline作用是告诉编译器这里是一个很小但是需要频繁调用的函数，你可以把函数调用直接展开，避免调用开销
inline void cpu_relax() noexcept
{
#if defined (__x86_64) || defined (__i386__)
    _mm_pause();
#elif defined(__aarch64__) || defined(__arm)
    __yield();
#endif
}

namespace pans 
{

// linux + glibc 环境下的自旋锁实现（使用pthread）
// 当线程无法获取锁时，不断检查锁的状态，直到锁被释放
// 不睡眠、忙等待
#if defined (__linux__) && defined (__GLIBC__)
class Spinlock
{
public:
    Spinlock() noexcept
    {
        // PTHREAD_PROCESS_PRIVATE 表示这个自旋锁只用于当前进程内部的线程之间的同步
        pthread_spin_init(&m_mutex, PTHREAD_PROCESS_PRIVATE);
    }

    ~Spinlock() noexcept
    {
        // 典型的RALL风格
        // 将资源的生命周期与对象的生命周期绑定，在构造函数中获取资源，在析构函数中释放资源。
        pthread_spin_destroy(&m_mutex);
    }

    // 这里取消掉了拷贝构造和拷贝移动赋值
    // 主要目的是为了避免生成的这个新的锁到底是继承原来的锁的状态还是创建一个新的独立锁
    // 的语义问题，因此直接设计为不允许拷贝
    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;

    void lock() noexcept
    {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() noexcept
    {
        pthread_spin_unlock(&m_mutex);
    }

private:
    // 属于 POSIX 线程 API，并不是 C++ 标准的一部分
    pthread_spinlock_t m_mutex{};
};
#else

class Spinlock
{
public:
    using Lock = std::lock_guard<Spinlock>;

    Spinlock() noexcept = default;
    ~Spinlock() noexcept = default;

    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;

    void lock() noexcept
    {
        // m_mutex.test_and_set(order); 原子地将标志设置为 true，并返回修改之前的值
        // test_and_set() 是原子的读-修改-写（Read-Modify-Write，RMW）操作。
        // 即使多个线程同时调用，也只有一个线程能在锁原来为 false 时成功获取锁。
        // 如果锁当前为false，设置为true，并返回false，直接结束循环
        // 如果锁当前为true ，设置为true，并返回true ，进入内层循环
        // 当线程成功获取锁时，acquire 能与先前释放该锁的 release 操作建立同步关系。
        // 确保当前线程能够正确观察到前一个持锁线程在临界区完成的修改。
        while(m_mutex.test_and_set(std::memory_order_acquire))
        {
            // 这层循环只是读取当前的锁的状态，并不会修改这个锁
            // 只要这个锁的状态是true，就会一直等待
            // 如果其他线程释放掉了这个锁，那么就会直接退出内层循环
            while(m_mutex.test(std::memory_order_relaxed))
            {
                // 他并不会直接要求操作系统降低该线程的CPU占用率
                // ta会向处理器提供一个提示：当前线程正在执行自旋等待循环，可以采取适当的微架构优化。
                cpu_relax();
            }
        }
    }

    [[nodiscard]] bool try_lock() noexcept
    {
        return !m_mutex.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept
    {
        m_mutex.clear(std::memory_order_release);
    }

private:
    // std::atomic_flag 属于 C++ 标准库，具有更好的跨平台可移植性
    // ATOMIC_FLAG_INIT 是标准库提供的宏，用于将 std::atomic_flag 初始化为清除状态（false）
    std::atomic_flag m_mutex = ATOMIC_FLAG_INIT;
}

#endif

}

#endif
#ifndef PANS_INCLUDE_PANS_MUTEX_H
#define PANS_INCLUDE_PANS_MUTEX_H

#include <atomic>
#include <mutex>
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
        pthread_spin_destroy(&m_mutex);
    }

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
        while(m_mutex.test_and_set(std::memory_order_acquire))
        {
            while(m_mutex.test(std::memory_order_relaxed))
            {
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
    std::atomic_flag m_mutex = ATOMIC_FLAG_INIT;
}

#endif



}

#endif
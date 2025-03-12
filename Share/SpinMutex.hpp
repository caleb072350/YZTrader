#pragma once 
#include <atomic>


class SpinMutex
{
private:
    std::atomic<bool> flag = {false};

public:
    void lock()
    {
        for (;;)
        {
            if (!flag.exchange(true, std::memory_order_acquire))
                break;
            
            while (flag.load(std::memory_order_relaxed))
            {
                __builtin_ia32_pause();
            }
        }
    }

    void unlock() {
        flag.store(false, std::memory_order_release);
    }
};
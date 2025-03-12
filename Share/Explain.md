```C++
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
```

这段代码实现了一个**自旋锁**,它基于 `std::atomic<bool>`来实现**忙等待**(`spinning`)机制，而不是使用操作系统`mutex`,从而减少线程上下文切换的开销。

- `flag` 作为 **锁标志位**,初始化为`false`(未加锁)。
- 使用 `std::atomic<bool>`来保证**原子操作**，避免数据竞争。

### 加锁(lock)
`flag.exchange(true, std::memory_order_acquire)`
- **原子交换(atomic exchange)**, 将`flag`设为`true`,表示当前线程**试图获取锁**。
- 返回交换前的值:
    - 如果`flag`之前是`false`, 说明锁未被占用，当前线程成功加锁，跳出循环。
    - 如果`flag`之前是`true`, 说明锁已被占用，进入自旋等待。

### **自旋等待**:
- 线程会不断检查`flag`,直到它变为false(即锁被释放)。
- `flag.load(std::memory_order_relaxed)`:
    - 只读取`flag`的值，不做任何同步保证(因为外部`exchange`已保证内存可见性)
- `builtin_ia32_pause()`:
    - 这是x86平台的优化指令(等价于 `pause`指令)
    - 作用:
        - 降低功耗(防止CPU过度占用执行资源)
        - 减少总线冲突(提高自旋锁性能)
    - 适用于**超线程(Hyper-Threading)**环境，防止CPU资源过度消耗。

### 解锁(unlock)
- `flag.store(false, std::memory_order_release)`:
    - 释放锁，允许其他线程进入`lock()`
    - `memory_order_release` 语义:
        - **确保`unlock()`之前的操作不会被CPU重排到`unlock()`之后**, 保证数据可见性。

## SpinMutex 特点
✔低延迟:
- 适用于**锁的持有时间非常短**的场景，(如CPU计算任务)

✔ 避免线程切换
- 适合**用户态锁**,不会调用操作系统的`sched_yield()`,减少上下文切换开销。

⚠ 高CPU占用:
-如果锁被长时间持有，等待线程会**持续消耗CPU**,浪费资源。
- **适合短时间加锁的场景**,如自旋锁在数据库索引、缓存等高频场景使用。

## SpinMutex 适用场景
🔹 线程同步短时间临界区（例如：CPU 计算任务，缓存管理）

🔹 自旋锁适合 CPU 绑定任务（高并发的 lock-free 数据结构）

🔹 不适用于长时间锁持有（否则会浪费 CPU 资源）
___

## 为什么操作系统的`std::mutex`会导致**上下文切换(context switch)**,其主要开销来源于**线程的阻塞与唤醒**。以下是详细解析:
### 1. std::mutex 的加锁机制
当多个线程竞争 `std::mutex`时：
1. **如果锁是空闲的**，调用`lock()`的线程可以**立即获得锁**(和自旋锁类似)
2. **如果锁已被占用**：
    - 线程会进入 **阻塞状态(blocked/waiting)**,由**操作系统调度器**管理。
    - 操作系统会**暂停该线程**，并切换到其他可运行的线程，这个过程会导致**上下文切换**。

### 2. 为什么会有上下文切换？
当一个线程尝试获取`std::mutex`时，如果锁已被占用，线程会进入**等待队列**，并触发**系统调用(syscall)**:
1. 线程调用 `pthread_mutex_lock()` 或 `WaitForSingleObject()`(Linux/Windows)。
2. **系统调用(syscall)进入内核态**,操作系统将线程标记为`BLOCKED`.
3. **操作系统调度器会选择其他可运行的线程执行**,当前线程被挂起。
4. 当锁可用时，操作系统**唤醒等待线程**：
- 唤醒的线程会**从内核态回到用户态**。
- 需要**恢复该线程的寄存器、栈、程序计数器等状态**。
- 线程**重新执行代码**，获取锁并继续执行。

### 3. 上下文切换的开销
(1) **系统调用开销**
- `std::mutex`需要进入`内核态(user mode -> kernel mode)`
- `syscall`是**昂贵的**,因为它设计**用户态和内核态的切换**。

(2) **线程调度开销**
- `std::mutex`可能导致线程**进入阻塞状态**,需要**操作系统调度器**进行调度。
- **调度器选择其他线程运行**,但每次切换都涉及到**寄存器、栈、TLB**等数据的恢复和保存。

(3) **CPU缓存污染**
- 上下文切换时，**CPU缓存(L1/L2/L3 Cache)可能被其他线程的数据覆盖**

___

## `memory_order_release、memory_order_acquire、memory_order_relaxed` 是 C++ `std::atomic`内存顺序(memory ordering)机制的一部分，他们用于控制**多线程环境下的内存可见性**和**指令重排**
### 1. `memory_order_release`(释放顺序)
**作用**：
- **用于写操作**,确保该操作**之前**的所有**写**操作在当前线程可见后，再让其他线程读取。
- **保证写入的可见性，但不阻止当前线程后续的指令重排**。

**示例:生产者线程:**
```cpp
std::atomic<bool> flag(false);
int data = 0;

void producer() {
    data = 42; // (1) 先写入数据
    flag.store(true, std::memory_order_release); // (2) 再发布 `flag`
}
```
📌 保证：
- (1) **不能重排到(2)之后**，确保`data = 42`先执行。
- 但(2)**之后的操作可能重排**,因为`release`仅约束**之前的写**,不影响**之后的指令**。

### 2. `memory_order_acquire`(获取顺序)
**作用:**
- **用于读操作**，确保该操作**之后**的所有读操作不会被CPU提前执行。
- **保证读取到的数据是最新的，但不阻止当前线程前面的指令重排**。

**示例:消费者线程**
```cpp
void consumer() {
    while (!flag.load(std::memory_order_acquire)); // (3) 等待 'flag' 变为true
    std::cout << data << std::endl; // (4) 读取data
}
```
📌 保证：
- (4) 不能重排到(3)之前，确保`data`在`flag == true`之后才读取。
- 但(3)**之前的指令可能重排**,因为`acquire`仅约束**之后的读**，不影响**之前的指令**。

### 3. `memory_order_acquire` + `memory_order_release`**一起使用**
```cpp
std::atomic<bool> flag(false);
int data = 0;

void producer() {
    data = 42;
    flag.store(true, std::memory_order_release);
}

void consumer() {
    while (!flag.load(std::memory_order_acquire)); // 等待生产者发布 flag
    std::cout << data << std::endl;
}
```
✅ 保证：
1. `data = 42` **发生在**`flag.store(true)`**之前(producer线程)**。
2. `flag.load(true)`**发生在`std::cout << data`**之前(consumer线程)**。
3. **消费者读取**`flag == true`**后, 保证`data`也已经更新**，不会读取旧值(**跨线程同步**)。

### 4. `memory_order_relaxed`(无序模式)
**作用:**
- **不提供任何顺序保证**,仅保证**该原子操作本身的原子性**。
- 允许CPU和编译器**自由重排**该操作前后的指令。

示例:
```cpp
std::atomic<int> counter(0);

void thread1() {
    counter.fetch_add(1, std::memory_order_relaxed); // (A)
}

void thread2() {
    int value = counter.load(std::memory_order_relaxed); // (B)
}
```
📌 保证：
- (A)和(B)**都是原子的**,但不同线程可能看到**不一致的顺序**。
- 可能的问题:
    - thread2 可能先看到`counter`增加，但其他变量的修改还没同步。
    - 适用于 **不需要同步保证的计数器、自旋锁等**。

### 5. `memory_order_relaxed` VS `release/acquire`
|操作类型 |`memory_order_relaxed`|`memory_order_release`|`memory_order_acquire`|
|----------------|-------------------|---------------------|------------------|
|**是否保证原子操作**|✔ |✔ |✔ |
|**是否保证跨线程可见性**|&#x2718;|✔|✔|
|**是否影响指令重排**|&#x2718;(允许重排)|✔(约束前序指令)|✔(约束后续指令)|
|**适用于**|计数器、日志、调试标志位|生产者线程同步|消费者线程同步|

### 6. 总结
- ✅ `memory_order_release`:确保**之前的写操作**先发生，适用于**发布数据**(写)
- ✅ `memory_order_acquire`:确保**之后的读操作**不会提前，适用于**读取数据**(读)
- ✅ `memory_order_relaxed`:**不保证顺序**,仅保证**原子性**,适用于**不需要同步的原子操作**(计数器等)。
🚀 **在线程同步中**,`acquire`+`release`**是常见的同步模式，确保跨线程数据可见性！**
---
---
---

# 内存可见性 vs. CPU指令重排

在多线程编程中，**内存可见性(Memory Visibility)**和**CPU 指令重排(Instruction Reordering)**是两个不同但相关的概念。

---
## 1. 什么是内存可见性？
**内存可见性**指的是**一个线程对数据的修改是否能被其他线程及时看到**。

***为什么会有可见性问题？***
现代cpu采用**多级缓存**(L1/L2/L3),每个CPU核心可能缓存变量值，而不是直接访问主内存。因此:
- 一个线程修改了变量，**另一个线程可能仍然看到旧值**。
- **不同CPU可能有自己的缓存副本**,数据可能不同步。

<font size="5">示例：线程之间的可见性问题</font>
```cpp
int shared_var = 0;
bool flag = false;

void write() {
    shared_var = 42; // (1) 线程1写入数据
    flag = true;     // (2) 线程1设置标志
}

void reader() {
    if (flag) {     // (3) 线程2检查标志
        std::cout << shared_var << std::endl; // (4) 可能输出 0 (未同步)
    }
}
```


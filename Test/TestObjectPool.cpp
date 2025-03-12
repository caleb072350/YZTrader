#include "../Share/ObjectPool.hpp"
#include <iostream>

struct Test
{
    int x;
    Test() : x(0) { std::cout << "Test Constructor\n"; }
    ~Test() { std::cout << "Test Destructor\n"; }
};

int main() {
    ObjectPool<Test> pool;
    Test* obj1 = pool.construct(); // 创建对象
    obj1->x = 42;
    std::cout << "obj1->x = " << obj1->x << std::endl;

    pool.destroy(obj1); // 销毁对象

    pool.release(); // 释放未使用的内存

    return 0;
}
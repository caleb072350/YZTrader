#include <iostream>
#include <thread>
#include <chrono>

int shared_var = 0;
bool flag = false;

void writer() {
    shared_var = 42; // (1) 线程1写入数据
    flag = true;     // (2) 线程1设置标志
}

void reader() {
    while (!flag);   // (3) 线程2检查标志(可能一直等待)
    std::cout << "shared_var: " << shared_var << std::endl; // (4) 可能输出 0
}

int main() {
    std::thread t1(writer);
    std::thread t2(reader);

    t1.join();
    t2.join();

    return 0;
}
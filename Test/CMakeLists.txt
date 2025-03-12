cmake_minimum_required(VERSION 3.10)
project(ObjectPoolDemo)

# 启用 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找 Boost 库
find_package(Boost REQUIRED)

# 指定源文件
add_executable(object_pool_demo TestObjectPool.cpp)

# 链接 Boost 库
target_include_directories(object_pool_demo PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(object_pool_demo PRIVATE ${Boost_LIBRARIES})

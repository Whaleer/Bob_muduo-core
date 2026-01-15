# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(test_timestamp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# 添加可执行文件
add_executable(test_timestamp test_timestamp.cc)

# 链接源文件
target_sources(test_timestamp PRIVATE
    ../src/Timestamp.cc
)

# 设置输出目录
set_target_properties(test_timestamp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

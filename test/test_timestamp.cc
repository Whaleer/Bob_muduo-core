#include <iostream>
#include <unistd.h>
#include "Timestamp.h"

// 测试辅助函数
void testDefaultConstructor() {
    std::cout << "=== 测试 1: 默认构造函数 ===" << std::endl;
    Timestamp ts;
    std::cout << "默认时间戳: " << ts.toString() << std::endl;
    std::cout << "预期: 1970/01/01 08:00:00 (UTC+8)" << std::endl;
    std::cout << std::endl;
}

void testParameterizedConstructor() {
    std::cout << "=== 测试 2: 带参数构造函数 ===" << std::endl;
    
    // 测试1: 0秒（纪元时间）
    Timestamp ts1(0);
    std::cout << "0秒: " << ts1.toString() << std::endl;
    
    // 测试2: 1000000000秒（约2001-09-09）
    Timestamp ts2(1000000000);
    std::cout << "1000000000秒: " << ts2.toString() << std::endl;
    
    // 测试3: 当前时间戳的秒数
    time_t now = time(NULL);
    Timestamp ts3(now);
    std::cout << "当前时间戳(" << now << "秒): " << ts3.toString() << std::endl;
    
    std::cout << std::endl;
}

void testNowFunction() {
    std::cout << "=== 测试 3: now() 静态函数 ===" << std::endl;
    
    // 获取当前时间
    Timestamp ts1 = Timestamp::now();
    std::cout << "当前时间1: " << ts1.toString() << std::endl;
    
    // 等待1秒
    sleep(1);
    
    // 再次获取当前时间
    Timestamp ts2 = Timestamp::now();
    std::cout << "当前时间2: " << ts2.toString() << std::endl;
    
    std::cout << "两个时间戳相差约1秒" << std::endl;
    std::cout << std::endl;
}

void testToStringFunction() {
    std::cout << "=== 测试 4: toString() 函数 ===" << std::endl;
    
    // 测试已知时间戳
    // 1642234567 = 2022-01-15 14:16:07 (UTC)
    Timestamp ts1(1642234567);
    std::string str1 = ts1.toString();
    std::cout << "1642234567秒 => " << str1 << std::endl;
    
    // 测试边界值
    Timestamp ts2(2147483647);  // 32位有符号整数最大值
    std::string str2 = ts2.toString();
    std::cout << "2147483647秒 => " << str2 << std::endl;
    
    std::cout << std::endl;
}

void testMultipleTimestamps() {
    std::cout << "=== 测试 5: 多个时间戳对比 ===" << std::endl;
    
    // 创建3个时间戳，间隔1秒
    Timestamp ts1 = Timestamp::now();
    std::cout << "时间戳1: " << ts1.toString() << std::endl;
    
    sleep(1);
    Timestamp ts2 = Timestamp::now();
    std::cout << "时间戳2: " << ts2.toString() << std::endl;
    
    sleep(1);
    Timestamp ts3 = Timestamp::now();
    std::cout << "时间戳3: " << ts3.toString() << std::endl;
    
    std::cout << std::endl;
}


void bob_test()
{
    std::cout << "Running Bob's additional tests..." << std::endl;
    // 这里可以添加 Bob 特定的测试用例
    Timestamp ts;
    Timestamp ts2(123123123123);
    std::cout << "Bob's Timestamp: " << ts.toString() << std::endl;
    std::cout << "Bob's Timestamp2: " << ts2.toString() << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Timestamp 类测试程序" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    testDefaultConstructor();
    testParameterizedConstructor();
    testNowFunction();
    testToStringFunction();
    testMultipleTimestamps();
    bob_test();
    
    std::cout << "========================================" << std::endl;
    std::cout << "   测试完成" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}

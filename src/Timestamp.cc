#include <time.h>

#include "Timestamp.h"


// 默认构造函数实现，将时间戳初始化为0（1970-01-01 00:00:00 UTC）
Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {} 

// 带参数构造函数实现
Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

/// @brief 获取当前时间戳
/// @return 当前时间的 Timestamp 对象
// 调用 time(NULL) 获取当前时间的秒数
// 将这个秒数传递给带参数的构造函数，创建并返回一个 Timestamp 对象
Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}


/*
1. 创建缓冲区 buf 用于存储格式化后的时间字符串。
2. 使用 localtime() 将 time_t 类型的微秒时间戳转换为 tm 结构体，表示本地时间。
    - tm 结构体包含：
        - tm_year: 自 1900 年以来的年数
        - tm_mon: 月份（0-11）
        - tm_mday: 一个月中的第几天（1-31）
        - tm_hour: 小时（0-23）
        - tm_min: 分钟（0-59）
        - tm_sec: 秒（0-60）
3. 使用 snprintf() 将 tm 结构体中的时间信息格式化为 "YYYY/MM/DD HH:MM:SS" 的字符串形式，并存储在 buf 中。
4. 返回格式化后的字符串。
*/
std::string Timestamp::toString() const
{
    char buf[128] = {0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}


// EOF
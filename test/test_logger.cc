#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#define MUDEBUG
#include "Logger.h"

namespace
{
int g_failures = 0;

void expectTrue(const char *name, bool condition)
{
    if (condition)
    {
        std::cout << "[OK]   " << name << std::endl;
    }
    else
    {
        std::cout << "[FAIL] " << name << std::endl;
        ++g_failures;
    }
}

std::string captureOutput(const std::function<void()> &fn)
{
    std::ostringstream oss;
    std::streambuf *old = std::cout.rdbuf(oss.rdbuf());
    fn();
    std::cout.rdbuf(old);
    return oss.str();
}

void testSingleton()
{
    std::cout << "=== Test 1: singleton ===" << std::endl;
    Logger *a = &Logger::instance();
    Logger *b = &Logger::instance();
    expectTrue("Logger::instance returns same object", a == b);
    std::cout << std::endl;
}

void testLogLevels()
{
    std::cout << "=== Test 2: log levels ===" << std::endl;
    std::string info = captureOutput([]() {
        Logger &logger = Logger::instance();
        logger.setLogLevel(INFO);
        logger.log("hello");
    });
    expectTrue("INFO prefix", info.find("[INFO]") == 0);
    expectTrue("INFO message", info.find(" : hello") != std::string::npos);

    std::string error = captureOutput([]() {
        Logger &logger = Logger::instance();
        logger.setLogLevel(ERROR);
        logger.log("oops");
    });
    expectTrue("ERROR prefix", error.find("[ERROR]") == 0);
    expectTrue("ERROR message", error.find(" : oops") != std::string::npos);

    std::string debug = captureOutput([]() {
        LOG_DEBUG("value=%d", 42);
    });
    expectTrue("DEBUG prefix", debug.find("[DEBUG]") == 0);
    expectTrue("DEBUG message", debug.find(" : value=42") != std::string::npos);
    std::cout << std::endl;
}

void testLogMacros()
{
    std::cout << "=== Test 3: log macros ===" << std::endl;
    std::string info = captureOutput([]() {
        LOG_INFO("name=%s id=%d", "bob", 7);
    });
    expectTrue("LOG_INFO format", info.find(" : name=bob id=7") != std::string::npos);

    std::string error = captureOutput([]() {
        LOG_ERROR("err=%s", "bad");
    });
    expectTrue("LOG_ERROR format", error.find(" : err=bad") != std::string::npos);
    std::cout << std::endl;
}
} // namespace

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   Logger 测试程序" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    testSingleton();
    testLogLevels();
    testLogMacros();

    std::cout << "========================================" << std::endl;
    if (g_failures == 0)
    {
        std::cout << "   测试通过" << std::endl;
    }
    else
    {
        std::cout << "   测试失败: " << g_failures << " 项" << std::endl;
    }
    std::cout << "========================================" << std::endl;

    return g_failures == 0 ? 0 : 1;
}

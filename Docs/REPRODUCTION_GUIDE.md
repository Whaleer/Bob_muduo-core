# muduo-core 复现指导文档

## 一、项目概述

muduo-core 是一个基于 Reactor 模式的高性能 C++ 网络库，采用 "one loop per thread" 的并发模型。本文档将指导你从零开始复现这个库。

## 二、架构设计总览

### 2.1 核心组件关系图

```
                    ┌──────────────┐
                    │   TcpServer  │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │   Acceptor   │ (mainloop)
                    └──────┬───────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
    ┌──────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐
    │ TcpConnection │ │EventLoopThreadPool│
    │              │ └──────────────┘ └─────┬──────┘
    │ (subloop)    │                        │
    └──────┬───────┘              ┌─────────▼─────────┐
           │                       │  EventLoopThread  │
    ┌──────▼──────┐               └─────────┬─────────┘
    │  Channel    │                         │
    └──────┬──────┘               ┌─────────▼─────────┐
           │                       │    EventLoop     │
    ┌──────▼──────┐               └─────────┬─────────┘
    │   Poller    │                         │
    │  (EPoll)    │               ┌─────────▼─────────┐
    └──────┬──────┘               │   Channel & Poller │
           │                       └───────────────────┘
    ┌──────▼──────┐
    │   Socket    │
    └─────────────┘
```

### 2.2 Reactor 模式说明

- **mainLoop (baseLoop)**: 负责监听新连接事件（通过 Acceptor）
- **subLoop**: 处理已建立连接的读写事件（通过 TcpConnection）
- **one loop per thread**: 每个线程运行一个 EventLoop

## 三、模块依赖关系分析

### 3.1 依赖层级（从底层到上层）

```
第0层（基础工具类，无依赖）:
  - noncopyable.h
  - Timestamp.h / Timestamp.cc
  - CurrentThread.h / CurrentThread.cc
  - InetAddress.h / InetAddress.cc

第1层（辅助类，依赖第0层）:
  - Logger.h / Logger.cc (依赖 noncopyable)
  - Callbacks.h (依赖 Buffer, TcpConnection, Timestamp 前向声明)
  - Buffer.h / Buffer.cc (无直接依赖)

第2层（网络底层）:
  - Socket.h / Socket.cc (依赖 noncopyable, InetAddress)

第3层（IO多路复用核心）:
  - Poller.h (依赖 Timestamp, Channel, EventLoop 前向声明)
  - EPollPoller.h / EPollPoller.cc (依赖 Poller)

第4层（事件循环核心）:
  - Channel.h / Channel.cc (依赖 Timestamp, EventLoop)
  - EventLoop.h / EventLoop.cc (依赖 Channel, Poller, CurrentThread)

第5层（并发组件）:
  - Thread.h / Thread.cc (依赖 noncopyable, CurrentThread)
  - EventLoopThread.h / EventLoopThread.cc (依赖 Thread, EventLoop)
  - EventLoopThreadPool.h / EventLoopThreadPool.cc (依赖 EventLoopThread)

第6层（网络应用层）:
  - Acceptor.h / Acceptor.cc (依赖 Socket, Channel, EventLoop)
  - TcpConnection.h / TcpConnection.cc (依赖 EventLoop, InetAddress, Callbacks, Buffer, Socket, Channel)
  - TcpServer.h / TcpServer.cc (依赖 EventLoop, Acceptor, EventLoopThreadPool, Callbacks, TcpConnection)
```

## 四、复现顺序建议

### 方案一：自底向上（推荐新手）

按照依赖关系从底层到上层逐个实现，这样可以确保每层都能独立测试。

#### 阶段1：基础工具类（第0-1层）

**优先级：★★★★★**

1. **noncopyable.h** - 最简单，直接复制
   - 禁止拷贝构造和赋值的基类

2. **Timestamp.h / Timestamp.cc**
   - 封装时间戳，实现 now() 和 toString()

3. **CurrentThread.h / CurrentThread.cc**
   - 线程ID缓存，避免频繁系统调用
   - 理解 __thread 关键字和 __builtin_expect

4. **InetAddress.h / InetAddress.cc**
   - 封装 sockaddr_in
   - 实现 toIp(), toIpPort(), toPort()

5. **Logger.h / Logger.cc**
   - 单例模式的日志类
   - 理解宏定义的用法（LOG_INFO, LOG_ERROR等）

6. **Buffer.h / Buffer.cc**
   - 网络缓冲区设计（readerIndex, writerIndex）
   - 理解 makeSpace 的空间复用逻辑
   - 实现 readFd 和 writeFd（使用 readv/writev）

#### 阶段2：网络底层（第2层）

**优先级：★★★★☆**

7. **Socket.h / Socket.cc**
   - 封装 socket 系统调用
   - 实现 bind, listen, accept, shutdownWrite
   - 理解 TCP 选项（TCP_NODELAY, SO_REUSEADDR等）

#### 阶段3：IO多路复用核心（第3层）

**优先级：★★★★★**

8. **Poller.h** - 抽象基类，只定义接口
   - poll(), updateChannel(), removeChannel()

9. **EPollPoller.h / EPollPoller.cc**
   - 实现 epoll_ctl, epoll_wait
   - 理解 activeChannels_ 的填充逻辑
   - 实现 updateChannel 的三种状态判断

#### 阶段4：事件循环核心（第4层）

**优先级：★★★★★**

10. **Channel.h / Channel.cc**
    - 封装 fd 和感兴趣的事件
    - 理解 tie_ 机制（防止跨线程释放）
    - 实现 handleEvent 的分发逻辑

11. **EventLoop.h / EventLoop.cc**
    - **核心类！** 实现 loop() 主循环
    - 理解 wakeupFd_ 和 pendingFunctors_
    - 实现 runInLoop 和 queueInLoop
    - 理解"one loop per thread"的保证机制

#### 阶段5：并发组件（第5层）

**优先级：★★★☆☆**

12. **Thread.h / Thread.cc**
    - 封装 std::thread
    - 在线程中保存 tid

13. **EventLoopThread.h / EventLoopThread.cc**
    - 包含一个 Thread 和一个 EventLoop
    - 理解条件变量的使用

14. **EventLoopThreadPool.h / EventLoopThreadPool.cc**
    - 管理多个 EventLoopThread
    - 实现 getNextLoop 轮询算法

#### 阶段6：网络应用层（第6层）

**优先级：★★★★★**

15. **Acceptor.h / Acceptor.cc**
    - 监听新连接
    - handleRead 中调用 accept() 并触发回调

16. **TcpConnection.h / TcpConnection.cc**
    - **核心类！** 管理 TCP 连接的完整生命周期
    - 理解 state_ 状态机
    - 实现 send, shutdown 的线程安全
    - 理解 inputBuffer_ 和 outputBuffer_

17. **TcpServer.h / TcpServer.cc**
    - **核心类！** 对外提供的服务器接口
    - newConnection 的轮询分配
    - connections_ 的管理

---

### 方案二：自顶向下（适合有经验的开发者）

先实现用户接口层，使用桩函数，然后逐步填充底层实现。

1. 先实现 TcpServer 和 TcpConnection 的接口定义
2. 使用简单的桩函数填充依赖
3. 逐步替换为真正的实现

---

## 五、关键难点解析

### 5.1 EventLoop 的跨线程通信

**问题**：如何在其他线程中唤醒 EventLoop？

**解决**：
- 使用 eventfd 创建 wakeupFd_
- 当其他线程调用 queueInLoop() 时，调用 wakeup()
- 在 loop() 中检测到 wakeupFd_ 可读时，执行 handleRead()

### 5.2 TcpConnection 的对象生命周期

**问题**：如何确保 TcpConnection 在回调执行期间不被析构？

**解决**：
- 使用 shared_ptr 管理 TcpConnection
- Channel 的 tie_ 机制绑定 shared_ptr
- 在 handleEvent 前提升为强引用

### 5.3 Buffer 的空间管理

**问题**：如何避免频繁的内存分配？

**解决**：
- 使用 vector<char> 作为底层存储
- makeSpace() 复用已读空间
- 只在必要时才扩容（resize）

### 5.4 one loop per thread 的保证

**问题**：如何确保每个线程只有一个 EventLoop？

**解决**：
- EventLoop 构造时记录线程ID到 threadId_
- 提供 isInLoopThread() 方法
- 跨线程调用使用 runInLoop() 和 queueInLoop()

## 六、测试建议

### 6.1 单元测试

每个模块实现后，建议编写单元测试：

```cpp
// 示例：测试 Buffer
void testBuffer() {
    Buffer buf;
    buf.append("hello", 5);
    assert(buf.readableBytes() == 5);
    assert(buf.retrieveAsString() == "hello");
}
```

### 6.2 集成测试

在完成 EventLoop 和 Poller 后，可以测试简单的事件循环：

```cpp
void testEventLoop() {
    EventLoop loop;
    // 添加 timer 测试
    // 添加 channel 测试
    loop.loop();
}
```

### 6.3 完整测试

使用 example/testserver.cc 作为最终测试：

```bash
cd build
cmake ..
make
./example/testserver
```

## 七、编译配置

### 7.1 CMakeLists.txt 结构

```
muduo-core/
├── CMakeLists.txt          # 主CMake配置
├── src/
│   └── CMakeLists.txt      # 构建muduo_core库
├── example/
│   └── CMakeLists.txt      # 构建示例程序
```

### 7.2 关键编译选项

```cmake
set(CMAKE_CXX_STANDARD 11)  # C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")  # 线程支持
```

## 八、常见问题

### Q1: 为什么要先实现 Logger？

**A**: Logger 是调试的关键工具，在后续开发中需要频繁使用日志输出。

### Q2: EPollPoller 为什么要继承 Poller？

**A**: 策略模式设计，方便后续扩展其他 IO 复用方式（如 select, kqueue）。

### Q3: TcpConnection 为什么要使用 enable_shared_from_this？

**A**: 需要在成员函数中获取指向当前对象的 shared_ptr，用于回调传递。

### Q4: 为什么 Channel 不能跨线程使用？

**A**: Channel 的 loop_ 成员指向 EventLoop，而 EventLoop 绑定了线程ID。

## 九、学习资源

1. **书籍**：《Linux多线程服务器编程-使用 muduo C++网络库》- 陈硕
2. **GitHub 官方**：https://github.com/chenshuo/muduo
3. **网络编程基础**：epoll, socket, TCP/IP 协议栈
4. **多线程编程**：pthread, std::thread, 条件变量, 互斥锁

## 十、复现检查清单

### 基础阶段
- [ ] noncopyable.h
- [ ] Timestamp (头文件和实现)
- [ ] CurrentThread (头文件和实现)
- [ ] InetAddress (头文件和实现)
- [ ] Logger (头文件和实现)
- [ ] Buffer (头文件和实现)

### 网络底层
- [ ] Socket (头文件和实现)
- [ ] Poller (头文件)
- [ ] EPollPoller (头文件和实现)

### 事件循环
- [ ] Channel (头文件和实现)
- [ ] EventLoop (头文件和实现)

### 并发组件
- [ ] Thread (头文件和实现)
- [ ] EventLoopThread (头文件和实现)
- [ ] EventLoopThreadPool (头文件和实现)

### 应用层
- [ ] Acceptor (头文件和实现)
- [ ] TcpConnection (头文件和实现)
- [ ] TcpServer (头文件和实现)

### 测试
- [ ] 单元测试通过
- [ ] example/testserver 运行正常

## 十一、时间预估

基于不同的 C++ 水平和网络编程经验：

- **新手**：3-4 周（每个阶段 4-5 天）
- **有经验**：1-2 周
- **精通者**：1 周内

## 十二、建议的开发顺序（总结）

**推荐顺序**：

1. Day 1-2: noncopyable, Timestamp, CurrentThread, InetAddress
2. Day 3-4: Logger, Buffer
3. Day 5: Socket
4. Day 6-7: Poller, EPollPoller
5. Day 8-10: Channel, EventLoop（最关键）
6. Day 11-13: Thread, EventLoopThread, EventLoopThreadPool
7. Day 14-17: Acceptor, TcpConnection, TcpServer
8. Day 18-20: 测试和调试

**核心建议**：
- EventLoop 是核心中的核心，一定要理解透彻
- 先让代码能编译通过，再逐步完善功能
- 善用日志输出调试
- 每完成一个模块就进行单元测试

祝你复现顺利！

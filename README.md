# Webserver项目简介

使用C++实现的高性能web服务器，经过webbenchh压力测试可以实现上万的QPS。

## 功能

- 利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型；
- 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求；
- 利用标准库容器vector封装char，实现自动增长的缓冲区；
- 基于堆结构实现的定时器，关闭超时的非活动连接；
- 基于C++ 11特性实现了线程池；

##  环境

- Linux
- C++ 11
- CMake 3.5.1

## 项目启动

```
mkdir build
cd build
cmake ..
make
cd ../
./build/webserver
```

## 压力测试

```
./webbench-1.5/webbench -c 10 -t 10 http://ip:port/
./webbench-1.5/webbench -c 100 -t 10 http://ip:port/
./webbench-1.5/webbench -c 1000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 10000 -t 10 http://ip:port/
```

![压力测试](/Users/mazhiwei/Desktop/webserver/压力测试.png)

测试环境: Ubuntu: 16.04

|   用户数   |  10   |  100  | 1000  | 10000 |
| :--: | :---: | :---: | :---: | :---: |
| QPS  | 4366 | 12287 | 13107 |  118  |

## 致谢

Linux高性能服务器编程，游双著.


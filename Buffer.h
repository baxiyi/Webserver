//
// Created by Administrator on 2021/12/8.
//

#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include<vector>
#include<iostream>
#include<string>
#include<assert.h>
#include<string.h>
#include<sys/uio.h>
#include<unistd.h>


class Buffer {
public:
    Buffer(int buffer_size = 1024);
    ~Buffer() = default;

    // 获取当前可写字节数
    size_t writeableBytes() const;
    // 获取当前可读字节数
    size_t readableBytes() const;
    // 获取当前已读字节数
    size_t readBytes() const;
    // 获取当前读写指针位置
    const char* readPtr() const;
    const char* writePtrConst() const;
    char* writePtr();
    // 更新读写位置
    void updateReadPtr(size_t len);
    void updateReadPtrUntilEnd(const char* end);
    void updateWritePtr(size_t len);
    // 初始化
    void initBuffer();
    // 确保数据写入缓冲区
    void ensureWritable(size_t len);
    // 将数据写入缓冲区
    void append(const char* str, size_t len);
    void append(const std::string& str);
    void append(const void* data, size_t len);
    void append(const Buffer& buffer);
    // IO操作读写
    ssize_t readFd(int fd, int* Errno);
    ssize_t writeFd(int fd, int* Errno);
    // 将缓冲区数据转化为字符串
    std::string bufferToString();

private:
    // 缓冲区起始位置
    char* BeginPtr_();
    const char* BeginPtr_() const;
    // 扩容
    void allocateSpace(size_t len);
    std::vector<char> buffer_; // buffer实体
    size_t readPos_; // 读指针
    size_t writePos_; // 写指针
};


#endif //WEBSERVER_BUFFER_H

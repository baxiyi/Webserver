//
// Created by Administrator on 2022/3/7.
//

#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H

#include<arpa/inet.h> //sockaddr_in
#include<sys/uio.h> //readv/writev
#include<iostream>
#include<sys/types.h>
#include<assert.h>
#include<atomic>

#include "Buffer.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTPConnection {
public:
    HTTPConnection();
    ~HTTPConnection();
    void initHTTPConn(int socketFd, const sockaddr_in& addr);

    // 从socket读入
    ssize_t readBuffer(int* saveErrno);
    // 向socket写入
    ssize_t writeBuffer(int* saveErrno);

    void closeHTTPConn();
    // 处理HTTP连接，解析request，生成response
    bool handleHTTPConn();

    const char* getIP() const;
    int getPort();
    int getFd();
    sockaddr_in getAddr() const;

    int writeBytes()
    {
        return iov_[1].iov_len + iov_[0].iov_len;
    }

    bool isKeepAlive() const
    {
        return request_.isKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
private:
    int fd_;
    struct sockaddr_in addr_;
    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuffer_;
    Buffer writeBuffer_;

    HTTPRequest request_;
    HTTPResponse response_;

};


#endif //WEBSERVER_HTTPCONNECTION_H

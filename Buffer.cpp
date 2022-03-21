//
// Created by Administrator on 2021/12/8.
//

#include "Buffer.h"

Buffer::Buffer(int buffer_size):buffer_(std::vector<char>(buffer_size)), readPos_(0), writePos_(0) {}

size_t Buffer::readableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::writeableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::readBytes() const {
    return readPos_;
}

const char* Buffer::readPtr() const {
    return BeginPtr_() + readPos_;
}

const char* Buffer::writePtrConst() const {
    return BeginPtr_() + writePos_;
}

char* Buffer::writePtr() {
    return BeginPtr_() + writePos_;
}

void Buffer::updateReadPtr(size_t len) {
    assert(len <= readableBytes());
    readPos_ += len;
}

void Buffer::updateReadPtrUntilEnd(const char* end)
{
    assert(end>=readPtr());
    updateReadPtr(end-readPtr());
}

void Buffer::updateWritePtr(size_t len) {
    assert(len <= writeableBytes());
    writePos_ += len;
}

void Buffer::initBuffer() {
    std::fill(buffer_.begin(), buffer_.end(), 0);
    readPos_ = 0;
    writePos_ = 0;
}

void Buffer::allocateSpace(size_t len) {
    if (writeableBytes() + readBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = readableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable = readableBytes());
    }
}

void Buffer::ensureWritable(size_t len) {
    if (writeableBytes() < len) {
        allocateSpace(len);
    }
    assert(writeableBytes() >= len);
}

void Buffer::append(const char *str, size_t len) {
    ensureWritable(len);
    std::copy(str, str + len, writePtr());
    updateWritePtr(len);
}

void Buffer::append(const std::string &str) {
    append(str.data(), str.length());
}

void Buffer::append(const void *data, size_t len) {
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const Buffer &buffer) {
    append(buffer.readPtr(), readableBytes());
}

ssize_t Buffer::readFd(int fd, int* Errno) {
    char buf[65535];
    struct iovec iov[2];
    const size_t writeable = writeableBytes();
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writeable;
    iov[1].iov_base = buf;
    iov[1].iov_len = sizeof(buf);

    const ssize_t len = readv(fd, iov, 2);

    if (len < 0) {
        *Errno = errno;
    } else if (static_cast<size_t>(len) <= writeable) {
        writePos_ += len;
    } else {
        writePos_ = buffer_.size();
        append(buf, len - writeable);
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int* Errno) {
    const int readable = readableBytes();
    ssize_t len = write(fd, readPtr(), readable);
    if (len < 0) {
        *Errno = errno;
    } else {
        readPos_ += len;
    }
    return len;
}

std::string Buffer::bufferToString() {
    std::string str(readPtr(), readableBytes());
    initBuffer();
    return str;
}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}
//
// Created by Administrator on 2022/3/7.
//

#include "HTTPRequest.h"

const std::unordered_set<std::string> HTTPRequest::DEFAULT_HTML{
        "/index", "/welcome", "/video", "/picture"};

void HTTPRequest::init() {
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HTTPRequest::isKeepAlive() const {
    if (header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HTTPRequest::parse(Buffer &buff) {
    const char *CRLF = "\r\n";
    if (buff.readableBytes() <= 0) {
        return false;
    }
    while (buff.readableBytes() && state_ != FINISH) {
        const char* lineEnd = std::search(buff.readPtr(), buff.writePtrConst(), CRLF, CRLF + 2);
        std::string line(buff.readPtr(), lineEnd);
        switch(state_) {
            case REQUEST_LINE:
                if (!parseRequestLine_(line)) {
                    return false;
                }
                parsePath_();
                break;
            case HEADERS:
                parseRequestHeader_(line);
                if (buff.readableBytes() <= 2) {
                    state_ = FINISH;
                }
                break;
            case BODY:
                parseDataBody_(line);
                break;
            default:
                break;
        }
        if (lineEnd == buff.writePtr()) {
            break;
        }
        buff.updateReadPtrUntilEnd(lineEnd + 2);
    }
    return true;
}

void HTTPRequest::parsePath_() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto &item : DEFAULT_HTML) {
            if (item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

bool HTTPRequest::parseRequestLine_(const std::string &line) {
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, pattern)) {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    return false;
}
void HTTPRequest::parseRequestHeader_(const std::string &line) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, pattern)) {
        header_[subMatch[1]] = header_[subMatch[2]];
    } else {
        state_ = BODY;
    }
}

void HTTPRequest::parseDataBody_(const std::string &line) {
    body_ = line;
    parsePost_();
    state_ = FINISH;
}

int HTTPRequest::convertHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HTTPRequest::parsePost_() {
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        if(body_.size() == 0) { return; }

        std::string key, value;
        int num = 0;
        int n = body_.size();
        int i = 0, j = 0;

        for(; i < n; i++) {
            char ch = body_[i];
            switch (ch) {
                case '=':
                    key = body_.substr(j, i - j);
                    j = i + 1;
                    break;
                case '+':
                    body_[i] = ' ';
                    break;
                case '%':
                    num = convertHex(body_[i + 1]) * 16 + convertHex(body_[i + 2]);
                    body_[i + 2] = num % 10 + '0';
                    body_[i + 1] = num / 10 + '0';
                    i += 2;
                    break;
                case '&':
                    value = body_.substr(j, i - j);
                    j = i + 1;
                    post_[key] = value;
                    break;
                default:
                    break;
            }
        }
        assert(j <= i);
        if(post_.count(key) == 0 && j < i) {
            value = body_.substr(j, i - j);
            post_[key] = value;
        }
    }
}

std::string HTTPRequest::path() const{
    return path_;
}

std::string& HTTPRequest::path(){
    return path_;
}
std::string HTTPRequest::method() const {
    return method_;
}

std::string HTTPRequest::version() const {
    return version_;
}

std::string HTTPRequest::getPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HTTPRequest::getPost(const char* key) const {
    assert(key != nullptr);
    if (post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}
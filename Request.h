#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>

struct Request {
    std::string method;
    std::string url;
    std::string headers;
    std::string body;
};

#endif // REQUEST_H

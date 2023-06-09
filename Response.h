#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <map>
#include <utility>

struct Response {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string content_type;

    Response(std::string body, int status)
            : body(std::move(body)), status(status) {}

    Response(std::string body, std::string content_type, int status)
            : body(std::move(body)), content_type(std::move(content_type)), status(status) {}
};

#endif // RESPONSE_H

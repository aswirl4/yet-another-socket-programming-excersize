#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <map>

struct Response {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;

    Response(const std::string& body, int status)
            : body(body), status(status) { }
};

#endif // RESPONSE_H

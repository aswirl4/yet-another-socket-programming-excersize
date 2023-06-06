#include <iostream>
#include "Request.h"
#include "Response.h"
#include "utils.h"

Request parse_request(const std::string &request_str) {
    Request request;

    //print request
    //print horizon line
    std::cout << "----------request_str begin---------------------" << std::endl;
    std::cout << request_str << std::endl;
    std::cout << "----------request_str end-----------------------" << std::endl;

    size_t pos = request_str.find(' ');
    request.method = request_str.substr(0, pos);

    size_t pos2 = request_str.find(' ', pos + 1);
    request.url = request_str.substr(pos + 1, pos2 - pos - 1);

    // handle headers body
    size_t pos3 = request_str.find("\r\n\r\n", pos2 + 1);
    request.headers = request_str.substr(request_str.find("\r\n", pos2 + 1) + 2, pos3 - pos2 - 2);
    request.body = request_str.substr(pos3 + 4);
    return request;
}

std::string format_response(const Response &response) {
    std::string response_str;

    // 这里只是一个简单的例子，没有处理headers
    // 实际的实现可能需要添加Content-Type，Content-Length等头部
    response_str = "HTTP/1.1 " + std::to_string(response.status)
                   + " OK\r\n"
                   + (response.content_type.empty() ? "Content-Type: text/html\r\n" : response.content_type)
                   + "Content-Length: " + std::to_string(response.body.size())
                   + "\r\n\r\n" + response.body;

    return response_str;
}
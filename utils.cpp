#include "Request.h"
#include "Response.h"
#include "utils.h"

Request parse_request(const std::string& request_str) {
    Request request;

    // 这里只是一个简单的例子，没有处理headers和body
    // 实际的实现可能需要更复杂的字符串操作
    size_t pos = request_str.find(' ');
    request.method = request_str.substr(0, pos);

    size_t pos2 = request_str.find(' ', pos + 1);
    request.url = request_str.substr(pos + 1, pos2 - pos - 1);

    return request;
}

std::string format_response(const Response& response) {
    std::string response_str;

    // 这里只是一个简单的例子，没有处理headers
    // 实际的实现可能需要添加Content-Type，Content-Length等头部
    response_str = "HTTP/1.1 " + std::to_string(response.status) + "\r\n\r\n" + response.body;

    return response_str;
}

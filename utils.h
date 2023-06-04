#ifndef UTILS_H
#define UTILS_H

#include "Request.h"
#include "Response.h"

Request parse_request(const std::string& request_str);
std::string format_response(const Response& response);

#endif // UTILS_H

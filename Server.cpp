#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "Request.h"
#include "Response.h"
#include "utils.h" // 此头文件需要后续创建，用于包含parse_request和format_response函数的声明

#include <ctime>
#include <fstream>
#include <sstream>

Server::Server(const Config& config): config(config), file_dir("filesSharing") {
    server_sockfd = create_server(config.port);
}

void Server::start() {
    listen(server_sockfd, 5);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_addr_len);

        handle_request(client_sockfd);

        close(client_sockfd);
    }
}

int Server::create_server(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    return sockfd;
}

void Server::handle_request(int client_sockfd) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);

    Request request = parse_request(std::string(buffer));

    Response response;

    if (request.url == "/chatRoom") {
        response = handle_chat_request(request);
    } else if (request.url == "/filesSharing") {
        response = handle_file_request(request);
    } else {
        response.status = 404;
    }

    std::string response_str = format_response(response);
    send(client_sockfd, response_str.c_str(), response_str.size(), 0);
}

Response Server::handle_chat_request(const Request& request) {
    std::string response_body;
    int response_status;

    // Assuming that chat_room has a method called add_message
    if (chat_room.add_message(request.body)) {
        response_body = "Message added successfully at " + std::to_string(std::time(nullptr));
        response_status = 200;
    } else {
        response_body = "Failed to add message";
        response_status = 500;
    }

    return Response(response_body, response_status);
}

Response Server::handle_file_request(const Request& request) {
    std::ifstream file(request.path, std::ios::binary);

    if (file.is_open()) {
        std::ostringstream oss;
        oss << file.rdbuf();

        return Response(oss.str(), 200);
    } else {
        return Response("Failed to open file", 500);
    }
}
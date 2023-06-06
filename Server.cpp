#include <filesystem>
#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "Request.h"
#include "Response.h"
#include "utils.h" // 此头文件需要后续创建，用于包含parse_request和format_response函数的声明
#include <csignal>
#include <fstream>
#include <sstream>
#include <iostream>

// the reason of this value's existence is that we need to close server_sockfd in handle_sigint
// this is the easiest way to do it; pass value to handle_sigint function is goddamn hard
int global_server_sockfd;

void handle_sigint(int sig) {
    // print close ifo
    std::cout << "Server is closed" << std::endl;
    close(global_server_sockfd);
    exit(0);
}


std::string Server::file_dir = "./filesSharing/";

Server::Server(const Config &config) : config(config) {
    server_sockfd = create_server(config.port);
    chat_room = ChatRoom(config.message_lifetime, config.message_rate_limit);
}

[[noreturn]] void Server::start() {
    listen(server_sockfd, 5);
    global_server_sockfd = server_sockfd;
    signal(SIGINT, handle_sigint);
    //print server info
    std::cout << "Server is listening on port " << config.port << std::endl;

    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);

        int client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_addr_len);

        handle_request(client_sockfd);

        ::close(client_sockfd);
    }
}


/*
 * this code only handle ipv4 connection.
int Server::create_server(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    return sockfd;
}
*/
int Server::create_server(int port) {
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);

    struct sockaddr_in6 server_addr{};
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    server_addr.sin6_addr = in6addr_any;

    // 这行代码允许 IPv4 流量在 IPv6 套接字上使用。
    int no = 0;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &no, sizeof(no)) == -1) {
        perror("setsockopt");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        return -1;
    }

    return sockfd;
}

void Server::handle_request(int client_sockfd) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);

    Request request = parse_request(std::string(buffer));

    //print chat history
    //print horizon line
    std::cout << "--------chat history begin--------------" << std::endl;
    std::cout << "chat history is " << chat_room.get_chat_history() << std::endl;
    std::cout << "--------chat history end----------------" << std::endl;

    Response response("", 200);
    if (request.url == "/chatRoom") {
        response = handle_chat_request(request);
        // 请求的目录包含/filesSharing
    } else if (request.url.find("/filesSharing") == 0) {
        response = handle_file_request(request);
    } else {
        response.status = 404;
    }
    std::string response_str = format_response(response);
    send(client_sockfd, response_str.c_str(), response_str.size(), 0);
    std::cout << "----------response end------------------" << std::endl;

}

Response Server::handle_chat_request(const Request &request) {
    std::string response_body;
    int response_status;

    // Assuming that chat_room has a method called add_message
    if (chat_room.add_message(request.body)) {
        response_body = request.body;
        response_status = 200;
    } else {
        response_body = "Failed to add message";
        response_status = 500;
    }
    response_body +=
            R"delimiter(<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>发送HTTP请求</title>
    <script>
        function sendData() {
            var input = document.getElementById("inputText").value;
            var url = window.location.href;

            var xhr = new XMLHttpRequest();
            xhr.open("POST", url, true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    // 请求成功后的处理逻辑
                    console.log(xhr.responseText);
                }
            };
            xhr.send(input);
        }

        function refreshPage() {
            window.location.reload();
        }
    </script>
</head>
<body>
<pre>
current chat history is:
----------------------------------------
)delimiter"
            + chat_room.get_chat_history()
            + R"delimiter(
</pre>
    <input type="text" id="inputText" autofocus>
    <button onclick="sendData()">发送</button>
    <button onclick="refreshPage()">刷新页面</button>
</body>
</html>)delimiter";
// 可以写的更好, 但是就10分分值, 不值得.
    return {response_body, response_status};
}

std::string getFileList(const std::string &directoryPath) {
    std::filesystem::path path(directoryPath);
    std::string fileList;

    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            fileList += entry.path().filename().string() + "\n";
        }
    }

    return fileList;
}

Response Server::handle_file_request(const Request &request) {
    std::string response_body;
    int response_status;
    response_status = 200;
    if (request.method == "GET") {
        if (request.url == "/filesSharing") {
            response_body = getFileList("./filesSharing");
            response_body =
                    R"delimiter(<!DOCTYPE html>
                    <html>
                      <head>
                        <meta charset="UTF-8">
                        <title>文件上传示例</title>
                      </head>
                      <body>
                        <h1>文件上传示例; 需要花里胡哨的web也可以做.</h1>
                        <input type="file" id="fileInput">
                        <button id="uploadButton" onclick="upload()">上传文件</button>
                    <pre>
                    current files list is:
                    ----------------------------------------
)delimiter"
                    + response_body
                    +
                    R"delimiter(
</pre>
                        <script>
                          function upload() {
                            const fileInput = document.getElementById('fileInput');
                            const file = fileInput.files[0];
                            if (!file) {
                              alert('请选择要上传的文件！');
                              return;
                            }

                            // 禁用按钮
                            const uploadButton = document.getElementById('uploadButton');
                            uploadButton.disabled = true;

                            // 构造请求对象
                            const xhr = new XMLHttpRequest();
                            xhr.open('POST', '/filesSharing');
                            xhr.setRequestHeader('Content-Type', file.type); // 设置请求头，发送文件类型
                            xhr.onreadystatechange = function() {
                              if (xhr.readyState === 4) {
                                // 上传完成，启用按钮
                                uploadButton.disabled = false;
                                alert('文件上传成功！');
                                window.location.reload();
                              }
                            };

                            // 发送请求
                            xhr.setRequestHeader('Content-Disposition', 'filename=' + file.name);
                            xhr.send(file);

                            // 提示用户等待上传完成
                            alert('正在上传文件，请勿操作！');
                          }
                        </script>
                      </body>
                    </html>)delimiter";
            return {response_body, response_status};
        } else {
            std::string content_type = "Content-Type: octet-stream\r\n";
            std::string file_path = request.url;
            std::ofstream file;
            file.open("." + request.url, std::ios::in | std::ios::binary);
            std::stringstream strstream;
            strstream << file.rdbuf();
            response_body = strstream.str();
            return {response_body, content_type, response_status};
        }
    } else if (request.method == "POST") {
        if (request.body.empty()) {
            return {response_body, response_status};
        }
        auto pos = request.headers.find("Content-Length: ") + strlen("Content-Length: ");

        auto content_length = request.headers.substr(pos, request.body.find("\r\n", pos) - pos);
        // print content_length
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "content_length: " << content_length << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // find string "filename=" in request.body from pos.
        pos = request.headers.find("filename=") + 9;
        std::string file_name = request.headers.substr(pos, request.headers.find("\r\n", pos) - pos);

        //print file_name
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "file_name: " << file_name << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        std::ofstream OsWrite(file_dir + file_name, std::ofstream::trunc);

//        std::cout << "am i writing?: "
//                     "\n file_dir"
//                     + file_dir + file_name
//                  << request.body;

        OsWrite << request.body;
        OsWrite << std::endl;
        return {response_body, response_status};
    }
    return {response_body, response_status};
}

int Server::get_server_sockfd() const {
    return server_sockfd;
}



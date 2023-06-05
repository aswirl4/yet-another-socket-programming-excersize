#include <filesystem>
#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "Request.h"
#include "Response.h"
#include "utils.h" // 此头文件需要后续创建，用于包含parse_request和format_response函数的声明

#include <fstream>
#include <sstream>
#include <iostream>

Server::Server(const Config &config) : config(config) {
    server_sockfd = create_server(config.port);
    chat_room = ChatRoom(config.message_lifetime, config.message_rate_limit);
    file_dir = std::string("./filesSharing");
}

[[noreturn]] void Server::start() {
    listen(server_sockfd, 5);

    //print server info
    std::cout << "Server is listening on port " << config.port << std::endl;

    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);

        int client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_addr_len);

        handle_request(client_sockfd);

        close(client_sockfd);
    }
}

/*
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
    std::cout << "chat history is " << chat_room.get_chat_history() << std::endl;

    //print request.url
    std::cout << "request url is " << request.url << std::endl;

    //print request body
    std::cout << "request body is " << request.body << std::endl;

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
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset=\"utf-8\">\n"
            "    <title>发送HTTP请求</title>\n"
            "    <script>\n"
            "        function sendData() {\n"
            "            var input = document.getElementById(\"inputText\").value;\n"
            "            var url = window.location.href;\n"
            "\n"
            "            var xhr = new XMLHttpRequest();\n"
            "            xhr.open(\"POST\", url, true);\n"
            "            xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\n"
            "            xhr.onreadystatechange = function () {\n"
            "                if (xhr.readyState === 4 && xhr.status === 200) {\n"
            "                    // 请求成功后的处理逻辑\n"
            "                    console.log(xhr.responseText);\n"
            "                }\n"
            "            };\n"
            "            xhr.send(input);\n"
            "        }\n"
            "\n"
            "        function refreshPage() {\n"
            "            window.location.reload();\n"
            "        }\n"
            "    </script>\n"
            "</head>\n"
            "<body>\n"
            "<pre>"
            "\ncurrent chat history is: \n"
            "----------------------------------------\n"
            + chat_room.get_chat_history()
            +
            "</pre>\n"
            "    <input type=\"text\" id=\"inputText\" autofocus>\n"
            "    <button onclick=\"sendData()\">发送</button>\n"
            "    <button onclick=\"refreshPage()\">刷新页面</button>\n"
            "</body>\n"
            "</html>\n";
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
//                    "<!DOCTYPE html>\n"
//                    "<html>\n"
//                    "<head>\n"
//                    "    <meta charset=\"utf-8\">\n"
//                    "    <title>发送HTTP请求</title>\n"
//                    "    <script>\n"
//                    "    </script>\n"
//                    "</head>\n"
//                    "<body>\n"
//                    "  <h1>文件上传</h1>\n"
//                    "  \n"
//                    "  <form action=\"#\" method=\"POST\" enctype=\"multipart/form-data\">\n"
//                    "    <input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\">\n"
//                    "    <input type=\"submit\" value=\"upload file\" name=\"submit\">\n"
//                    "  </form>\n"
//                    "  \n"
//                    "  <script>\n"
//                    "    document.querySelector('form').addEventListener('submit', function(event) {\n"
//                    "      event.preventDefault(); // 阻止表单默认提交行为\n"
//                    "      var fileInput = document.getElementById('fileToUpload');\n"
//                    "      var file = fileInput.files[0];\n"
//                    "      const reader = new FileReader();\n"
//                    "      reader.onload = function() { const fileData = reader.result; alarm(\"upload done\"};\n"
//                    "reader.readAsDataURL(file)';\n"
//                    "      console.log(file)\n"
//                    "      console.log(fileInput)\n"
//                    "      \n"
//                    "      if (file) {\n"
//                    "        var xhr = new XMLHttpRequest();\n"
//                    "        xhr.open('POST', /filesSharing, true); // 替换为你的服务器端上传接口地址\n"
//                    "        xhr.onload = function() {\n"
//                    "        console.log(window.location.href);\n"
//                    "          if (xhr.status === 200) {\n"
//                    "            // 文件上传成功\n"
//                    "            console.log('文件上传成功！');\n"
//                    "          } else {\n"
//                    "            // 文件上传失败\n"
//                    "            console.log('文件上传失败。');\n"
//                    "          }\n"
//                    "        };\n"
//                    "        xhr.send(fileInput);\n"
//                    "      }\n"
//                    "    });\n"
//                    "  </script>\n"
//                    "<pre>"
//                    "\ncurrent files list is: \n"
//                    "----------------------------------------\n"
//                    + response_body
//                    +
//                    "</pre>\n"
//                    "</body>\n"
//                    "</html>\n";
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "  <head>\n"
                    "    <meta charset=\"UTF-8\">\n"
                    "    <title>文件上传示例</title>\n"
                    "  </head>\n"
                    "  <body>\n"
                    "    <h1>文件上传示例</h1>\n"
                    "    <input type=\"file\" id=\"fileInput\">\n"
                    "    <button id=\"uploadButton\" onclick=\"upload()\">上传文件</button>\n"
                    "<pre>"
                    "\ncurrent files list is: \n"
                    "----------------------------------------\n"
                    + response_body
                    +
                    "</pre>\n"
                    "    <script>\n"
                    "      function upload() {\n"
                    "        const fileInput = document.getElementById('fileInput');\n"
                    "        const file = fileInput.files[0];\n"
                    "        if (!file) {\n"
                    "          alert('请选择要上传的文件！');\n"
                    "          return;\n"
                    "        }\n"
                    "\n"
                    "        // 禁用按钮\n"
                    "        const uploadButton = document.getElementById('uploadButton');\n"
                    "        uploadButton.disabled = true;\n"
                    "\n"
                    "        // 构造请求对象\n"
                    "        const xhr = new XMLHttpRequest();\n"
                    "        xhr.open('POST', '/filesSharing');\n"
                    "        xhr.setRequestHeader('Content-Type', file.type); // 设置请求头，发送文件类型\n"
                    "        xhr.onreadystatechange = function() {\n"
                    "          if (xhr.readyState === 4) {\n"
                    "            // 上传完成，启用按钮\n"
                    "            uploadButton.disabled = false;\n"
                    "            alert('文件上传成功！');\n"
                    "            window.location.reload();\n"
                    "          }\n"
                    "        };\n"
                    "\n"
                    "        // 发送请求\n"
                    "        xhr.setRequestHeader('Content-Disposition', 'filename=' + file.name);\n"
                    "        xhr.send(file);\n"
                    "        \n"
                    "        // 提示用户等待上传完成\n"
                    "        alert('正在上传文件，请勿操作！');\n"
                    "      }\n"
                    "    </script>\n"
                    "  </body>\n"
                    "</html>";
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
        auto pos = request.headers.find("Content-Length: ") + 16;

        // pinrt headers
//        std::cout << "****headers: " << request.headers << "***" << std::endl;
        auto content_length = request.headers.substr(pos, request.body.find("\r\n", pos) - pos);
        // print content_length
        std::cout << "content_length: " << content_length << std::endl;

        // find string "filename=" in request.body from pos.
        pos = request.headers.find("filename=") + 9;
        std::string file_name = request.headers.substr(pos, request.headers.find("\r\n", pos) - pos);

        //print file_name
        std::cout << "***file_name: " << file_name << "***" << std::endl;

        std::ofstream OsWrite("./filesSharing/" + file_name, std::ofstream::trunc);
        OsWrite << request.body;
        OsWrite << std::endl;
        return {response_body, response_status};
    }

}

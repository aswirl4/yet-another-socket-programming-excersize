#ifndef SERVER_H
#define SERVER_H

#include "Config.h"
#include "ChatRoom.h"
#include "Request.h"
#include "Response.h"
#include <map>
#include <string>

class Server {
public:
    explicit Server(const Config& config);
    int server_sockfd;

    // Assuming that Server has a constructor that initializes chat_room

    Response handle_chat_request(const Request& request);
    static Response handle_file_request(const Request& request);

    [[noreturn]] void start();
    // make a close function
    void close(int sig);

private:
    ChatRoom chat_room;
    void handle_request(int client_sockfd);
    static int create_server(int port);

    Config config;
    std::map<std::string, ChatRoom> chat_rooms;
    static std::string file_dir;
};

#endif // SERVER_H

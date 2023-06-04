#ifndef CHATROOM_H
#define CHATROOM_H

#include <queue>
#include <string>

struct Message {
    std::string content;
    int timestamp;
};

class ChatRoom {
public:
    ChatRoom(int message_lifetime, int message_rate_limit);

    void add_message(const std::string& content, int timestamp);
    std::string get_chat_history();

private:
    void clean_up();

    std::queue<Message> messages;
    int message_lifetime;
    int message_rate_limit;
};

#endif // CHATROOM_H

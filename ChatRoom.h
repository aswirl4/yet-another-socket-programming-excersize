#ifndef CHATROOM_H
#define CHATROOM_H

#include <queue>
#include <string>

struct Message {
    std::string content;
//    time_t timestamp;
};

class ChatRoom {
public:
    ChatRoom();
    ChatRoom(int message_lifetime, int message_rate_limit);

    bool add_message(const std::string& content);
    std::string get_chat_history();

private:
    void clean_up();

    std::queue<Message> messages;
    int message_lifetime;
    int message_rate_limit;
};

#endif // CHATROOM_H

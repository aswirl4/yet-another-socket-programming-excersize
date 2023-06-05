#include "ChatRoom.h"
#include <ctime>

ChatRoom::ChatRoom() {}

ChatRoom::ChatRoom(int message_lifetime, int message_rate_limit)
        : message_lifetime(message_lifetime), message_rate_limit(message_rate_limit) {}

bool ChatRoom::add_message(const std::string &content) {
    if (!content.empty()) {
        messages.push({content});
    }
//    clean_up();
    return true;
}

std::string ChatRoom::get_chat_history() {
//    clean_up();

    std::string chat_history;
    std::queue<Message> temp_messages = messages;
    while (!temp_messages.empty()) {
        chat_history += temp_messages.front().content + "\n";
        temp_messages.pop();
    }

    return chat_history;
}

void ChatRoom::clean_up() {
    time_t current_time = std::time(nullptr);
//    while (!messages.empty() && current_time - messages.front().timestamp > message_lifetime) {
//        messages.pop();
//    }
}



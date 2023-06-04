#ifndef NETASSIGNMENT_CONFIG_H
#define NETASSIGNMENT_CONFIG_H
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    int port;
    int message_lifetime;
    int message_rate_limit;

    Config(const std::string& config_filename);
};

#endif // CONFIG_H

#endif //NETASSIGNMENT_CONFIG_H

#include "Config.h"
#include <fstream>

Config::Config(const std::string& config_filename) {
    std::ifstream file(config_filename);

    file >> port >> message_lifetime >> message_rate_limit;
}

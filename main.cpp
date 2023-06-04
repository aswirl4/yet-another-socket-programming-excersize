#include "Server.h"
#include "Config.h"

int main() {
    Config config("config.txt");
    Server server(config);
    server.start();

    return 0;
}

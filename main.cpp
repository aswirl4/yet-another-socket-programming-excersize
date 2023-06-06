#include "Server.h"
#include "Config.h"
#include <signal.h>
#include <unistd.h>



int main() {
    Config config("config.txt");
    Server server(config);
    server.start();
    return 0;
}

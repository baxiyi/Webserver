#include <unistd.h>
#include "webserver.h"

int main() {
    WebServer server(1300, 3, 60000, false, 20);
    server.Start();
}

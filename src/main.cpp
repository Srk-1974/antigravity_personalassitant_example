#include "ConfigManager.h"
#include "MemoryManager.h"
#include "Agent.h"
#include "Server.h"
#include <iostream>

int main() {
    std::cout << "Initializing Personal AI Agent..." << std::endl;

    ConfigManager config_mgr("config.json", ".env");
    MemoryManager memory_mgr("conversations.json");
    Agent agent(config_mgr, memory_mgr);

    // Ensure web UI directory exists or run from correct path
    // We assume the binary runs from the project root where "web" folder exists.
    Server server(8080, "./web", agent, config_mgr, memory_mgr);

    server.start();

    return 0;
}

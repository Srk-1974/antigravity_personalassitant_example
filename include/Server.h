#ifndef SERVER_H
#define SERVER_H

#include "httplib.h"
#include "Agent.h"
#include "ConfigManager.h"
#include "MemoryManager.h"
#include <string>

class Server {
public:
    Server(int port, const std::string& web_dir, Agent& agent, ConfigManager& config_mgr, MemoryManager& memory_mgr);

    void start();
    void stop();

private:
    void setupRoutes();

    int port_;
    std::string web_dir_;
    Agent& agent_;
    ConfigManager& config_mgr_;
    MemoryManager& memory_mgr_;
    httplib::Server svr_;
};

#endif // SERVER_H

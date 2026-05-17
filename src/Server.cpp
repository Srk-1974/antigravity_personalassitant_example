#include "Server.h"
#include "nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

Server::Server(int port, const std::string& web_dir, Agent& agent, ConfigManager& config_mgr, MemoryManager& memory_mgr)
    : port_(port), web_dir_(web_dir), agent_(agent), config_mgr_(config_mgr), memory_mgr_(memory_mgr) {
    setupRoutes();
}

void Server::setupRoutes() {
    // Serve static UI files
    svr_.set_mount_point("/", web_dir_);

    // API: Get App Config
    svr_.Get("/api/config", [this](const httplib::Request&, httplib::Response& res) {
        auto& config = config_mgr_.getConfig();
        json j;
        j["active_provider"] = config.active_provider;
        j["api_key"] = config.api_key;
        j["base_url"] = config.base_url;
        j["model_name"] = config.model_name;
        j["temperature"] = config.temperature;
        j["max_tokens"] = config.max_tokens;
        j["system_prompt"] = config.system_prompt;
        j["agent_mode"] = config.agent_mode;
        j["reasoning_mode"] = config.reasoning_mode;
        
        json providers = json::array();
        for (const auto& p : config.providers) {
            providers.push_back({{"name", p.name}, {"base_url", p.base_url}, {"default_model", p.default_model}});
        }
        j["providers"] = providers;

        res.set_content(j.dump(), "application/json");
    });

    // API: Update App Config
    svr_.Post("/api/config", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json j = json::parse(req.body);
            auto& config = config_mgr_.getConfig();
            
            if (j.contains("active_provider")) {
                std::string prov = j["active_provider"];
                if (prov != config.active_provider) {
                    config_mgr_.switchProvider(prov);
                }
            }
            if (j.contains("api_key")) config.api_key = j["api_key"];
            if (j.contains("base_url")) config.base_url = j["base_url"];
            if (j.contains("model_name")) config.model_name = j["model_name"];
            if (j.contains("temperature")) config.temperature = j["temperature"];
            if (j.contains("max_tokens")) config.max_tokens = j["max_tokens"];
            if (j.contains("system_prompt")) config.system_prompt = j["system_prompt"];
            if (j.contains("agent_mode")) config.agent_mode = j["agent_mode"];
            if (j.contains("reasoning_mode")) config.reasoning_mode = j["reasoning_mode"];

            config_mgr_.setConfig(config);
            res.set_content("{\"status\":\"success\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        }
    });

    // API: Get Conversations list
    svr_.Get("/api/conversations", [this](const httplib::Request&, httplib::Response& res) {
        json j = json::array();
        for (const auto& conv : memory_mgr_.getAllConversations()) {
            j.push_back({{"id", conv.id}, {"title", conv.title}, {"created_at", conv.created_at}});
        }
        res.set_content(j.dump(), "application/json");
    });

    // API: Create Conversation
    svr_.Post("/api/conversations", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json j = json::parse(req.body);
            std::string title = j.value("title", "New Chat");
            std::string id = memory_mgr_.createConversation(title);
            res.set_content("{\"id\":\"" + id + "\"}", "application/json");
        } catch (...) {
            res.status = 400;
        }
    });

    // API: Get Conversation History
    svr_.Get(R"(/api/conversations/(.*?)/messages)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string conv_id = req.matches[1];
        auto history = memory_mgr_.getConversationHistory(conv_id);
        json j = json::array();
        for (const auto& msg : history) {
            j.push_back({{"role", msg.role}, {"content", msg.content}, {"timestamp", msg.timestamp}});
        }
        res.set_content(j.dump(), "application/json");
    });

    // API: Chat Generate Response
    svr_.Post("/api/chat", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json j = json::parse(req.body);
            std::string conv_id = j.value("conv_id", "");
            std::string message = j.value("message", "");

            if (conv_id.empty() || message.empty()) {
                res.status = 400;
                res.set_content("{\"error\":\"conv_id and message are required\"}", "application/json");
                return;
            }

            std::string reply = agent_.processMessage(conv_id, message);
            
            json res_j;
            res_j["reply"] = reply;
            res.set_content(res_j.dump(), "application/json");
        } catch (...) {
            res.status = 400;
        }
    });
}

void Server::start() {
    std::cout << "Starting Personal AI Agent Server on http://localhost:" << port_ << std::endl;
    svr_.listen("0.0.0.0", port_);
}

void Server::stop() {
    svr_.stop();
}

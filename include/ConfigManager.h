#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"

struct ProviderConfig {
    std::string name;
    std::string base_url;
    std::string api_key;
    std::string default_model;
};

struct AppConfig {
    std::string active_provider;
    std::string api_key;
    std::string base_url;
    std::string model_name;
    double temperature = 0.7;
    int max_tokens = 2048;
    std::string system_prompt = "You are a helpful, expert Personal AI Agent.";
    std::string agent_mode = "Coding Assistant"; // Coding, Research, DevOps, CyberSec
    bool reasoning_mode = true; // Think before answering
    std::vector<ProviderConfig> providers;
};

class ConfigManager {
public:
    ConfigManager(const std::string& config_path = "config.json", const std::string& env_path = ".env");

    bool loadConfig();
    bool saveConfig();
    bool loadEnv();

    AppConfig& getConfig();
    void setConfig(const AppConfig& new_config);

    // Helper to switch provider preset
    bool switchProvider(const std::string& provider_name);

private:
    std::string config_path_;
    std::string env_path_;
    AppConfig config_;

    void createDefaultConfig();
};

#endif // CONFIG_MANAGER_H

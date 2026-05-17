#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

ConfigManager::ConfigManager(const std::string& config_path, const std::string& env_path)
    : config_path_(config_path), env_path_(env_path) {
    createDefaultConfig();
    loadEnv();
    loadConfig();
}

void ConfigManager::createDefaultConfig() {
    config_.active_provider = "Groq";
    config_.api_key = "";
    config_.base_url = "https://api.groq.com/openai/v1";
    config_.model_name = "llama3-8b-8192";
    config_.temperature = 0.7;
    config_.max_tokens = 2048;
    config_.system_prompt = "You are a helpful, expert Personal AI Agent.";
    config_.agent_mode = "Coding Assistant";
    config_.reasoning_mode = true;

    config_.providers = {
        {"Groq", "https://api.groq.com/openai/v1", "", "llama3-8b-8192"},
        {"OpenAI", "https://api.openai.com/v1", "", "gpt-4o-mini"},
        {"Ollama", "http://localhost:11434/v1", "ollama", "llama3:latest"},
        {"Gemini", "https://generativelanguage.googleapis.com/v1beta/openai", "", "gemini-1.5-flash"}
    };
}

bool ConfigManager::loadEnv() {
    std::ifstream file(env_path_);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if (key == "GROQ_API_KEY") {
                for (auto& p : config_.providers) if (p.name == "Groq") p.api_key = value;
                if (config_.active_provider == "Groq") config_.api_key = value;
            } else if (key == "OPENAI_API_KEY") {
                for (auto& p : config_.providers) if (p.name == "OpenAI") p.api_key = value;
                if (config_.active_provider == "OpenAI") config_.api_key = value;
            } else if (key == "GEMINI_API_KEY") {
                for (auto& p : config_.providers) if (p.name == "Gemini") p.api_key = value;
                if (config_.active_provider == "Gemini") config_.api_key = value;
            }
        }
    }
    file.close();
    return true;
}

bool ConfigManager::loadConfig() {
    std::ifstream file(config_path_);
    if (!file.is_open()) {
        saveConfig(); // Create default file
        return true;
    }

    try {
        json j;
        file >> j;

        if (j.contains("active_provider")) config_.active_provider = j["active_provider"];
        if (j.contains("api_key")) config_.api_key = j["api_key"];
        if (j.contains("base_url")) config_.base_url = j["base_url"];
        if (j.contains("model_name")) config_.model_name = j["model_name"];
        if (j.contains("temperature")) config_.temperature = j["temperature"];
        if (j.contains("max_tokens")) config_.max_tokens = j["max_tokens"];
        if (j.contains("system_prompt")) config_.system_prompt = j["system_prompt"];
        if (j.contains("agent_mode")) config_.agent_mode = j["agent_mode"];
        if (j.contains("reasoning_mode")) config_.reasoning_mode = j["reasoning_mode"];

        if (j.contains("providers")) {
            config_.providers.clear();
            for (const auto& item : j["providers"]) {
                ProviderConfig pc;
                pc.name = item.value("name", "");
                pc.base_url = item.value("base_url", "");
                pc.api_key = item.value("api_key", "");
                pc.default_model = item.value("default_model", "");
                config_.providers.push_back(pc);
            }
        }
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config.json: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool ConfigManager::saveConfig() {
    try {
        json j;
        j["active_provider"] = config_.active_provider;
        j["api_key"] = config_.api_key;
        j["base_url"] = config_.base_url;
        j["model_name"] = config_.model_name;
        j["temperature"] = config_.temperature;
        j["max_tokens"] = config_.max_tokens;
        j["system_prompt"] = config_.system_prompt;
        j["agent_mode"] = config_.agent_mode;
        j["reasoning_mode"] = config_.reasoning_mode;

        json prov_arr = json::array();
        for (const auto& p : config_.providers) {
            prov_arr.push_back({
                {"name", p.name},
                {"base_url", p.base_url},
                {"api_key", p.api_key},
                {"default_model", p.default_model}
            });
        }
        j["providers"] = prov_arr;

        std::ofstream file(config_path_);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving config.json: " << e.what() << std::endl;
    }
    return false;
}

AppConfig& ConfigManager::getConfig() {
    return config_;
}

void ConfigManager::setConfig(const AppConfig& new_config) {
    config_ = new_config;
    saveConfig();
}

bool ConfigManager::switchProvider(const std::string& provider_name) {
    for (const auto& p : config_.providers) {
        if (p.name == provider_name) {
            config_.active_provider = p.name;
            config_.base_url = p.base_url;
            config_.api_key = p.api_key;
            config_.model_name = p.default_model;
            saveConfig();
            return true;
        }
    }
    return false;
}

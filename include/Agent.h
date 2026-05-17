#ifndef AGENT_H
#define AGENT_H

#include "MemoryManager.h"
#include "ConfigManager.h"
#include "LLMProvider.h"
#include <memory>
#include <string>

class Agent {
public:
    Agent(ConfigManager& config_mgr, MemoryManager& memory_mgr);

    // Main entry point for chatting
    std::string processMessage(const std::string& conv_id, const std::string& user_input);

    // Mode management
    void setMode(const std::string& mode);
    void toggleReasoning(bool enabled);

private:
    ConfigManager& config_mgr_;
    MemoryManager& memory_mgr_;
    std::unique_ptr<LLMProvider> provider_;

    // Handle tool calling and internal reasoning (simulated for simplicity)
    std::string internalReasoning(const std::string& user_input);
};

#endif // AGENT_H

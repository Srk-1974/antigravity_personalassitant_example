#include "Agent.h"
#include <iostream>

Agent::Agent(ConfigManager& config_mgr, MemoryManager& memory_mgr)
    : config_mgr_(config_mgr), memory_mgr_(memory_mgr) {
    provider_ = std::make_unique<UniversalOpenAIProvider>();
}

std::string Agent::internalReasoning(const std::string& user_input) {
    // In a fully developed agent, this would do a multi-step chain-of-thought
    // For now, we simulate the internal reasoning log.
    std::cout << "[Agent Thinking...] Analyzing request in " << config_mgr_.getConfig().agent_mode << " mode." << std::endl;
    return "[Reasoning complete]";
}

std::string Agent::processMessage(const std::string& conv_id, const std::string& user_input) {
    // 1. Save User Message
    memory_mgr_.addMessage(conv_id, "user", user_input);

    // 2. Load Short-term memory context
    std::vector<ChatMessage> context = memory_mgr_.getShortTermMemory(conv_id, 20);

    // 3. Optional Reasoning
    if (config_mgr_.getConfig().reasoning_mode) {
        internalReasoning(user_input);
    }

    // 4. Generate Response using Provider
    LLMResponse response = provider_->generateResponse(context, config_mgr_.getConfig());

    std::string final_reply = response.success ? response.content : ("[Error] " + response.error_message);

    // 5. Save Assistant Message
    memory_mgr_.addMessage(conv_id, "assistant", final_reply);

    return final_reply;
}

void Agent::setMode(const std::string& mode) {
    AppConfig cfg = config_mgr_.getConfig();
    cfg.agent_mode = mode;
    config_mgr_.setConfig(cfg);
}

void Agent::toggleReasoning(bool enabled) {
    AppConfig cfg = config_mgr_.getConfig();
    cfg.reasoning_mode = enabled;
    config_mgr_.setConfig(cfg);
}

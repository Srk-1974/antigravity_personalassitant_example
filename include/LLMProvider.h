#ifndef LLMPROVIDER_H
#define LLMPROVIDER_H

#include <string>
#include <vector>
#include "MemoryManager.h"
#include "ConfigManager.h"
#include "nlohmann/json.hpp"

struct LLMResponse {
    std::string content;
    bool success;
    std::string error_message;
};

class LLMProvider {
public:
    virtual ~LLMProvider() = default;
    
    // Abstract method to generate a response given a conversation history and config
    virtual LLMResponse generateResponse(const std::vector<ChatMessage>& history, const AppConfig& config) = 0;
};

class UniversalOpenAIProvider : public LLMProvider {
public:
    LLMResponse generateResponse(const std::vector<ChatMessage>& history, const AppConfig& config) override;

private:
    std::string buildRequestBody(const std::vector<ChatMessage>& history, const AppConfig& config);
    std::string formatUrl(const std::string& base_url);
};

#endif // LLMPROVIDER_H

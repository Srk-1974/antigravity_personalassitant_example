#include "LLMProvider.h"
#include "WinHttpClient.h"
#include <iostream>

using json = nlohmann::json;

std::string UniversalOpenAIProvider::formatUrl(const std::string& base_url) {
    std::string url = base_url;
    if (url.back() == '/') {
        url.pop_back();
    }
    // Automatically append /chat/completions if not present
    if (url.find("/chat/completions") == std::string::npos) {
        url += "/chat/completions";
    }
    return url;
}

std::string UniversalOpenAIProvider::buildRequestBody(const std::vector<ChatMessage>& history, const AppConfig& config) {
    json j;
    j["model"] = config.model_name;
    j["temperature"] = config.temperature;
    j["max_tokens"] = config.max_tokens;
    
    json messages = json::array();
    
    // Add system prompt first
    messages.push_back({
        {"role", "system"},
        {"content", config.system_prompt}
    });

    // Inject Agent Mode / Reasoning Mode directives
    if (config.reasoning_mode) {
        std::string reasoning_prompt = "Before answering, think internally step-by-step. "
            "You are acting in mode: " + config.agent_mode + ". "
            "Ensure your response is highly professional and analytical.";
        messages.push_back({
            {"role", "system"},
            {"content", reasoning_prompt}
        });
    }

    // Add conversation history
    for (const auto& msg : history) {
        messages.push_back({
            {"role", msg.role},
            {"content", msg.content}
        });
    }
    
    j["messages"] = messages;
    return j.dump();
}

LLMResponse UniversalOpenAIProvider::generateResponse(const std::vector<ChatMessage>& history, const AppConfig& config) {
    LLMResponse res;
    res.success = false;

    std::string endpoint = formatUrl(config.base_url);
    std::string body = buildRequestBody(history, config);

    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    
    // Some local providers like Ollama don't strictly require API keys, but OpenAI/Groq do
    if (!config.api_key.empty()) {
        headers["Authorization"] = "Bearer " + config.api_key;
    }

    HttpResponse http_res = WinHttpClient::Post(endpoint, headers, body);

    if (http_res.status_code == 200) {
        try {
            json response_json = json::parse(http_res.body);
            if (response_json.contains("choices") && !response_json["choices"].empty()) {
                res.content = response_json["choices"][0]["message"]["content"];
                res.success = true;
            } else {
                res.error_message = "Unexpected API response format.";
            }
        } catch (const std::exception& e) {
            res.error_message = "JSON Parse Error: " + std::string(e.what());
        }
    } else {
        res.error_message = "HTTP Error " + std::to_string(http_res.status_code) + " - " + http_res.error_message + "\n" + http_res.body;
    }

    return res;
}

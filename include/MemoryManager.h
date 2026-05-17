#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"

struct ChatMessage {
    std::string role; // "system", "user", "assistant"
    std::string content;
    std::string timestamp;
};

struct Conversation {
    std::string id;
    std::string title;
    std::string created_at;
    std::vector<ChatMessage> messages;
};

class MemoryManager {
public:
    MemoryManager(const std::string& storage_path = "conversations.json");

    bool loadMemory();
    bool saveMemory();

    // Conversation management
    std::vector<Conversation> getAllConversations() const;
    Conversation getConversation(const std::string& conv_id) const;
    std::string createConversation(const std::string& title);
    bool deleteConversation(const std::string& conv_id);

    // Message management
    bool addMessage(const std::string& conv_id, const std::string& role, const std::string& content);
    std::vector<ChatMessage> getConversationHistory(const std::string& conv_id) const;

    // Short-term memory (gets last N messages suitable for LLM context window)
    std::vector<ChatMessage> getShortTermMemory(const std::string& conv_id, int max_messages = 10) const;

    // Long-term memory summary helper
    std::string getLongTermSummary(const std::string& conv_id) const;

private:
    std::string storage_path_;
    std::vector<Conversation> conversations_;

    std::string generateUUID() const;
    std::string getCurrentTimestamp() const;
};

#endif // MEMORY_MANAGER_H

#include "MemoryManager.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

using json = nlohmann::json;

MemoryManager::MemoryManager(const std::string& storage_path)
    : storage_path_(storage_path) {
    loadMemory();
    if (conversations_.empty()) {
        createConversation("Default Conversation");
    }
}

std::string MemoryManager::generateUUID() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << dis(gen);
    return ss.str();
}

std::string MemoryManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool MemoryManager::loadMemory() {
    std::ifstream file(storage_path_);
    if (!file.is_open()) {
        return false;
    }

    try {
        json j;
        file >> j;
        conversations_.clear();

        for (const auto& item : j) {
            Conversation conv;
            conv.id = item.value("id", "");
            conv.title = item.value("title", "");
            conv.created_at = item.value("created_at", "");

            if (item.contains("messages")) {
                for (const auto& msg_item : item["messages"]) {
                    ChatMessage msg;
                    msg.role = msg_item.value("role", "");
                    msg.content = msg_item.value("content", "");
                    msg.timestamp = msg_item.value("timestamp", "");
                    conv.messages.push_back(msg);
                }
            }
            conversations_.push_back(conv);
        }
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing conversations.json: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool MemoryManager::saveMemory() {
    try {
        json j = json::array();
        for (const auto& conv : conversations_) {
            json conv_j;
            conv_j["id"] = conv.id;
            conv_j["title"] = conv.title;
            conv_j["created_at"] = conv.created_at;

            json msg_arr = json::array();
            for (const auto& msg : conv.messages) {
                msg_arr.push_back({
                    {"role", msg.role},
                    {"content", msg.content},
                    {"timestamp", msg.timestamp}
                });
            }
            conv_j["messages"] = msg_arr;
            j.push_back(conv_j);
        }

        std::ofstream file(storage_path_);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving conversations.json: " << e.what() << std::endl;
    }
    return false;
}

std::vector<Conversation> MemoryManager::getAllConversations() const {
    return conversations_;
}

Conversation MemoryManager::getConversation(const std::string& conv_id) const {
    for (const auto& conv : conversations_) {
        if (conv.id == conv_id) {
            return conv;
        }
    }
    return Conversation{};
}

std::string MemoryManager::createConversation(const std::string& title) {
    Conversation conv;
    conv.id = generateUUID();
    conv.title = title.empty() ? "New Conversation" : title;
    conv.created_at = getCurrentTimestamp();
    conversations_.push_back(conv);
    saveMemory();
    return conv.id;
}

bool MemoryManager::deleteConversation(const std::string& conv_id) {
    for (auto it = conversations_.begin(); it != conversations_.end(); ++it) {
        if (it->id == conv_id) {
            conversations_.erase(it);
            saveMemory();
            return true;
        }
    }
    return false;
}

bool MemoryManager::addMessage(const std::string& conv_id, const std::string& role, const std::string& content) {
    for (auto& conv : conversations_) {
        if (conv.id == conv_id) {
            ChatMessage msg;
            msg.role = role;
            msg.content = content;
            msg.timestamp = getCurrentTimestamp();
            conv.messages.push_back(msg);
            saveMemory();
            return true;
        }
    }
    return false;
}

std::vector<ChatMessage> MemoryManager::getConversationHistory(const std::string& conv_id) const {
    for (const auto& conv : conversations_) {
        if (conv.id == conv_id) {
            return conv.messages;
        }
    }
    return {};
}

std::vector<ChatMessage> MemoryManager::getShortTermMemory(const std::string& conv_id, int max_messages) const {
    auto history = getConversationHistory(conv_id);
    if (history.size() <= static_cast<size_t>(max_messages)) {
        return history;
    }
    return std::vector<ChatMessage>(history.end() - max_messages, history.end());
}

std::string MemoryManager::getLongTermSummary(const std::string& conv_id) const {
    auto history = getConversationHistory(conv_id);
    if (history.empty()) return "No conversation history.";
    std::stringstream ss;
    ss << "Conversation covers " << history.size() << " turns. Active topics include Personal AI agent interactions.";
    return ss.str();
}

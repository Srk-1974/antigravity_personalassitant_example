import os
import json
import uuid
from datetime import datetime
from flask import Flask, request, jsonify, send_from_directory
import requests
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__, static_folder='web')

CONFIG_FILE = 'config.json'
MEMORY_FILE = 'conversations.json'

def load_json(filepath, default_val):
    if os.path.exists(filepath):
        try:
            with open(filepath, 'r') as f:
                return json.load(f)
        except:
            pass
    return default_val

def save_json(filepath, data):
    with open(filepath, 'w') as f:
        json.dump(data, f, indent=4)

# --- Default Config ---
default_config = {
    "active_provider": "Groq",
    "api_key": os.getenv("GROQ_API_KEY", ""),
    "base_url": "https://api.groq.com/openai/v1",
    "model_name": "llama3-8b-8192",
    "temperature": 0.7,
    "max_tokens": 2048,
    "system_prompt": "You are a helpful, expert Personal AI Agent.",
    "agent_mode": "Coding Assistant",
    "reasoning_mode": True,
    "providers": [
        {"name": "Groq", "base_url": "https://api.groq.com/openai/v1", "default_model": "llama3-8b-8192", "env_key": "GROQ_API_KEY"},
        {"name": "OpenAI", "base_url": "https://api.openai.com/v1", "default_model": "gpt-4o-mini", "env_key": "OPENAI_API_KEY"},
        {"name": "Ollama", "base_url": "http://localhost:11434/v1", "default_model": "llama3:latest", "env_key": ""},
        {"name": "Gemini", "base_url": "https://generativelanguage.googleapis.com/v1beta/openai", "default_model": "gemini-1.5-flash", "env_key": "GEMINI_API_KEY"}
    ]
}

# --- Load Config and Memory ---
app_config = load_json(CONFIG_FILE, default_config)
conversations = load_json(MEMORY_FILE, [])

# Ensure config matches defaults structurally
for k, v in default_config.items():
    if k not in app_config:
        app_config[k] = v

# Provide proper API keys if available in env but empty in config
for p in app_config.get("providers", []):
    env_k = p.get("env_key")
    if env_k and os.getenv(env_k) and not app_config.get("api_key"):
        if app_config["active_provider"] == p["name"]:
            app_config["api_key"] = os.getenv(env_k)

@app.route('/')
def index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/<path:path>')
def static_files(path):
    return send_from_directory(app.static_folder, path)

@app.route('/api/config', methods=['GET'])
def get_config():
    return jsonify(app_config)

@app.route('/api/config', methods=['POST'])
def set_config():
    global app_config
    data = request.json
    for key in ["active_provider", "api_key", "base_url", "model_name", "temperature", "max_tokens", "agent_mode", "reasoning_mode", "system_prompt"]:
        if key in data:
            app_config[key] = data[key]
    save_json(CONFIG_FILE, app_config)
    return jsonify({"status": "success"})

@app.route('/api/conversations', methods=['GET'])
def get_conversations():
    return jsonify([{"id": c["id"], "title": c["title"], "created_at": c["created_at"]} for c in conversations])

@app.route('/api/conversations', methods=['POST'])
def create_conversation():
    data = request.json
    new_id = str(uuid.uuid4())
    conv = {
        "id": new_id,
        "title": data.get("title", "New Conversation"),
        "created_at": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "messages": []
    }
    conversations.append(conv)
    save_json(MEMORY_FILE, conversations)
    return jsonify({"id": new_id})

@app.route('/api/conversations/<conv_id>/messages', methods=['GET'])
def get_messages(conv_id):
    for c in conversations:
        if c["id"] == conv_id:
            return jsonify(c["messages"])
    return jsonify([])

@app.route('/api/chat', methods=['POST'])
def chat():
    data = request.json
    conv_id = data.get("conv_id")
    message = data.get("message")
    
    conv = next((c for c in conversations if c["id"] == conv_id), None)
    if not conv:
        return jsonify({"error": "Conversation not found"}), 404

    # Add user message
    conv["messages"].append({
        "role": "user",
        "content": message,
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    })

    # Build context
    messages_for_llm = [{"role": "system", "content": app_config["system_prompt"]}]
    if app_config["reasoning_mode"]:
        messages_for_llm.append({"role": "system", "content": f"Think before answering. Act as a {app_config['agent_mode']}."})
    
    for m in conv["messages"][-20:]: # Last 20 messages short term memory
        messages_for_llm.append({"role": m["role"], "content": m["content"]})

    payload = {
        "model": app_config["model_name"],
        "temperature": app_config["temperature"],
        "max_tokens": app_config["max_tokens"],
        "messages": messages_for_llm
    }

    headers = {"Content-Type": "application/json"}
    if app_config.get("api_key"):
        headers["Authorization"] = f"Bearer {app_config['api_key']}"

    base_url = app_config["base_url"]
    if not base_url.endswith("/chat/completions"):
        base_url = base_url.rstrip("/") + "/chat/completions"

    reply_content = ""
    try:
        response = requests.post(base_url, json=payload, headers=headers)
        if response.status_code == 200:
            res_data = response.json()
            if "choices" in res_data and len(res_data["choices"]) > 0:
                reply_content = res_data["choices"][0]["message"]["content"]
            else:
                reply_content = "Error: Unexpected API response format."
        else:
            reply_content = f"API Error {response.status_code}: {response.text}"
    except Exception as e:
        reply_content = f"Network Error: {str(e)}"

    # Add assistant message
    conv["messages"].append({
        "role": "assistant",
        "content": reply_content,
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    })
    save_json(MEMORY_FILE, conversations)

    return jsonify({"reply": reply_content})

if __name__ == '__main__':
    print("Starting Nexus AI Backend on http://localhost:8080")
    app.run(host='0.0.0.0', port=8080, debug=False)

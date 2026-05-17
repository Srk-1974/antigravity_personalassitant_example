# Personal AI Agent (C++)

A highly modular, production-ready, zero-dependency C++ Personal AI Agent.

## Features
- **Dynamic Multiple LLM Providers:** Works out of the box with Groq, OpenAI, Ollama, Gemini, and any OpenAI-compatible API.
- **Ultra-Fast C++ Backend:** Zero Python or Node.js dependencies. Powered by `cpp-httplib` and native Windows WinHTTP.
- **Modern Web GUI:** A stunning, glassmorphic UI built with HTML/CSS/Vanilla JS that connects seamlessly to the C++ core.
- **Local Memory & Config:** Automatically saves conversations and settings locally in standard JSON format.
- **Agent Modes:** Switch between Coding, Research, DevOps, and Cybersecurity modes.
- **Internal Reasoning:** Toggleable internal reasoning simulation before answering.

## Architecture
- `ConfigManager`: Manages `.env` files and `config.json` dynamically.
- `MemoryManager`: Stores JSON-based short-term and long-term memory conversation history.
- `LLMProvider`: A universal API adapter for OpenAI schema endpoints.
- `WinHttpClient`: Robust native HTTPS requests on Windows without requiring OpenSSL compilation.
- `Server`: Lightweight REST/WebSocket server hosting the React-style Web UI.

## Setup Instructions
1. **Configure API Keys:**
   Copy `.env.example` to `.env` and fill in your API keys. Or configure them through the Web UI.
   
2. **Build the Application:**
   Run `build.bat`
   (Requires CMake and a C++ compiler like MSVC or MinGW installed on your system).

3. **Run the Application:**
   Run `run.bat`
   
4. **Access the UI:**
   Open your browser and navigate to `http://localhost:8080`.

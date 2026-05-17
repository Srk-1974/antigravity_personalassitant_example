document.addEventListener('DOMContentLoaded', () => {
    
    // --- UI State ---
    let currentConvId = null;
    let isGenerating = false;

    // --- DOM Elements ---
    const tabs = document.querySelectorAll('.tab-btn');
    const sections = document.querySelectorAll('.view-section');
    
    const chatWindow = document.getElementById('chat-window');
    const chatInput = document.getElementById('chat-input');
    const btnSend = document.getElementById('btn-send');
    const btnNewChat = document.getElementById('btn-new-chat');
    const historyList = document.getElementById('history-list');
    
    const currentChatTitle = document.getElementById('current-chat-title');
    const agentModeDisplay = document.getElementById('agent-mode-display');
    const statusDot = document.querySelector('.dot');

    // Config Elements
    const selProvider = document.getElementById('sel-provider');
    const inpBaseUrl = document.getElementById('inp-base-url');
    const inpApiKey = document.getElementById('inp-api-key');
    const inpModelName = document.getElementById('inp-model-name');
    const inpTemp = document.getElementById('inp-temp');
    const valTemp = document.getElementById('val-temp');
    const inpMaxTokens = document.getElementById('inp-max-tokens');
    const selAgentMode = document.getElementById('sel-agent-mode');
    const chkReasoning = document.getElementById('chk-reasoning');
    const inpSystemPrompt = document.getElementById('inp-system-prompt');
    const btnSaveConfig = document.getElementById('btn-save-config');
    const configStatus = document.getElementById('config-status');

    // --- Default Config ---
    const defaultConfig = {
        active_provider: "Groq",
        api_key: "",
        base_url: "https://api.groq.com/openai/v1",
        model_name: "llama3-8b-8192",
        temperature: 0.7,
        max_tokens: 2048,
        system_prompt: "You are a helpful, expert Personal AI Agent.",
        agent_mode: "Coding Assistant",
        reasoning_mode: true,
        providers: [
            {name: "Groq", base_url: "https://api.groq.com/openai/v1", default_model: "llama3-8b-8192"},
            {name: "OpenAI", base_url: "https://api.openai.com/v1", default_model: "gpt-4o-mini"},
            {name: "Ollama", base_url: "http://localhost:11434/v1", default_model: "llama3:latest"},
            {name: "Gemini", base_url: "https://generativelanguage.googleapis.com/v1beta/openai", default_model: "gemini-1.5-flash"}
        ]
    };

    let configData = JSON.parse(localStorage.getItem('nexus_config')) || defaultConfig;
    let conversations = JSON.parse(localStorage.getItem('nexus_conversations')) || [];

    function saveConfig() {
        localStorage.setItem('nexus_config', JSON.stringify(configData));
    }

    function saveConversations() {
        localStorage.setItem('nexus_conversations', JSON.stringify(conversations));
    }

    // --- Navigation ---
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            tabs.forEach(t => t.classList.remove('active'));
            sections.forEach(s => s.classList.remove('active'));
            tab.classList.add('active');
            document.getElementById(tab.dataset.target).classList.add('active');
        });
    });

    // --- Config Logic ---
    function loadConfigUI() {
        selProvider.value = configData.active_provider || "Groq";
        inpBaseUrl.value = configData.base_url || "";
        inpApiKey.value = configData.api_key || "";
        inpModelName.value = configData.model_name || "";
        inpTemp.value = configData.temperature || 0.7;
        valTemp.innerText = configData.temperature || 0.7;
        inpMaxTokens.value = configData.max_tokens || 2048;
        selAgentMode.value = configData.agent_mode || "Coding Assistant";
        chkReasoning.checked = configData.reasoning_mode !== false;
        inpSystemPrompt.value = configData.system_prompt || defaultConfig.system_prompt;
        
        agentModeDisplay.innerText = configData.agent_mode || "Coding Assistant";
    }

    inpTemp.addEventListener('input', (e) => {
        valTemp.innerText = e.target.value;
    });

    selProvider.addEventListener('change', (e) => {
        const provName = e.target.value;
        const prov = defaultConfig.providers.find(p => p.name === provName);
        if (prov) {
            inpBaseUrl.value = prov.base_url;
            inpModelName.value = prov.default_model;
        }
    });

    btnSaveConfig.addEventListener('click', () => {
        configData = {
            ...configData,
            active_provider: selProvider.value,
            base_url: inpBaseUrl.value,
            api_key: inpApiKey.value,
            model_name: inpModelName.value,
            temperature: parseFloat(inpTemp.value),
            max_tokens: parseInt(inpMaxTokens.value),
            agent_mode: selAgentMode.value,
            reasoning_mode: chkReasoning.checked,
            system_prompt: inpSystemPrompt.value
        };
        saveConfig();

        configStatus.innerText = "Configuration saved successfully!";
        agentModeDisplay.innerText = selAgentMode.value;
        setTimeout(() => configStatus.innerText = "", 3000);
    });

    // --- Chat Logic ---

    function loadConversationsList() {
        historyList.innerHTML = '';
        const reversed = [...conversations].reverse();
        reversed.forEach(c => {
            const div = document.createElement('div');
            div.className = 'history-item';
            if (c.id === currentConvId) div.classList.add('active');
            div.innerText = c.title;
            div.onclick = () => loadConversation(c.id);
            historyList.appendChild(div);
        });
    }

    function createNewChat() {
        const newId = crypto.randomUUID ? crypto.randomUUID() : Math.random().toString(36).substring(2);
        const conv = {
            id: newId,
            title: "New Conversation",
            created_at: new Date().toISOString(),
            messages: []
        };
        conversations.push(conv);
        saveConversations();
        
        currentConvId = newId;
        currentChatTitle.innerText = "New Conversation";
        chatWindow.innerHTML = '<div class="welcome-message"><h2>Nexus AI is ready.</h2><p>How can I assist you today?</p></div>';
        loadConversationsList();
    }

    function loadConversation(id) {
        currentConvId = id;
        const conv = conversations.find(c => c.id === id);
        if (!conv) return;

        currentChatTitle.innerText = conv.title;
        loadConversationsList(); // update active class

        chatWindow.innerHTML = '';
        if (conv.messages.length === 0) {
            chatWindow.innerHTML = '<div class="welcome-message"><h2>Nexus AI is ready.</h2><p>How can I assist you today?</p></div>';
        } else {
            conv.messages.forEach(m => appendMessage(m.role, m.content));
        }
    }

    function appendMessage(role, content) {
        const div = document.createElement('div');
        div.className = `message msg-${role === 'user' ? 'user' : 'agent'}`;
        
        // Very basic markdown parsing for code blocks
        let htmlContent = content
            .replace(/```([\s\S]*?)```/g, '<pre><code>$1</code></pre>')
            .replace(/`([^`]+)`/g, '<code>$1</code>')
            .replace(/\n/g, '<br>');

        div.innerHTML = htmlContent;
        chatWindow.appendChild(div);
        chatWindow.scrollTop = chatWindow.scrollHeight;
    }

    async function generateTitle(messages) {
        if (messages.length !== 1) return; // Only title on first message
        const conv = conversations.find(c => c.id === currentConvId);
        if (conv) {
            // Take the first 30 chars of the user message as a simple title
            let title = messages[0].content.substring(0, 30);
            if (messages[0].content.length > 30) title += "...";
            conv.title = title;
            currentChatTitle.innerText = title;
            saveConversations();
            loadConversationsList();
        }
    }

    async function sendMessage() {
        const text = chatInput.value.trim();
        if (!text || isGenerating || !currentConvId) return;

        const conv = conversations.find(c => c.id === currentConvId);
        if (!conv) return;

        chatInput.value = '';
        chatInput.style.height = 'auto';
        appendMessage('user', text);
        
        conv.messages.push({ role: "user", content: text, timestamp: new Date().toISOString() });
        saveConversations();
        generateTitle(conv.messages);

        isGenerating = true;
        statusDot.classList.add('pulse');
        agentModeDisplay.innerText = "Agent is thinking...";

        try {
            // Build Context
            let system_prompt = configData.system_prompt;
            if (configData.reasoning_mode) {
                system_prompt += `\nThink before answering. Act as a ${configData.agent_mode}.`;
            }

            const messagesForLLM = [{ role: "system", content: system_prompt }];
            
            // Last 20 messages short term memory
            const recentMessages = conv.messages.slice(-20);
            recentMessages.forEach(m => {
                messagesForLLM.push({ role: m.role, content: m.content });
            });

            let baseUrl = configData.base_url;
            if (!baseUrl.endsWith("/chat/completions")) {
                baseUrl = baseUrl.replace(/\/$/, "") + "/chat/completions";
            }

            const payload = {
                model: configData.model_name,
                temperature: configData.temperature,
                max_tokens: configData.max_tokens,
                messages: messagesForLLM
            };

            const headers = { "Content-Type": "application/json" };
            if (configData.api_key) {
                headers["Authorization"] = `Bearer ${configData.api_key}`;
            }

            const res = await fetch(baseUrl, {
                method: 'POST',
                headers: headers,
                body: JSON.stringify(payload)
            });

            if (res.ok) {
                const data = await res.json();
                const reply = data.choices && data.choices.length > 0 
                    ? data.choices[0].message.content 
                    : "Error: Unexpected API response format.";
                
                appendMessage('agent', reply);
                conv.messages.push({ role: "assistant", content: reply, timestamp: new Date().toISOString() });
                saveConversations();
            } else {
                const errText = await res.text();
                appendMessage('agent', `API Error ${res.status}: ${errText}`);
            }

        } catch (e) {
            appendMessage('agent', `Network Error: ${e.message}. Please check your Base URL and API Key in Configuration.`);
        } finally {
            isGenerating = false;
            statusDot.classList.remove('pulse');
            agentModeDisplay.innerText = configData.agent_mode || "Coding Assistant";
        }
    }

    // Input handlers
    chatInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
    chatInput.addEventListener('input', function() {
        this.style.height = 'auto';
        this.style.height = (this.scrollHeight) + 'px';
    });
    btnSend.addEventListener('click', sendMessage);
    btnNewChat.addEventListener('click', createNewChat);

    // Init
    loadConfigUI();
    loadConversationsList();
    if (conversations.length === 0) {
        createNewChat();
    } else {
        // Load the most recent conversation by default
        loadConversation(conversations[conversations.length - 1].id);
    }
});

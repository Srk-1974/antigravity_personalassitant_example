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

    // --- Navigation ---
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            tabs.forEach(t => t.classList.remove('active'));
            sections.forEach(s => s.classList.remove('active'));
            tab.classList.add('active');
            document.getElementById(tab.dataset.target).classList.add('active');
            
            if (tab.dataset.target === 'config-tab') {
                loadConfig();
            }
        });
    });

    // --- Config Logic ---
    let configData = {};

    async function loadConfig() {
        try {
            const res = await fetch('/api/config');
            configData = await res.json();
            
            selProvider.value = configData.active_provider;
            inpBaseUrl.value = configData.base_url;
            inpApiKey.value = configData.api_key;
            inpModelName.value = configData.model_name;
            inpTemp.value = configData.temperature;
            valTemp.innerText = configData.temperature;
            inpMaxTokens.value = configData.max_tokens;
            selAgentMode.value = configData.agent_mode;
            chkReasoning.checked = configData.reasoning_mode;
            inpSystemPrompt.value = configData.system_prompt;
            
            agentModeDisplay.innerText = configData.agent_mode;

        } catch (e) { console.error("Error loading config", e); }
    }

    inpTemp.addEventListener('input', (e) => {
        valTemp.innerText = e.target.value;
    });

    selProvider.addEventListener('change', (e) => {
        const provName = e.target.value;
        const prov = configData.providers.find(p => p.name === provName);
        if (prov) {
            inpBaseUrl.value = prov.base_url;
            inpModelName.value = prov.default_model;
            // API key logic could fetch from env, for now we clear or keep empty if unknown
            inpApiKey.value = ""; 
        }
    });

    btnSaveConfig.addEventListener('click', async () => {
        const payload = {
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

        try {
            const res = await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            if (res.ok) {
                configStatus.innerText = "Configuration saved successfully!";
                agentModeDisplay.innerText = selAgentMode.value;
                setTimeout(() => configStatus.innerText = "", 3000);
            }
        } catch (e) {
            configStatus.innerText = "Failed to save configuration.";
            configStatus.style.color = "#ef4444";
        }
    });

    // --- Chat Logic ---

    async function loadConversations() {
        try {
            const res = await fetch('/api/conversations');
            const convs = await res.json();
            historyList.innerHTML = '';
            
            convs.reverse().forEach(c => {
                const div = document.createElement('div');
                div.className = 'history-item';
                if (c.id === currentConvId) div.classList.add('active');
                div.innerText = c.title;
                div.onclick = () => loadConversation(c.id, c.title);
                historyList.appendChild(div);
            });
        } catch (e) { console.error("Error loading conversations", e); }
    }

    async function createNewChat() {
        try {
            const res = await fetch('/api/conversations', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ title: "New Conversation" })
            });
            const data = await res.json();
            currentConvId = data.id;
            currentChatTitle.innerText = "New Conversation";
            chatWindow.innerHTML = '<div class="welcome-message"><h2>Nexus AI is ready.</h2><p>How can I assist you today?</p></div>';
            loadConversations();
        } catch (e) { console.error("Error creating chat", e); }
    }

    async function loadConversation(id, title) {
        currentConvId = id;
        currentChatTitle.innerText = title;
        loadConversations(); // update active class

        try {
            const res = await fetch(`/api/conversations/${id}/messages`);
            const msgs = await res.json();
            
            chatWindow.innerHTML = '';
            if (msgs.length === 0) {
                chatWindow.innerHTML = '<div class="welcome-message"><h2>Nexus AI is ready.</h2><p>How can I assist you today?</p></div>';
            } else {
                msgs.forEach(m => appendMessage(m.role, m.content));
            }
        } catch (e) { console.error("Error loading messages", e); }
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

    async function sendMessage() {
        const text = chatInput.value.trim();
        if (!text || isGenerating || !currentConvId) return;

        chatInput.value = '';
        chatInput.style.height = 'auto';
        appendMessage('user', text);
        
        isGenerating = true;
        statusDot.classList.add('pulse');
        agentModeDisplay.innerText = "Agent is thinking...";

        try {
            const res = await fetch('/api/chat', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ conv_id: currentConvId, message: text })
            });
            const data = await res.json();
            appendMessage('agent', data.reply || "Error: No reply");
        } catch (e) {
            appendMessage('agent', "[Network Error] Unable to reach agent.");
        } finally {
            isGenerating = false;
            statusDot.classList.remove('pulse');
            agentModeDisplay.innerText = selAgentMode.value || "Coding Assistant";
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
    loadConfig();
    loadConversations().then(() => {
        if (!currentConvId) {
            createNewChat();
        }
    });
});

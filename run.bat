@echo off
echo Starting Personal AI Agent Server...
if exist build\Release\PersonalAIAgent.exe (
    build\Release\PersonalAIAgent.exe
) else if exist build\Debug\PersonalAIAgent.exe (
    build\Debug\PersonalAIAgent.exe
) else if exist build\PersonalAIAgent.exe (
    build\PersonalAIAgent.exe
) else (
    echo Cannot find executable. Did you run build.bat?
)

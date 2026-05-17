cmake_minimum_required(VERSION 3.15)
project(PersonalAIAgent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/ConfigManager.cpp
    src/MemoryManager.cpp
    src/LLMProvider.cpp
    src/Agent.cpp
    src/Server.cpp
)

# On Windows, we use WinHttpClient
if(WIN32)
    list(APPEND SOURCES src/WinHttpClient.cpp)
endif()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 wsock32 winhttp)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

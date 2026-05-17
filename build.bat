@echo off
echo Building Personal AI Agent...
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
echo Build Complete!
echo Run the application using: run.bat

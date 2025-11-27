@echo off
echo Compiling BattleShip Game...

echo Compiling Server...
g++ -o Server.exe GameManager/MainServer.cpp ExtraClasses.cpp GameManager/RunServer.cpp -lws2_32 -std=c++20
if %errorlevel% neq 0 (
    echo Failed to compile Server!
    pause
    exit /b
)

echo Compiling Player 1...
g++ -o P1.exe Game/Player.cpp ExtraClasses.cpp Game/RunP1.cpp -lws2_32 -std=c++20
if %errorlevel% neq 0 (
    echo Failed to compile P1!
    pause
    exit /b
)

echo Compiling Player 2...
g++ -o P2.exe Game/Player.cpp ExtraClasses.cpp Game/RunP2.cpp -lws2_32 -std=c++20
if %errorlevel% neq 0 (
    echo Failed to compile P2!
    pause
    exit /b
)

echo All compilations successful!
echo.
echo Now run the game using 'run.bat'
pause
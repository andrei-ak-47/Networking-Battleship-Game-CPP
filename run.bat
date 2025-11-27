@echo off
echo Starting BattleShip Game...
echo.

echo Make sure to start these in the correct order:
echo 1. First: Server (this window)
echo 2. Then: Player 1 (new window) 
echo 3. Then: Player 2 (new window)
echo.

echo Starting Server...
start "BattleShip Server" Server.exe

timeout /t 2 /nobreak >nul

echo Starting Player 1...
start "BattleShip Player 1" P1.exe

timeout /t 2 /nobreak >nul

echo Starting Player 2...
start "BattleShip Player 2" P2.exe

echo All processes started!
echo Check the opened windows for each component.
pause
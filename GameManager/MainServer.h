#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <array>
#include "../ExtraClasses.h" 
#pragma comment(lib, "ws2_32.lib")
class MainServer{
private:
    SOCKET MainSocket;
    
    SOCKET PlayerSockets[2];
    std::string PlayerNames[2];
    std::array<std::array<Cell, 10>, 10> PlayerBoards[2];
    int PlayerShipCounts[2] = {17, 17};
public:
    MainServer();
    ~MainServer();
    void ConnectPlayers();
    void GetPlayerNames();
    void RunServer();
    void PlaceShips();


    void RunGame();
    //Helper
    inline bool GameContinues();
};
std::string ArrayToStringBoard(std::array<std::array<char, 10>, 10> Board);
#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <array>
#include <string>
#include "../ExtraClasses.h"  // ✅ Go UP from Game folder
#pragma comment(lib, "ws2_32.lib")

class Player{
private:
    SOCKET PlayerSocket;
    std::array<std::array<Cell, 10>, 10> board;
    std::array<std::array<char, 10>, 10> oppBoard;
    std::string name;
    int PlayerID;
public:
    Player(int id);
    ~Player();
    void Run();
    void ConnectServer();
    void Setup();
    void GetPlayerName();
    void PlaceShips();

    bool PlaceHorizontally(int Shiplength, int row, int col);
    bool PlaceVertically(int Shiplength, int row, int col);

    bool VerifyHorizontally(int Shiplength, int row, int col, bool PrintError = true);
    bool VerifyVertically(int Shiplength, int row, int col, bool PrintError = true);

    void runGame();
    void YourTurn();
    void OppTurn();
    void PrintBothBoards();

    inline bool PossiblePlacement(int length, int row, int col){//Bunch of bs, dont even look here
        bool Pass1 = false, Pass2 = false, Pass3 = false, Pass4 = false;

        if(!LegalCoor(row) || !LegalCoor(col)) return false;

        std::cout << "[DEBUG]: ROW + LENGTH = " << row + length << '\n';
        std::cout << "[DEBUG]: COL - LENGTH = " << col - length << '\n';
        if(LegalCoor(row) && LegalCoor(col) && VerifyHorizontally(length, row, col, false)) Pass1 = true;
        if(LegalCoor(row) && LegalCoor(col - length) && VerifyHorizontally(length, row, col - length, false)) Pass2 = true;
        if(LegalCoor(row) && LegalCoor(col) && VerifyVertically(length, row, col, false)) Pass3 = true;
        if(LegalCoor(row + length) && LegalCoor(col) && VerifyVertically(length, row + length, col, false)) Pass4 = true;
    
        return Pass1 || Pass2 || Pass3 || Pass4;
    }
};
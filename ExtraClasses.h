#pragma once
//#include "C:/Programming/Networking/BattleShip/Game/Player.h"
#include <iostream>
#include <string>
#include <array>
enum class CellType{
    EMPTY,
    SHIP,
    MISS,
    HIT,
    ILLEGAL
};
struct Cell{
    CellType cellType;
    char character;
    Cell(){
        cellType = CellType::EMPTY;
        character = ' ';
    }
};
struct Ship{
    std::string name;
    size_t length;
    bool placed = false;
    Ship(std::string n, int l) : name(n), length(l){}
};
void PrintBoardArray(std::array<std::array<Cell, 10>, 10> board);
std::string ArrayToStringBoard(std::array<std::array<Cell, 10>, 10> board);
std::array<std::array<Cell, 10>, 10> StringToArrayBoard(char board[]);

inline bool LegalCoor(int coor){
    if(coor >= 0 && coor < 10) return true;
    return false;
}
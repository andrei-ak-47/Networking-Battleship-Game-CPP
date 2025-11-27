#include "ExtraClasses.h"
#include <iostream>
#include <array>
#include <string>

void PrintBoardArray(std::array<std::array<Cell, 10>, 10> board){
    std::cout << "  ";
    for(int i = 0; i < 10; i++){
        int num = i + 1;
        std::cout << num << '|';
    }
    std::cout << '\n';
    for(int i = 0; i < 10; i++){
        char letter = 'A' + i;
        std::cout << letter << ' ';
        for(int j = 0; j < 10; j++){
            std::cout << board[i][j].character << '|';
        }
        std::cout << '\n';
        std::cout << "  ";
        for(int i = 0; i < 10; i++){
            std::cout << "--";
        }
        std::cout << '\n';
    }
};

std::string ArrayToStringBoard(std::array<std::array<Cell, 10>, 10> board){
    std::string boardStr;
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            boardStr += board[i][j].character;
        }
    }
    return boardStr;
};

std::array<std::array<Cell, 10>, 10> StringToArrayBoard(char board[]){//Fix this later.
    std::array<std::array<Cell, 10>, 10> ret;
    for(int i = 0; i < 100; i++){
        ret[i / 10][i % 10].character = board[i];
        switch(board[i]){
            case ' ':{//Empty
                ret[i / 10][i % 10].cellType = CellType::EMPTY;
                break;
            }
            case 'S':{//Ship
                ret[i / 10][i % 10].cellType = CellType::SHIP;
                break;
            }
            case 'O':{//Missed
                ret[i / 10][i % 10].cellType = CellType::MISS;
                break;
            }
            case '*':{//Hit
                ret[i / 10][i % 10].cellType = CellType::HIT;
                break;
            }
        }
    }
    return ret;
}


#include "MainServer.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "../ExtraClasses.h"
#pragma comment(lib, "ws2_32.lib")

void MainServer::RunServer(){
    ConnectPlayers();

    PlaceShips();

    Sleep(1000);

    RunGame();
}

MainServer::MainServer(){
    WSAData wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    MainSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(MainSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    // Prepare to accept player connections
}

MainServer::~MainServer(){
    closesocket(MainSocket);
    closesocket(PlayerSockets[0]);
    closesocket(PlayerSockets[1]);

    WSACleanup();
}

void MainServer::ConnectPlayers(){
    listen(MainSocket, 2);
    int numOfClients = 0;

    while(numOfClients < 2){
        SOCKET clientSocket = accept(MainSocket, NULL, NULL);
        if(clientSocket == INVALID_SOCKET){ std::cerr << "accept failed\n"; continue; }
        PlayerSockets[numOfClients++] = clientSocket;
        std::cout << "Player connected (" << numOfClients << "/2)\n";
    }
    Sleep(1000);
    GetPlayerNames();//Should be able to get player names
}

void MainServer::GetPlayerNames(){
    fd_set readfds;
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    bool complete1 = false;
    bool complete2 = false;

    std::string msg = "NAME_REQUEST";
    int sent1 = send(PlayerSockets[0], msg.c_str(), msg.size() + 1, 0);
    int sent2 = send(PlayerSockets[1], msg.c_str(), msg.size() + 1, 0);
    if (sent1 == SOCKET_ERROR || sent2 == SOCKET_ERROR) {
        std::cerr << "Failed to send name request\n";
        return;
    }

    std::cout << "Waiting for player names...\n";
    std::cout << "[DEBUG]: Send Request for Names\n";

    while(!complete1 || !complete2){
        FD_ZERO(&readfds);
        FD_SET(PlayerSockets[0], &readfds);
        FD_SET(PlayerSockets[1], &readfds);

        int result = select(0, &readfds, NULL, NULL, &timeout);

        if(result > 0){
            if(FD_ISSET(PlayerSockets[0], &readfds)){
                char nameBuffer[100];
                int bytes = recv(PlayerSockets[0], nameBuffer, sizeof(nameBuffer)-1, 0);
                if(bytes > 0){
                    // construct string from received bytes (avoid relying on a null terminator)
                    PlayerNames[0] = std::string(nameBuffer, bytes);
                } else {
                    PlayerNames[0] = std::string();
                }
                complete1 = true;
            }
            if(FD_ISSET(PlayerSockets[1], &readfds)){
                char nameBuffer[100];
                int bytes = recv(PlayerSockets[1], nameBuffer, sizeof(nameBuffer) - 1, 0);
                if(bytes > 0){
                    PlayerNames[1] = std::string(nameBuffer, bytes);
                } else {
                    PlayerNames[1] = std::string();
                }
                complete2 = true;
            }
        }
    }
    std::cout << "Got player names\n";
    std::cout << "P1: " << PlayerNames[0] << "P2: " << PlayerNames[1];
}


void MainServer::PlaceShips(){
    std::string msg = "PLACE_SHIPS";
    for(int i = 0; i < 2; i++){
        send(PlayerSockets[i], msg.c_str(), msg.size(), 0);
    }
    std::cout << "Waiting for players to place ships...\n";
    fd_set readfds;
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    bool complete1 = false;
    bool complete2 = false;
    while(!complete1 || !complete2){
        FD_ZERO(&readfds);
        FD_SET(PlayerSockets[0], &readfds);
        FD_SET(PlayerSockets[1], &readfds);

        int result = select(0, &readfds, NULL, NULL, &timeout);
        
        if(result > 0){
            char buffer[256] = {0};  // Larger buffer for board data
            if(FD_ISSET(PlayerSockets[0], &readfds) && !complete1){
                int bytes = recv(PlayerSockets[0], buffer, sizeof(buffer) - 1, 0);  // ✅ FIXED: PlayerSockets[0]
                if(bytes > 0){
                    buffer[bytes] = '\0';
                    std::string received(buffer);
                    if(received.find("BOARD: ") == 0){  // ✅ FIXED: "BOARD:" not "BOARD: "

                        complete1 = true;
                        std::cout << "[DEBUG]: RECEIVED BOARD FROM P1\n";

                        char BoardStr[100];
                        strncpy(BoardStr, &buffer[7], strlen(buffer) - 7);
                        BoardStr[strlen(buffer) - 7] = '\0';

                        PlayerBoards[0] = StringToArrayBoard(BoardStr);
                        std::cout << "[DEBUG]: BOARD P2: \n";
                        PrintBoardArray(PlayerBoards[0]);
                    }
                }
            }
            if(FD_ISSET(PlayerSockets[1], &readfds) && !complete2){
                int bytes = recv(PlayerSockets[1], buffer, sizeof(buffer) - 1, 0);  // ✅ FIXED: PlayerSockets[1]
                if(bytes > 0){
                    buffer[bytes] = '\0';
                    std::string received(buffer);
                    if(received.find("BOARD: ") == 0){  // ✅ FIXED: "BOARD:" not "BOARD: "
                        complete2 = true;
                        std::cout << "[DEBUG]: RECEIVED BOARD FROM P2\n";

                        char BoardStr[100];
                        strncpy(BoardStr, &buffer[7], strlen(buffer) - 7);
                        BoardStr[strlen(buffer) - 7] = '\0';

                        PlayerBoards[1] = StringToArrayBoard(BoardStr);
                        std::cout << "[DEBUG]: BOARD P2: \n";
                        PrintBoardArray(PlayerBoards[1]);
                    }
                }
            }
        }
    }
    std::cout << "Both boards received! Starting battle phase...\n";
    //Server Boards Done, Player Ships ready, Battle can start
}

void MainServer::RunGame(){
    int shooterPlayerID = 0;
    int oppPlayerID = 1;
    SOCKET shooterSocket;
    SOCKET oppSocket;

    char buffer[1024];
    int bytes;
    std::string msgToShooter;
    std::string msgToOpp;
    while(GameContinues()){
        memset(buffer, 0, sizeof(buffer));//clear buffer

        shooterSocket = PlayerSockets[shooterPlayerID];
        oppSocket = PlayerSockets[oppPlayerID];

        msgToShooter = "YOUR_TURN";
        send(shooterSocket, msgToShooter.c_str(), msgToShooter.size(), 0);
        msgToOpp = "OPP_TURN";
        send(oppSocket, msgToOpp.c_str(), msgToOpp.size(), 0);
        //Sent both players their messages

        bytes = recv(shooterSocket, buffer, sizeof(buffer), 0);//Recieves the Shot Coor
        if(bytes > 0){
            buffer[bytes] = '\0';
            std::string bufferStr = buffer;
            std::cout << "[DEBUG]: SHOOTER SOCKET SENT: " << bufferStr << '\n';
            int row = static_cast<int>(std::toupper(bufferStr[0])) - 65;
            int col = std::stoi(bufferStr.substr(1)) - 1;
            std::cout << "[DEBUG]: ROW IS " << row << '\n';
            std::cout << "[DEBUG]: COL IS " << col << '\n';
            switch((PlayerBoards[oppPlayerID])[row][col].cellType){
                case CellType::EMPTY:{
                    std::cout << "[DEBUG]: CASE IS EMPTY\n";
                    msgToShooter = "YOU_MISSED";
                    send(shooterSocket, msgToShooter.c_str(), msgToShooter.size(), 0); 

                    msgToOpp = "OPP_MISSED: ";
                    msgToOpp += std::toupper(bufferStr[0]);
                    msgToOpp += std::to_string(col + 1);
                    send(oppSocket, msgToOpp.c_str(), msgToOpp.size(), 0);
                    //Only update the opponents ship board

                    (PlayerBoards[oppPlayerID])[row][col].cellType = CellType::MISS;
                    (PlayerBoards[oppPlayerID])[row][col].character = 'O';

                    break;
                }

                case CellType::SHIP:{
                    std::cout << "[DEBUG]: CASE IS SHIP\n";
                    msgToShooter = "YOU_HIT";
                    send(shooterSocket, msgToShooter.c_str(), msgToShooter.size(), 0);

                    msgToOpp = "YOU_GOT_HIT: ";
                    msgToOpp += std::toupper(bufferStr[0]);
                    msgToOpp += std::to_string(col + 1);
                    send(oppSocket, msgToOpp.c_str(), msgToOpp.size(), 0);

                    //Only update the opponents ship board
                    (PlayerBoards[oppPlayerID])[row][col].cellType = CellType::HIT;
                    (PlayerBoards[oppPlayerID])[row][col].character = '*';

                    PlayerShipCounts[oppPlayerID]--;

                    break;
                }

                default:{
                    std::cout << "WOW HOW TF DID WE GET HERE\n";

                    if((PlayerBoards[oppPlayerID])[row][col].cellType == CellType::ILLEGAL) std::cout << "WERE ILLEGAL NOW\n";
                    if((PlayerBoards[oppPlayerID])[row][col].cellType == CellType::HIT) std::cout << "WHY ARE WE HIT\n";
                    if((PlayerBoards[oppPlayerID])[row][col].cellType == CellType::MISS) std::cout << "WE MISSED\n";
                }
            }
        }
        else{
            std::cerr << "[ERROR]: CONNECTION LOST\nTERMINATING GAME\n";
            break;
        }
        std::cout << "\n[DEBUG]: BOARD P1\n";
        PrintBoardArray(PlayerBoards[0]);
        std::cout << "\n[DEBUG]: BOARD P2\n";
        PrintBoardArray(PlayerBoards[1]);

        std::swap(shooterPlayerID, oppPlayerID);
    }
    if(PlayerShipCounts[0] == 0){
        send(PlayerSockets[0], "YOU_LOSE", 8, 0); 
        send(PlayerSockets[1], "YOU_WIN", 7, 0); 
    } 
    else if(PlayerShipCounts[1] == 0){
        send(PlayerSockets[0], "YOU_WIN", 7, 0); 
        send(PlayerSockets[1], "YOU_LOSE", 8, 0); 
    }
}

bool MainServer::GameContinues(){
    return PlayerShipCounts[0] > 0 && PlayerShipCounts[1] > 0;
}
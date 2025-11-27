#include "Player.h"
#include <algorithm>
#include <limits>
void Player::Run(){ 
    char buffer[1024];
    int bytes;
    Setup();
    //SETUP SHOULD BE DONE
    runGame();
    
    closesocket(PlayerSocket);
}

Player::Player(int id) : PlayerID(id){
    WSAData wsaData; 
    WSAStartup(MAKEWORD(2,2), &wsaData);

    PlayerSocket = socket(AF_INET, SOCK_STREAM, 0);

    for(auto& row : oppBoard) row.fill(' ');
}

Player::~Player(){
    closesocket(PlayerSocket);
    WSACleanup();
}

void Player::Setup(){
    int bytes;
    char msg[100];
    ConnectServer();
    bytes = recv(PlayerSocket, msg, sizeof(msg), 0);
    if(bytes > 0){
        msg[bytes] = '\0';
        if(strcmp(msg, "NAME_REQUEST") == 0){
            GetPlayerName();
            send(PlayerSocket, name.c_str(), name.size(), 0);
            std::cout << "[DUBUG]: Sent: " << name << '\n';
        }
        //std::cout << msg << '\n';
    }
    bytes = 0;
    bytes = recv(PlayerSocket, msg, sizeof(msg), 0);
    if(bytes > 0){
        msg[bytes] = '\0';
        
        std::string recievedMsg = msg;
        
        if(strcmp(msg, "PLACE_SHIPS") == 0){
            PlaceShips();
        } else {
            std::cout << "STRING COMPARE FAILED - Expected 'PLACE_SHIPS' but got different" << std::endl;
        }
    }

    for (auto &row : board){//TO CLEAR ALL "ILLEGAL" ENUM SQUARES TO "EMPTY" SQUARES
        for (auto& cell : row) {
            if (cell.cellType == CellType::ILLEGAL)
                cell.cellType = CellType::EMPTY;
        }
    }

}

void Player::ConnectServer(){
    std::string serverIP;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    std::cout << "Enter in servers IP address: ";
    std::getline(std::cin, serverIP);
    if(serverIP.empty()){
        std::cout << "Empty\n";
        serverIP = "127.0.0.1";
    }
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    
    connect(PlayerSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "Connected to " << serverIP << ":8080\n";  // ✅ Debug confirmation
}

void Player::GetPlayerName(){
    // Prompt for username; do not pre-clear the stream (blocks waiting for input)
    std::cout << "Enter your username (or press Enter for default): ";
    std::string username;
    std::getline(std::cin, username);
    
    // Trim whitespace
    username.erase(0, username.find_first_not_of(" \t\n\r"));
    username.erase(username.find_last_not_of(" \t\n\r") + 1);
    
    if(username.empty()){
        username = "Player " + std::to_string(PlayerID);
    }
    name = username;
    //std::cout << "Your name is: " << name << "\n";
}

void Player::PlaceShips(){
    std::array<Ship, 5> Ships = {
        Ship("Destroyer", 2),
        Ship("Submarine", 3),
        Ship("Cruiser", 3),
        Ship("Battleship", 4),
        Ship("Carrier", 5)
    };
    for(int i = 0; i < 5; i++){
        PrintBoardArray(board);
        bool validCoor = true;
        std::string input;
        int row, col;
        do{
            if(!validCoor) std::cout << "[ERROR]: NOT A POSSIBLE PLACEMENT FOR SHIP OR INVALID COOR\n";
            std::cout << "Where would you like to place your " << Ships[i].name << "?\n";
            std::getline(std::cin, input);
            row = static_cast<int>(std::toupper(input[0])) - 65;
            col = std::stoi(input.substr(1)) - 1;
            std::cout << "[DEBUG]: ROW " << row << "\tCOL " << col << '\n';
            validCoor = PossiblePlacement(Ships[i].length, row, col);
        }while(!validCoor);

        std::cout << "Horizontal or vertical? (h/v): ";
        std::string dirStr;
        do{
            std::getline(std::cin, dirStr);
            char dir = dirStr[0];

            if(dir == 'h' || dir == 'H'){
                Ships[i].placed = PlaceHorizontally(Ships[i].length, row, col);
            }
            else if(dir == 'v' || dir == 'V'){
                Ships[i].placed = PlaceVertically(Ships[i].length, row, col);
            }
            else{
                std::cerr << "Not a valid choice, please pick either h or v.";
            }
        }while(!Ships[i].placed);
    }
    std::string boardStr = "BOARD: " +  ArrayToStringBoard(board);
    
    send(PlayerSocket, boardStr.c_str(), boardStr.size(), 0);
}

bool Player::VerifyHorizontally(int Shiplength, int row, int col, bool PrintError){
    if(col + Shiplength > 10){
        if(PrintError) std::cout << "[ERROR]: Ship Overflows Barrier";
        return false;
    }

    for(int i = col; i < col + Shiplength; i++){
        if(board[row][i].cellType == CellType::SHIP 
            || board[row][i].cellType == CellType::ILLEGAL){
                if(PrintError) std::cout << "[ERROR]: SHIP TOUCHES ANOTHER SHIP\n";
                return false;
            }
    }
    return true;
}

bool Player::VerifyVertically(int Shiplength, int row, int col, bool PrintError){
    if(row - Shiplength + 1 < 0){
        if(PrintError) std::cout << "[ERROR]: Ship Overflows Barrier";
        return false;
    }

    for(int i = row; i > row - Shiplength; i--){
        if(board[i][col].cellType == CellType::SHIP 
            || board[i][col].cellType == CellType::ILLEGAL){
                if(PrintError) std::cout << "[ERROR]: SHIP TOUCHES ANOTHER SHIP\n";
                return false;
            }
    }
    return true;
}

bool Player::PlaceHorizontally(int Shiplength, int row, int col){
    if(!VerifyHorizontally(Shiplength, row, col)) return false;

    for(int i = col - 1; i < col + Shiplength + 1; i++){//start at left, end at right
        if(i == col - 1){//Left case
            if(LegalCoor(i)){
                board[row][i].cellType = CellType::ILLEGAL;
            }
            int UpperCoor = row - 1;
            int LowerCoor = row + 1;
            if(LegalCoor(UpperCoor) && LegalCoor(i)){
                board[UpperCoor][i].cellType = CellType::ILLEGAL;
            }
            if(LegalCoor(LowerCoor) && LegalCoor(i)){
                board[LowerCoor][i].cellType = CellType::ILLEGAL;
            }  
        }

        else if(i == col + Shiplength){//right case
            if(LegalCoor(i)){
                board[row][i].cellType = CellType::ILLEGAL;
            }

            int UpperCoor = row - 1;
            int LowerCoor = row + 1;
            if(LegalCoor(UpperCoor) && LegalCoor(i)){
                board[UpperCoor][i].cellType = CellType::ILLEGAL;
            }
            if(LegalCoor(LowerCoor) && LegalCoor(i)){
                board[LowerCoor][i].cellType = CellType::ILLEGAL;
            } 
        }

        else{//Normal case
            board[row][i].cellType = CellType::SHIP;
            board[row][i].character = 'S';

            int UpperCoor = row - 1;
            int LowerCoor = row + 1;
            if(LegalCoor(UpperCoor) && LegalCoor(i)){
                board[UpperCoor][i].cellType = CellType::ILLEGAL;
            }
            if(LegalCoor(LowerCoor) && LegalCoor(i)){
                board[LowerCoor][i].cellType = CellType::ILLEGAL;
            }  
        }
    }
    return true;
}

bool Player::PlaceVertically(int Shiplength, int row, int col){
    if(!VerifyVertically(Shiplength, row, col)) return false;

    for(int i = row + 1; i > row - Shiplength - 1; i--){//Start at bottom, end at top
        if(i == row + 1){//Lower case
            if(LegalCoor(i)){
                board[i][col].cellType = CellType::ILLEGAL;
            }
            int LeftCoor = col - 1;
            int RightCoor = col + 1;
            if(LegalCoor(LeftCoor) && LegalCoor(i)){
                board[i][LeftCoor].cellType = CellType::ILLEGAL;
            }
            if(LegalCoor(RightCoor) && LegalCoor(i)){
                board[i][RightCoor].cellType = CellType::ILLEGAL;
            }
        }

        else if(i == row - Shiplength){//Upper case
            if(LegalCoor(i)){
                board[i][col].cellType = CellType::ILLEGAL;
            }

            int LeftCoor = col - 1;
            int RightCoor = col + 1;
            if(LegalCoor(LeftCoor) && LegalCoor(i)){
                board[i][LeftCoor].cellType = CellType::ILLEGAL;
            }
            if(LegalCoor(RightCoor) && LegalCoor(i)){
                board[i][RightCoor].cellType = CellType::ILLEGAL;
            }
        }

        else{//Normal Case
            board[i][col].cellType = CellType::SHIP;
            board[i][col].character = 'S';

            int LeftCoor = col - 1;
            int RightCoor = col + 1;
            if(LegalCoor(LeftCoor) && LegalCoor(i)){
                board[i][LeftCoor].cellType = CellType::ILLEGAL;
            }
            if(LegalCoor(RightCoor) && LegalCoor(i)){
                board[i][RightCoor].cellType = CellType::ILLEGAL;
            }
        }
    }
    return true;
}

void Player::runGame(){
    char buffer[1024];
    int bytes;
    while(true){
        bytes = recv(PlayerSocket, buffer, sizeof(buffer), 0);
        if(bytes > 0){
            buffer[bytes] = '\0';
            std::string bufferStr = buffer;
            if(bufferStr.find("YOU_WIN") != std::string::npos
                || bufferStr.find("YOU_LOSE") != std::string::npos) break;

            if(bufferStr.find("YOUR_TURN") != std::string::npos) YourTurn();
            if(bufferStr.find("OPP_TURN") != std::string::npos) OppTurn();
        }   
        else{
            std::cerr << "[ERROR]: SERVER CONNECTION LOST\nTERMINATING GAME";
            break;
        }
    }
    if(buffer == "YOU_WIN"){
        std::cout << "YOU WON!!\n";
    }
    else if(buffer == "YOU_LOSE"){
        std::cout << "YOU LOST!!\n";
    }
    else{
        std::cout << "[ERROR]: RECIEVED WRONG MESSAGE FROM SERVER\n";
    }   
}

void Player::YourTurn(){
    bool validCoor = true;
    std::string coor;

    int row, col;
    do{
        if(!validCoor) std::cout << "[ERROR]: INVALID COOR OR ALREADY HAVE BEEN SHOT\n";
        std::cout << "Enter in the coordinate you would like to shoot: ";
        std::getline(std::cin, coor);
        row = static_cast<int>(std::toupper(coor[0])) - 65;
        col = std::stoi(coor.substr(1)) - 1;
        if(!LegalCoor(row) || !LegalCoor(col) || oppBoard[row][col] != ' ') validCoor = false;
    }while(!validCoor);
    send(PlayerSocket, coor.c_str(), coor.size(), 0);//Send coordinate to server

    int bytes;
    char buffer[128];

    bytes = recv(PlayerSocket, buffer, sizeof(buffer), 0);
    if(bytes > 0){
        std::cout << "[DEBUG]: BUFFER IS" << buffer << '\n';
        buffer[bytes] = '\0';
        std::string bufferStr = buffer;
        if(bufferStr.find("YOU_HIT") != std::string::npos){
            oppBoard[row][col] = '*';
            PrintBothBoards();
            std::cout << char(std::toupper(coor[0])) << col + 1 << " IS A HIT!\n";
        }

        else if(bufferStr.find("YOU_MISSED") != std::string::npos){
            oppBoard[row][col] = 'O';
            PrintBothBoards();
            std::cout << char(std::toupper(coor[0])) << col + 1 << " IS A MISS!\n";
        }
        else{
            std::cerr << "[ERROR]: RECIEVED WRONG MESSAGE FROM SERVER\n";
        }
    }
}

void Player::OppTurn(){
    std::cout << "Waiting for opponents turn...\n";

    int bytes;
    char buffer[128];

    bytes = recv(PlayerSocket, buffer, sizeof(buffer), 0);
    if(bytes > 0){
        buffer[bytes] = '\0';
        std::string bufferStr = buffer;

        if(bufferStr.find("YOU_GOT_HIT") != std::string::npos){
            int row = bufferStr[13] - 65;//13 is the the index of the letter
            int col = std::stoi(bufferStr.substr(14));
            board[row][col - 1].cellType = CellType::HIT;
            board[row][col - 1].character = '*';
            PrintBothBoards();
            std::cout << "YOU GOT HIT AT " << bufferStr[13] << col << "!\n";
        }
        else if(bufferStr.find("OPP_MISSED") != std::string::npos){
            int row = bufferStr[12]- 65;
            int col = std::stoi(bufferStr.substr(13));
            board[row][col - 1].cellType = CellType::MISS;
            board[row][col - 1].character = 'O';
            PrintBothBoards();
            std::cout << "THE OPPONENT MISSED AT " << bufferStr[12] << col << "!\n";
        }
        else{
            std::cerr << "[ERROR]: RECIVED WRONG MESSAGE FROM SERVER\n";
        }
    }
}

void Player::PrintBothBoards(){
    std::cout << "  ";
    for(int i = 0; i < 10; i++){
        int num = i + 1;
        std::cout << num;
        if(i != 9) std::cout <<  '|';
    }

    std::cout << "  ";
    for(int i = 0; i < 10; i++){
        int num = i + 1;
        std::cout << num;
        if(i != 9) std::cout << '|';
    }

    std::cout << '\n';
    for(int i = 0; i < 10; i++){
        char letter = 'A' + i;
        std::cout << letter << ' ';
        for(int j = 0; j < 10; j++){
            std::cout << board[i][j].character << '|';
        }
        std::cout << "  ";
        for(int j = 0; j < 10; j++){
            std::cout << oppBoard[i][j] << '|';
        }
        std::cout << '\n';
        std::cout << "  ";
        for(int i = 0; i < 10; i++){
            std::cout << "--";
        }
        std::cout << "  ";
        for(int i = 0; i < 10; i++){
            std::cout << "--";
        }
        std::cout << '\n';
    }
}


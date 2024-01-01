// chess_server.c
// Compile with: gcc chess_server.c -o chess_server -lSDL2 -lSDL2_ttf -lSDL2_image

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BOARD_SIZE 8
#define SQUARE_SIZE 100

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int board[BOARD_SIZE][BOARD_SIZE] = {
    {2, 4, 3, 5, 6, 3, 4, 2},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {7, 7, 7, 7, 7, 7, 7, 7},
    {8, 10, 9, 11, 12, 9, 10, 8}
};

char* whitePieceImages[] = {
    "chess_pieces/white_pawn.png",
    "chess_pieces/white_rook.png",
    "chess_pieces/white_bishop.png",
    "chess_pieces/white_knight.png",
    "chess_pieces/white_queen.png",
    "chess_pieces/white_king.png"
};

char* blackPieceImages[] = {
    "chess_pieces/black_pawn.png",
    "chess_pieces/black_rook.png",
    "chess_pieces/black_bishop.png",
    "chess_pieces/black_knight.png",
    "chess_pieces/black_queen.png",
    "chess_pieces/black_king.png"
};

int selectedPiece[2] = {-1, -1}; // Coordinates of the selected piece
int offset[2] = {0, 0}; // Offset for the selected piece's position relative to the mouse

void renderPieces();
void handleEvents(int clientSocket);
void sendBoardState(int clientSocket);
int isMoveValid(int fromX, int fromY, int toX, int toY);
void movePiece(int fromX, int fromY, int toX, int toY);

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Configure server address
    memset(&serverAddr, '0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(12345);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Binding failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept connection
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen)) < 0) {
        perror("Acceptance failed");
        return 1;
    }

    printf("Connection established with client\n");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Chess Server", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        return 1;
    }

    // Send initial board state to the client
    sendBoardState(clientSocket);

    while (1) {
        handleEvents(clientSocket);
        renderPieces();
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    close(clientSocket);
    close(serverSocket);

    return 0;
}

void sendBoardState(int clientSocket) {
    // Send initial board state to the client
    if (send(clientSocket, board, sizeof(board), 0) < 0) {
        perror("Send failed");
        exit(1);
    }
}

void handleEvents(int clientSocket) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            SDL_Quit();
            exit(0);
        }
    }

    int move[4];

    // Receive move from the client
    if (recv(clientSocket, move, sizeof(move), 0) < 0) {
        perror("Receive failed");
        exit(1);
    }

    // Process the received move
    int fromX = move[0];
    int fromY = move[1];
    int toX = move[2];
    int toY = move[3];

    if (isMoveValid(fromX, fromY, toX, toY)) {
        movePiece(fromX, fromY, toX, toY);
    }
}

int isMoveValid(int fromX, int fromY, int toX, int toY) {
    // Check if the move is within the board boundaries
    if (toX < 0 || toX >= BOARD_SIZE || toY < 0 || toY >= BOARD_SIZE) {
        return 0;
    }

    // Check if the destination square is empty or has an opponent's piece
    if (board[toY][toX] == 0 || (board[fromY][fromX] < 7 && board[toY][toX] >= 7) || (board[fromY][fromX] >= 7 && board[toY][toX] < 7)) {
        // Validate moves based on the piece type
        int piece = board[fromY][fromX];
        int dx = abs(toX - fromX);
        int dy = abs(toY - fromY);

        switch (piece) {
            case 1: // Pawn
                if (fromY == toY && board[toY][toX] == 0) {
                    // Pawn can move forward one square if the destination is empty
                    return 1;
                } else if (dy == 1 && dx == 1 && board[toY][toX] != 0 && (board[toY][toX] >= 7) != (piece >= 7)) {
                    // Pawn can capture diagonally
                    return 1;
                }
                break;

            case 2: // Rook
                if (fromX == toX || fromY == toY) {
                    // Rook can move horizontally or vertically
                    return 1;
                }
                break;

            case 3: // Bishop
                if (dx == dy) {
                    // Bishop can move diagonally
                    return 1;
                }
                break;

            case 4: // Knight
                if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) {
                    // Knight moves in an "L" shape
                    return 1;
                }
                break;

            case 5: // Queen
                if (fromX == toX || fromY == toY || dx == dy) {
                    // Queen can move horizontally, vertically, or diagonally
                    return 1;
                }
                break;

            case 6: // King
                if (dx <= 1 && dy <= 1) {
                    // King can move one square in any direction
                    return 1;
                }
                break;
        }
    }

    return 0;
}

void movePiece(int fromX, int fromY, int toX, int toY) {
    // Perform the move if it's valid
    board[toY][toX] = board[fromY][fromX];
    board[fromY][fromX] = 0;

    // Send the updated board state to the client
    if (send(window, board, sizeof(board), 0) < 0) {
        perror("Send failed");
        exit(1);
    }
}

void renderPieces() {
    SDL_RenderClear(renderer);

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            SDL_Rect squareRect = {col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};

            if ((row + col) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 206, 158, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 209, 139, 71, 255);
            }
            SDL_RenderFillRect(renderer, &squareRect);

            int piece = board[row][col];
            if (piece != 0) {
                char* pieceImage;

                if (piece < 7) {
                    pieceImage = whitePieceImages[piece - 1];
                } else {
                    // Adjust the index for black pieces
                    pieceImage = blackPieceImages[piece - 7];
                }

                // Load and render piece image
                SDL_Surface* pieceSurface = IMG_Load(pieceImage);
                if (pieceSurface == NULL) {
                    printf("Error loading image: %s\n", IMG_GetError());
                    // Handle error as needed
                }
                SDL_Texture* pieceTexture = SDL_CreateTextureFromSurface(renderer, pieceSurface);
                if (pieceTexture == NULL) {
                    printf("Error creating texture: %s\n", SDL_GetError());
                    // Handle error as needed
                }
                SDL_RenderCopy(renderer, pieceTexture, NULL, &squareRect);

                SDL_FreeSurface(pieceSurface);
                SDL_DestroyTexture(pieceTexture);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

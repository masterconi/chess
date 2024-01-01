// chess_client.c
// Compile with: gcc chess_client.c -o chess_client -lSDL2 -lSDL2_ttf -lSDL2_image

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

int board[BOARD_SIZE][BOARD_SIZE];

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
void handleEvents(int serverSocket);
void receiveBoardState(int serverSocket);

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Configure server address
    memset(&serverAddr, '0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }

    // Connect to the server
    if (connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection Failed");
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Chess Client", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE, SDL_WINDOW_SHOWN);
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

    // Initial board state
    receiveBoardState(serverSocket);

    while (1) {
        handleEvents(serverSocket);
        renderPieces();
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    close(serverSocket);

    return 0;
}

void receiveBoardState(int serverSocket) {
    // Receive initial board state from the server
    if (recv(serverSocket, board, sizeof(board), 0) < 0) {
        perror("Receive failed");
        exit(1);
    }
}

void handleEvents(int serverSocket) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            SDL_Quit();
            exit(0);
        } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Convert mouse coordinates to board coordinates
            int boardX = mouseX / SQUARE_SIZE;
            int boardY = mouseY / SQUARE_SIZE;

            if (selectedPiece[0] == -1) {
                // No piece selected, check if there is a piece on the clicked square
                if (board[boardY][boardX] != 0) {
                    selectedPiece[0] = boardX;
                    selectedPiece[1] = boardY;
                    offset[0] = mouseX - (selectedPiece[0] * SQUARE_SIZE);
                    offset[1] = mouseY - (selectedPiece[1] * SQUARE_SIZE);
                }
            } else {
                // A piece is already selected, try to move it to the clicked square
                int move[4] = {selectedPiece[0], selectedPiece[1], boardX, boardY};
                if (send(serverSocket, move, sizeof(move), 0) < 0) {
                    perror("Send failed");
                    exit(1);
                }
                selectedPiece[0] = -1;
                selectedPiece[1] = -1;
            }
        } else if (event.type == SDL_MOUSEMOTION && selectedPiece[0] != -1) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Update the position of the selected piece based on mouse movement
            selectedPiece[0] = (mouseX - offset[0]) / SQUARE_SIZE;
            selectedPiece[1] = (mouseY - offset[1]) / SQUARE_SIZE;
        }
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

int isMoveValid(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    // Implement your move validation logic here
    return 1; // For now, consider all moves valid
}

void makeMove(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    // Implement your move logic here
    char temp = board[toRow][toCol];
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = temp;
}

void handleClient(int clientSocket, char board[8][8]) {
    char buffer[MAX_BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // Receive move from client
        recv(clientSocket, buffer, sizeof(buffer), 0);

        // Extract move information from the received string
        int fromRow, fromCol, toRow, toCol;
        sscanf(buffer, "%d %d %d %d", &fromRow, &fromCol, &toRow, &toCol);

        // Check if the move is valid
        if (isMoveValid(board, fromRow, fromCol, toRow, toCol)) {
            // Make the move
            makeMove(board, fromRow, fromCol, toRow, toCol);

            // Send the updated board to the client
            send(clientSocket, board, sizeof(board), 0);
        } else {
            // Inform the client that the move is invalid
            send(clientSocket, "Invalid move. Try again.", sizeof("Invalid move. Try again."), 0);
        }
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    // Initialize the chess board
    char chessBoard[8][8];
    // Initialize or set up the chess board as needed

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept connections and handle clients
    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
        if (clientSocket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Handle the client in a separate function
        handleClient(clientSocket, chessBoard);

        close(clientSocket); // Close the client socket after handling
    }

    close(serverSocket); // Close the server socket

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    while (1) {
        // Get move input from the player
        int fromRow, fromCol, toRow, toCol;
        printf("Enter move (fromRow fromCol toRow toCol): ");
        scanf("%d %d %d %d", &fromRow, &fromCol, &toRow, &toCol);

        // Send move to server
        sprintf(buffer, "%d %d %d %d", fromRow, fromCol, toRow, toCol);
        send(clientSocket, buffer, sizeof(buffer), 0);

        // Receive updated board from server
        recv(clientSocket, buffer, sizeof(buffer), 0);
        printf("Updated Board:\n%s", buffer);

        // Implement game logic or rendering here

        // Break the loop or add exit conditions
    }

    close(clientSocket);

    return 0;
}

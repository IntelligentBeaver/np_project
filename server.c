// server.c: UDP Guess-the-Number Game Server
// Implements a simple UDP server that receives numeric guesses and responds with hints
// TODO: Add comments for each function and major code block 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define BUF_SIZE 512 // Max buffer size for communication

// Generates a random number within a given range
int generate_number(int min, int max) {
    return min + rand() % (max - min + 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    // Create a UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Setup the server address structure
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));
    int target = generate_number(1, 100); // Initialize first target number
    printf("[Server] Listening on port %s. Target=%d\n", argv[1], target);

    char buffer[BUF_SIZE];
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);

    // Infinite loop to process incoming guesses
    while (1) {
        memset(buffer, 0, BUF_SIZE);

        // Receive data from client
        int recv_len = recvfrom(sock, buffer, BUF_SIZE-1, 0,
                                (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len == SOCKET_ERROR) {
            fprintf(stderr, "recvfrom() failed: %d\n", WSAGetLastError());
            continue;
        }

        int guess = atoi(buffer); // Convert received guess to integer
        const char *reply;

        // Determine server response
        if (guess < target) reply = "HIGHER\r\n";
        else if (guess > target) reply = "LOWER\r\n";
        else {
            reply = "CORRECT\r\n";
            target = generate_number(1, 100); // Reset game with new number
            printf("[Server] Client %s:%d guessed correctly. New target=%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), target);
        }

        // Send response to client
        sendto(sock, reply, (int)strlen(reply), 0,
               (struct sockaddr*)&client_addr, addr_len);
    }

    // Cleanup (never reached due to infinite loop)
    closesocket(sock);
    WSACleanup();
    return EXIT_SUCCESS;
}
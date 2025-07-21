// client.c: UDP Guess-the-Number Game Client
// Sends user guesses to server and receives response hints

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define BUF_SIZE 512 // Max buffer size for communication

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    // Create UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Setup server address structure
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];
    int play = 1;

    while (play) {
        printf("\nGuess a number between 1 and 100.\n");
        while (1) {
            printf("Enter guess: ");
            fgets(send_buf, BUF_SIZE, stdin); // Read user input

            // Ensure CRLF line termination
            size_t len = strlen(send_buf);
            if (send_buf[len-1] != '\n') strcat(send_buf, "\r\n");
            else {
                send_buf[len-1] = '\r'; send_buf[len] = '\n'; send_buf[len+1] = '\0';
            }

            // Send guess to server
            sendto(sock, send_buf, (int)strlen(send_buf), 0,
                   (struct sockaddr*)&server_addr, sizeof(server_addr));

            // Receive server's response
            int recv_len = recvfrom(sock, recv_buf, BUF_SIZE-1, 0, NULL, NULL);
            if (recv_len == SOCKET_ERROR) {
                fprintf(stderr, "recvfrom() failed: %d\n", WSAGetLastError());
                continue;
            }

            recv_buf[recv_len] = '\0'; // Null-terminate response
            printf("Server: %s", recv_buf); // Display server response

            // Check if guess is correct and handle play again
            if (strncmp(recv_buf, "CORRECT", 7) == 0) {
                char choice[10];
                printf("You guessed correctly! Do you want to play again? (y/n or e to exit): ");
                fgets(choice, sizeof(choice), stdin);
                if (choice[0] == 'n' || choice[0] == 'e' || choice[0] == 'N' || choice[0] == 'E') {
                    play = 0;
                }
                break; // Break inner loop to reset if continuing
            }
        }
    }

    // Clean up resources
    closesocket(sock);
    WSACleanup();
    return EXIT_SUCCESS;
}
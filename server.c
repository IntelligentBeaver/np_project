// server.c - Concurrent UDP Guess-the-Number Game Server with Session and Broadcast on Win

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define BUF_SIZE 512
#define MAX_CLIENTS 100

typedef struct {
    struct sockaddr_in addr;
    int target;
} ClientSession;

ClientSession clients[MAX_CLIENTS];
int client_count = 0;

// Compares sockaddr_in structs for client identity
int compare_clients(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_addr.s_addr == b->sin_addr.s_addr) &&
           (a->sin_port == b->sin_port);
}

// Finds or registers a client and returns its index
int get_or_create_client_index(struct sockaddr_in *client_addr) {
    for (int i = 0; i < client_count; ++i) {
        if (compare_clients(&clients[i].addr, client_addr)) {
            return i;
        }
    }

    if (client_count >= MAX_CLIENTS) {
        return -1;
    }

    clients[client_count].addr = *client_addr;
    clients[client_count].target = 1 + rand() % 100;
    printf("[+] New client connected: %s:%d | Target=%d\n",
           inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port),
           clients[client_count].target);
    return client_count++;
}

// Sends a message to all clients except the sender
void broadcast_message(SOCKET sock, const char *message, struct sockaddr_in *sender, int sender_len) {
    for (int i = 0; i < client_count; ++i) {
        if (!compare_clients(&clients[i].addr, sender)) {
            sendto(sock, message, (int)strlen(message), 0,
                   (struct sockaddr*)&clients[i].addr, sender_len);
        }
    }
}

// Generate target number
int generate_number() {
    return 1 + rand() % 100;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));
    printf("[Server] Listening on port %s\n", argv[1]);

    char buffer[BUF_SIZE];
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);

    while (1) {
        memset(buffer, 0, BUF_SIZE);

        int recv_len = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                                (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len == SOCKET_ERROR) {
            fprintf(stderr, "recvfrom() failed: %d\n", WSAGetLastError());
            continue;
        }

        buffer[recv_len] = '\0';

        int index = get_or_create_client_index(&client_addr);
        if (index < 0) {
            const char *msg = "Server full. Try again later.\n";
            sendto(sock, msg, (int)strlen(msg), 0, (struct sockaddr*)&client_addr, addr_len);
            continue;
        }

        int guess = atoi(buffer);
        const char *response;

        if (guess < clients[index].target) {
            response = "HIGHER\n";
        } else if (guess > clients[index].target) {
            response = "LOWER\n";
        } else {
            response = "CORRECT\n";
            printf("Client %s:%d guessed the number correctly!!!!\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Notify all other clients
            char announce[BUF_SIZE];
            snprintf(announce, BUF_SIZE, "Player from %s:%d guessed the number correctly!!!\n",
                     inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            broadcast_message(sock, announce, &client_addr, addr_len);

            // Reset target for this client
            clients[index].target = generate_number();
            printf("[*] New target for %s:%d = %d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                   clients[index].target);
        }

        // Send response to guessing client
        sendto(sock, response, (int)strlen(response), 0,
               (struct sockaddr*)&client_addr, addr_len);
    }

    closesocket(sock);
    WSACleanup();
    return EXIT_SUCCESS;
}

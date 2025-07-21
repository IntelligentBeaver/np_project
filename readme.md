# 1. Project Overview

This project uses Winsock in C to implement a basic "Guess the Number" game over UDP. After creating a random integer, a server watches for client guesses. The server replies with a hint (HIGHER, LOWER, or CORRECT) after each client submits a guess via UDP. When the client makes a correct guess, the server notifies them and changes the game's random number.

## Key features:

- UDP communication with Winsock (Windows sockets).

- Stateless server handling multiple clients via message tags.

- Simple protocol: ASCII text messages terminated by CRLF.

- Logging and error checking for robust operation.

# Prerequisites

- Windows OS with Winsock2 development libraries (included in Visual Studio or MSYS2).

- C compiler supporting Winsock (e.g., MSVC or GCC with -lws2_32).

# Build & Run Instructions

## 1. Building

Open a Command Prompt or Developer Shell and run:

`cl /W4 /Fe:server.exe server.c ws2_32.lib`

`cl /W4 /Fe:client.exe client.c ws2_32.lib`

Or with GCC:

`gcc -Wall -Wextra -o server.exe server.c -lws2_32`

`gcc -Wall -Wextra -o client.exe client.c -lws2_32`

## 2. Execution

Start the server:

`./server <port>`

`<port>`: UDP port number (e.g., 5000).

Run a client:

`./client <server_ip> <server_port>`

`<server_ip>`: IPv4 address of server (e.g., 127.0.0.1).

`<server_port>`: Must match server port.

Follow on-screen prompts to submit integer guesses.

## Protocol Definition

Client → Server: ASCII string of an integer guess (e.g., 42\r\n).

Server → Client: One of:

`HIGHER\r\n` — actual number is higher.

`LOWER\r\n` — actual number is lower.

`CORRECT\r\n` — guess is correct; game resets.

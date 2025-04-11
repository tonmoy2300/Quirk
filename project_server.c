#include <winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 4096

SOCKET clientSocket;
char username[100];

// Function to receive messages
DWORD WINAPI ReceiveMessages(LPVOID lpParam) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while (1) {
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            printf("\nDisconnected from server\n");
            break;
        }
        buffer[bytesReceived] = '\0';
        printf("\n%s\n", buffer);
    }

    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Failed to connect to server\n");
        return 1;
    }

    // Receive server's request for username
    char serverMessage[BUFFER_SIZE];
    recv(clientSocket, serverMessage, BUFFER_SIZE - 1, 0);
    printf("%s", serverMessage);

    // Get username and send it to the server
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // Remove newline
    send(clientSocket, username, strlen(username), 0);

    printf("\nConnected to chat server! Type 'quit' to exit.\n");
    printf("To ask Gemini AI a question, start your message with 'Hi Gemini'\n\n");

    // Start receiving messages
    HANDLE threadHandle = CreateThread(NULL, 0, ReceiveMessages, NULL, 0, NULL);
    if (threadHandle == NULL) {
        printf("Failed to create thread\n");
        return 1;
    }

    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "quit") == 0) {
            break;
        }

        send(clientSocket, message, strlen(message), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

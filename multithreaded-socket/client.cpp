#include <iostream>
#include <winsock2.h>
#include <string.h>

using namespace std;

#define SERVER_ADDRESS "127.0.0.1"
#define PORT 9000

// Initialize Winsock
void initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        exit(EXIT_FAILURE);
    }
}

// Main client function
int main() {
    initWinsock();

    // Create the socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Send a message to the server
    const char* message = "Hello from client!";
    if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
        cerr << "Error sending data" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Receive server response
    char buffer[1024];
    int result = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (result == SOCKET_ERROR) {
        cerr << "Error receiving data" << endl;
    } else if (result == 0) {
        // Server closed the connection gracefully
        cout << "Server closed the connection" << endl;
    } else {
        buffer[result] = '\0'; // Null-terminate the received message
        cout << "Server response: " << buffer << endl;
    }

    // Gracefully shut down the connection before closing the socket
    shutdown(clientSocket, SD_SEND);  // SD_SEND indicates that we are done sending data

    // Close the socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

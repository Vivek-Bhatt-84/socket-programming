#include <iostream>
#include <winsock2.h>
#include <thread>

#define PORT 9000
using namespace std;

void initwinsock(){
    WSADATA ws;
    int res = WSAStartup(MAKEWORD(2 ,2) ,&ws);
    if(res != 0){
        cerr << "WSAStratup failed" << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }
}




void handleClient(SOCKET client_socket) {
    char buff[1024];
    
    while (true) {
        int res = recv(client_socket, buff, sizeof(buff), 0);
        
        if (res == SOCKET_ERROR) {
            cerr << "Error receiving data from client" << endl;
            break;
        }

        if (res == 0) {
            // Client disconnected gracefully (zero bytes received)
            cout << "Client disconnected gracefully" << endl;
            break;
        }

        buff[res] = '\0'; // Null-terminate the received message
        cout << "Client message: " << buff << endl;

        // Send a response to the client
        const char* response = "Message received!";
        send(client_socket, response, strlen(response), 0);
    }

    // After the loop ends (client disconnects or error occurs), close the socket
    closesocket(client_socket);
    cout << "Client connection closed, thread exiting" << endl;
}

int main()
{

    
    initwinsock();
    
    int server_socket = socket(AF_INET ,SOCK_STREAM ,IPPROTO_TCP);
    if(server_socket == SOCKET_ERROR){
        cerr << "Socket creation failed " << WSAGetLastError() << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }


    sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(PORT);

    if(bind(server_socket ,(struct sockaddr*)&serv ,sizeof(serv)) == SOCKET_ERROR){
        cerr << " Bind failed : " << endl;
        closesocket(server_socket);
        WSACleanup(); 
        return -1;
    }


    if(listen(server_socket ,5) == SOCKET_ERROR){
        cerr << " listen failed : " << endl;
        closesocket(server_socket);
        WSACleanup(); 
        return -1;
    }

    cout << "Server Listening on port " << PORT << endl;

    // handle client 
    while(true){
        SOCKET client_socket = accept(server_socket ,NULL ,NULL);
        if(client_socket == INVALID_SOCKET){
            cerr << "Accept failed " << endl;
            continue;
        }

        thread clientThread(handleClient ,client_socket);
        clientThread.detach();
    }

    closesocket(server_socket);
    WSACleanup();


return 0;
}
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 9000

using namespace std;
struct sockaddr_in srv;

int main()
{
    int nRet = 0;
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        cerr << "startup failed with error " << wsaResult << endl;
        return 1;
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

     nRet = connect(clientSocket, (const sockaddr *)&srv, sizeof(srv));
    if (nRet == SOCKET_ERROR)
    { 
        cerr << "Connection failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        cout << "Connected to the server!" << endl;
        char buff[255] = {0 ,};
        recv(clientSocket ,buff ,255 ,0);  // receive is also a blocking call it wait until it receive something 
        cout << "Press any key to see the received message from server ." << endl;
        getchar();
        cout << buff << endl;

        cout << "Now send your message to the server " << endl;
        while(1){
            fgets(buff ,255 ,stdin);
            send(clientSocket ,buff ,256 ,0);
            cout << "press any key to get response from server " << endl;
            getchar();
            recv(clientSocket ,buff ,256 ,0);
            cout << "Now Send next message : " << endl;
        }
    }

    return 0;
}
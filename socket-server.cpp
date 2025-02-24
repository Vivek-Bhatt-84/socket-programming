#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define PORT 9000

using namespace std;

// a structure that holds server's address information such as ip and port
struct sockaddr_in srv;

// set of file desciptors used in select() call
fd_set fr, fw, fe; // contains set of file descriptors where we are going to read write and descriptors throwing exceptions
int nMaxFd;        // max file descriptors used in select() call
int nSocket;
int nArrClient[5] = {0}; // array holding client socket descriptors

// function to handle messages from client
void processNewMessage(int clientSocket)
{
    cout << "Processing new message for client :  " << clientSocket << endl;
    char buff[257] = {0}; // Declares a buffer to store the received message.
    int receive = recv(clientSocket, buff, 256, 0);
    //  Receives the message from the client.

    // If the recv() function returns a value less than or equal to 0, it means there was an error or the client disconnected.
    if (receive <= 0)
    {
        cout << "Something wrong happened . Closing the connection for client " << endl;
        closesocket(clientSocket);
        //  Loops through the nArrClient array to remove the client from the list of active clients.
        for (int i = 0; i < 5; i++)
        {
            if (nArrClient[i] == clientSocket)
            {
                nArrClient[i] = 0;
                break;
            }
        }
    }
    else
    {
        cout << "The message received from client " << buff << endl;
        // send response to the client
        send(clientSocket, "Processed your request ", 23, 0); // Sends a response to the client.
        cout << "********************************************" << endl;
    }
}

// function to handle new client connection requests and process messages
void processNewRequest()
{
    // new  connection request
    // FD_ISSET(nSocket, &fr): Checks if the server socket is ready to accept new connections
    if (FD_ISSET(nSocket, &fr))
    {
        int nLength = sizeof(struct sockaddr);
        // int nClientSocket = accept(nSocket, NULL, &nLength);: Accepts a new client connection and creates a new socket for that client.
        int nClientSocket = accept(nSocket, NULL, &nLength); // using this socket descriptor request will be processed

        //         if (nClientSocket > 0): If a valid client socket is returned, the connection is established.
        // for (i = 0; i < 5; i++): Loops through the nArrClient array to find an empty spot (0) for the new client socket.
        // send(nClientSocket, "Got the connection done successfully .", 39, 0);: Sends a success message to the new client
        if (nClientSocket > 0)
        {
            // put it into client fd_set
            int i;
            for (i = 0; i < 5; i++)
            {
                if (nArrClient[i] == 0)
                {
                    nArrClient[i] = nClientSocket;
                    send(nClientSocket, "Got the connection done successfully .", 39, 0);
                    break;
                }
                if (i == 5)
                {
                    cout << "No Space for a new Connection " << endl;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (FD_ISSET(nArrClient[i], &fr))
            {
                // got new message from client
                // recv new message and queue that for your server to fulfill the req
                processNewMessage(nArrClient[i]);
            }
        }
    }
}

int main()
{
    // Initialize winsock WSA variables -> there are variables which provide enviornment for  socket programming on windows

    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        cerr << "startup failed with error " << wsaResult << endl;
        return 1;
    }

    /****************************************************************************************************************************************** */

    // ✅ Create a socket
    /**
     * @param AF_INET      -> Address Family: IPv4 (Internet Protocol)
     * @param SOCK_STREAM  -> Type: Stream socket (TCP)
     * @param IPPROTO_TCP  -> Protocol: Transmission Control Protocol (TCP)
     *
     * The socket() function returns a descriptor for the new socket.
     */
    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (nSocket < 0)
    {
        cout << "Socket Not opened. " << endl;
    }
    else
    {
        cout << "Success !!! " << endl;
        cout << "Socket id : " << nSocket << endl;
    }
    /**************************************************************************************************************************************** */
    // initialize the enviornment for socket structure
    srv.sin_family = AF_INET;         // specify address family (IPv4)
    srv.sin_port = htons(PORT);       //  Sets the port for the server to listen on, converting it to network byte order using htons().
    srv.sin_addr.s_addr = INADDR_ANY; // Binds the socket to any available network interface on the machine.
    memset(&(srv.sin_zero), 0, 8);    // Clears out the extra 8 bytes of the sockaddr_in structure.

    /**************************************************************************************************************************************** */
    /*
        // befor binding we set blobking and non blocking behavour
        // optval == 0 means blocking and == 1  means non-blocking
        // by deafult every socket is blocking
        u_long optval = 0;
        int nRet = ioctlsocket(nSocket ,FIONBIO ,&optval);
        if(nRet != 0){
            cout << "ioctlsocket call failed " << endl;
        }
        else
        {
            cout << " ioctlsocket call passed " << endl ;
        }

    */

    /**************************************************************************************************************************************** */

    int noptVal = 0; // if 1 other server can both listen and accept new connection to same port if 0 only listen and bind 
    int noptLen = sizeof(noptVal);

    // setsockopt return 0 if successfull
    // setsockopt() : Configures the socket options, specifically allowing the address to be reused in the SO_REUSEADDR option.int nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&noptVal, noptLen);

    int nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&noptVal, noptLen);
    if (nRet != 0)
    {
        cout << "Fail to setsockopt call  " << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Success setsockopt call  !!! " << endl;
    }

    /**************************************************************************************************************************************** */
    // Binds the socket to the specified address and port, allowing the server to listen on that address.
    nRet = bind(nSocket, (sockaddr *)&srv, sizeof(sockaddr));
    if (nRet < 0)
    {
        cout << "Fail to bind the local port " << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Success in binding the local port !!! " << endl;
    }

    /**************************************************************************************************************************************** */
    // Listen to request from client ( queues the requests ).
    // Prepares the server to listen for incoming connections. It can handle up to 5 pending connections
    nRet = listen(nSocket, 5);
    if (nRet < 0)
    {
        cout << "Fail to listen the local port " << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "started listening to local port !!! " << endl;
    }

    /**************************************************************************************************************************************** */

    nMaxFd = nSocket; // set max file descriptors for select () call

    // Defines the timeout for the select() call (1 second).
    struct timeval tv; // how much time to wait until all descripters are ready
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while (1)
    { // clearing file descriptors
        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        // setting file descriptors
        // add server socket and active client sockets to respective sets
        FD_SET(nSocket, &fr);
        FD_SET(nSocket, &fe);

        for (int i = 0; i < 5; i++)
        {
            if (nArrClient[i] != 0)
            {
                FD_SET(nArrClient[i], &fr);
                FD_SET(nArrClient[i], &fe);
            }
        }

        cout << "Before select call " << fr.fd_count << endl;

        // keep waiting for new requests and proceed as per request
        // before select function returns it clears all socket descriptors set
        // select(): Waits for activity on the file descriptors (either read, write, or exception).
        nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv);

        if (nRet > 0)
        {
            // when someone connects or communicates with a message over dedicated connection
            cout << "client waiting ..." << endl;
            // process request
            // FD_ISSET() macro which helps to find if something is avaible is available at a particular socket fd fw or fe
            // if (FD_ISSET(nSocket, &fe))
            // {
            //     cout << " There is an exception " << endl;
            // }
            // if (FD_ISSET(nSocket, &fw))
            // {
            //     cout << "there is something to write " << endl;
            // }
            // if (FD_ISSET(nSocket, &fr))
            // {
            //     cout << "Ready to read" << endl;
            // }

            processNewRequest();
        }
        else if (nRet == 0)
        {
            // No connection or any communiv=cation made or u can say none of the socket descrptors are ready
            cout << "Nothing on PORT : " << PORT << endl;
        }
        else
        {
            // it failed and application must show some error message
            cout << "Failed ... " << endl;
            exit(EXIT_FAILURE);
        }

        cout << "after select call " << fr.fd_count << endl;
    }

    /**************************************************************************************************************************************** */

    return 0;
}
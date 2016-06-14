#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <iostream>

#define DEFAULT_PORT            "8765"          // Default server port
#define DEFAULT_BUFFER_LEN      4096            // Default send/recv buffer length


int main(int argc, char **argv) 
{
    WSADATA          wsaData;
    SOCKET           conn_socket = INVALID_SOCKET;
    struct addrinfo *results = NULL,
					*addrptr = NULL,
					hints;
    char            *server_name = "127.0.0.1",
					*port = DEFAULT_PORT,
					Buffer[DEFAULT_BUFFER_LEN],
					hoststr[NI_MAXHOST],
					servstr[NI_MAXSERV];
    int              retval;


    // Load Winsock
    if ((retval = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
    {
        fprintf(stderr,"WSAStartup failed with error %d\n",retval);
        WSACleanup();
        return -1;
    }

    // Resolve the server name
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol =  IPPROTO_TCP;

    retval = getaddrinfo(server_name, port, &hints, &results);
    if (retval != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %d\n", retval);
        goto cleanup;
    }
    if (results == NULL)
    {
        fprintf(stderr, "Server (%s) name could not be resolved!\n", server_name);
        goto cleanup;
    }

    addrptr = results;
    conn_socket = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
    if (conn_socket == INVALID_SOCKET)
    {
        fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
        goto cleanup;
    }
    retval = getnameinfo(
                        addrptr->ai_addr,
                        (socklen_t)addrptr->ai_addrlen,
                        hoststr,
                        NI_MAXHOST,
                        servstr,
                        NI_MAXSERV,
                        NI_NUMERICHOST | NI_NUMERICSERV
                        );
    if (retval != 0)
    {
        fprintf(stderr, "getnameinfo failed: %d\n", retval);
        goto cleanup;
    }

    printf("Client attempting connection to: %s port: %s\n", hoststr, servstr);
    retval = connect(conn_socket, addrptr->ai_addr, (int)addrptr->ai_addrlen);
    if (retval == SOCKET_ERROR)
    {
        closesocket(conn_socket);
        conn_socket = INVALID_SOCKET;
		exit(1);
    }
    freeaddrinfo(results);
    results = NULL;
    if (conn_socket == INVALID_SOCKET)
    {
        printf("Unable to establish connection...\n");
        goto cleanup;
    }
    else
        printf("Connection established...\n");


    while (1)
    {
		std::cin >> Buffer;
		int bufferLength = lstrlen(Buffer);
		Buffer[bufferLength] = '\r';
        // Send the data
        retval = send(conn_socket, Buffer, bufferLength+1, NULL);
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr,"send failed: error %d\n", WSAGetLastError());
            goto cleanup;
        }
        // Receive the data back
        retval = recv(conn_socket, Buffer, DEFAULT_BUFFER_LEN, 0);
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed: error %d\n", WSAGetLastError());
            goto cleanup;
        }
        if (retval == 0)
        {
            printf("Server closed connection\n");
            break;
        }
		Buffer[retval] = 0;
        printf("%s", Buffer);
    }

    cleanup:
    // clean up the client connection
    if (conn_socket != INVALID_SOCKET)
    {
        // Indicate no more data to send
        retval = shutdown(conn_socket, SD_SEND);
        if (retval == SOCKET_ERROR)
            fprintf(stderr, "shutdown failed: %d\n", WSAGetLastError());
        // Close the socket
        retval = closesocket(conn_socket);
        if (retval == SOCKET_ERROR)
            fprintf(stderr, "closesocket failed: %d\n", WSAGetLastError());
        conn_socket = INVALID_SOCKET;
    }
    if (results != NULL)
    {
        freeaddrinfo(results);
        results = NULL;
    }
    WSACleanup();

    return 0;
}

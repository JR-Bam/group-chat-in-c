#ifndef SOCKET_UTILS
#define SOCKET_UTILS

#include <sys/socket.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
int createTCPIPv4Socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in *createIPv4Address(char* ip, int port)
{
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_port = htons(port);
    address->sin_family = AF_INET;
    if (strlen(ip) == 0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

struct SocketUpdateArgs {
    int socketFD;
    void (*callback)(char*, int);
};

void listenForUpdates(struct SocketUpdateArgs *args) {
    if (!args) return;

    char buffer[1024];
    while (true)
    {
        ssize_t amountReceived = recv(args->socketFD, buffer, 1024, 0);

        if (amountReceived > 0){
            buffer[amountReceived] = 0; // Null terminate the buffer string

            if (args->callback)
                args->callback(buffer, args->socketFD);

            // ! Note: This does not take into account empty buffers yet
            // If message is signal
            char* signal = strtok(buffer, "@@");
            if (strcmp(signal, buffer) != 0) {
                char* name = strtok(NULL, "@@");

                if (name == NULL)
                    printf("Something's wrong with the signal format.. be a better coder.. %s\n", buffer);
                
                // Check for signal type
                if (strcmp(signal, "ENTER") == 0) 
                    printf(">> %s entered the chat!\n", name);
                else if (strcmp(signal, "EXIT") == 0)
                    printf(">> %s left the chat!\n", name);

            } else {
                printf("%s\n", buffer);
            }
        }
        if (amountReceived == 0)
            break;
    }
    close(args->socketFD);
    free(args);
}

#endif
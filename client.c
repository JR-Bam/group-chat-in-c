#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "utils.h"

void listenForUpdatesAsync(int);
void listenForUpdates(int);

int main(int argc, char** argv){
    // * Establish Connection
    int socketFD = createTCPIPv4Socket();

    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

    int result = connect(socketFD, address, sizeof(*address));

    if (result == 0) {
        printf("Connection was Successful!\n");
    } else {
        printf("Not successful..\n");
    }

    char *name = NULL;
    size_t nameSize = 0;
    printf("Type your name.\n>> ");
    ssize_t nameCount = getline(&name, &nameSize, stdin);
    name[nameCount-1] = 0; // Get rid of \n
    
    char *line = NULL;
    size_t lineSize = 0;
    printf("Type and we will send (type exit)..\n");

    listenForUpdatesAsync(socketFD);

    char buffer[1024];

    while (true)
    {
        ssize_t charCount = getline(&line, &lineSize, stdin);
        line[charCount-1] = 0; // Get rid of \n

        sprintf(buffer, "%s: %s", name, line);

        if (charCount > 0) {
            if (strcmp(line, "exit") == 0)
                break;
            
            ssize_t amountSent = send(socketFD, buffer, strlen(buffer), 0);
        }
    }

    close(socketFD);

    return 0;
}

void listenForUpdatesAsync(int socketFD)
{
    pthread_t id;
    pthread_create(&id, NULL, listenForUpdates, socketFD);
}

void listenForUpdates(int socketFD) {
    char buffer[1024];
    while (true)
    {
        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);

        if (amountReceived > 0){
            buffer[amountReceived] = 0; // Null terminate the buffer string
            printf("%s\n", buffer);

        }
        if (amountReceived == 0)
            break;
    }
    close(socketFD);
}

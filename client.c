#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "utils.h"

typedef enum ClientSignal {
    ENTER,
    EXIT
} ClientSignal;

void listenForUpdatesAsync(int);
int sendClientSignal(char*, int, ClientSignal);
void newMessageCallback(char *, int);

int main(int argc, char** argv){
    // * Establish Connection
    int socketFD = createTCPIPv4Socket();

    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

    int result = connect(socketFD, address, sizeof(*address));

    if (result == 0) {
        printf("Connection was Successful!\n");
    } else {
        printf("Not successful..\n");
        return -1;
    }

    char *name = NULL;
    size_t nameSize = 0;
    printf("Type your name.\n>> ");
    ssize_t nameCount = getline(&name, &nameSize, stdin);
    name[nameCount-1] = 0; // Get rid of \n

    sendClientSignal(name, socketFD, ENTER);
    
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

    sendClientSignal(name, socketFD, EXIT);
    close(socketFD);

    return 0;
}

void listenForUpdatesAsync(int socketFD)
{
    pthread_t id;
    struct SocketUpdateArgs *args = malloc(sizeof(struct SocketUpdateArgs));
    args->socketFD = socketFD;
    args->callback = NULL;

    pthread_create(&id, NULL, listenForUpdates, args);
}

int sendClientSignal(char* name, int socketFD, ClientSignal signal)
{
    char buffer[1024];

    switch (signal) {
    case ENTER:
        sprintf(buffer, "@@ENTER@@%s@@", name);
        break;
    case EXIT:
        sprintf(buffer, "@@EXIT@@%s@@", name);
        break;
    default:
        return -1;
    }

    return send(socketFD, buffer, strlen(buffer), 0);;
}

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "utils.h"

struct AcceptedSocket {
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket * acceptIncomingConnection (int);
void receiveAndPrintIncomingData(int);
void spawnThreadForClient(struct AcceptedSocket*);
void startAcceptingIncomingConnections(int);
void updateOtherClients(char *, int);

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketCount = 0;

int main(int argc, char** argv) {
    int serverSocketFD = createTCPIPv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("", 2000);

    // Binds to specified address (localhost)
    int result = bind(serverSocketFD, serverAddress, sizeof(*serverAddress));
    if (result == 0) {
        printf("Server socket was found successfully.\n");
    }

    // Listens for incoming connections
    int listenResult = listen(serverSocketFD, 10);

    // In separate threads
    startAcceptingIncomingConnections(serverSocketFD);

    shutdown(serverSocketFD, SHUT_RDWR);

    return 0;
}

void startAcceptingIncomingConnections(int serverSocketFD)
{
    while (true) {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketCount++] = *clientSocket;
        spawnThreadForClient(clientSocket);
    }
}

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD)
{
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddressSize);

    struct AcceptedSocket *acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0; // See accept() comments

    if (!acceptedSocket->acceptedSuccessfully) {
        acceptedSocket->error = clientSocketFD;
    }

    return acceptedSocket;
}

void spawnThreadForClient(struct AcceptedSocket * clientSocket)
{
    pthread_t id;
    pthread_create(&id, NULL, receiveAndPrintIncomingData, clientSocket->acceptedSocketFD);
}

void receiveAndPrintIncomingData(int socketFD)
{
    char buffer[1024];
    while (true)
    {
        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);

        if (amountReceived > 0){
            buffer[amountReceived] = 0; // Null terminate the buffer string
            printf("%s\n", buffer);

            updateOtherClients(buffer, socketFD);
        }
        if (amountReceived == 0)
            break;
    }
    close(socketFD);
}

void updateOtherClients(char *buffer, int socketFD)
{
    for (int i = 0; i < acceptedSocketCount; i++) {
        if (acceptedSockets[i].acceptedSocketFD != socketFD) {
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
        }
    }
}




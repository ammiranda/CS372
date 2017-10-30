/**
 * Name: Alexander Miranda
 * Assignment: Project 1 Chat client/server
 * Due Date: 10/29/2017
 * 
 * References: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
*/

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>

#define userNameLength 10
#define messageLength 500

/**
 * Returns a pointer to an address information linked list
 * with an address and port number pair
 * 
 * inputtedAddress: The inputted hostname to establish the connection
 * portNum: The port number where the chat server host is listening
 * 
 * returns: A pointer (reference) to the address info pair of address
 *  and port number
*/
struct addrinfo * generateAddressPtr(char * inputtedAddress, char * portNum) {
    int data;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    data = getaddrinfo(inputtedAddress, portNum, &hints, &res);

    if (data != 0) {
        fprintf(stderr, "Error getaddrinfo: %s\nEnter the correct host/port!\n", gai_strerror(status));
        exit(1);
    }

    return res;
}

/**
 * Generates a socket from the addrinfo pair pointer
 * 
 * res: Pointer to the address info linked list pair
 * 
 * returns: A socket descriptor
*/
int generateSocket(struct addrinfo * response) {
    int socketFileDesc = socket(response->ai_family, response->ai_socktype, response->ai_protocol);

    if (socketFileDesc == -1) {
        fprintf(stderr, "Could not generate socket\n");
        exit(1);
    }

    return socketFileDesc;
}

/**
 * Establishes the connection between chat client and chat server. Passes
 * the client's username to the chat server and passes the chat server's username
 * to the chat client
 * 
 * socketFileDesc: The socket host/port pair
 * userName: The chat client's username
 * serverName: The chat server's username
*/
void serverHandshake(int socketFileDesc, char * userName, char * serverName) {
    send(socketFileDesc, userName, strlen(userName), 0);
    recv(socketFileDesc, serverName, userNameLength, 0);
}

/**
 * Establishes a socket connection to the address passed in
 * 
 * socketFileDesc: Pointer to the host/port address information
 * response: 
*/
void establishConnection(int socketFileDesc, struct addrinfo * response) {
    int data = connect(socketFileDesc, response->ai_addr, response->ai_addrlen);

    if (data == -1) {
        fprintf(stderr, "Error establishing socket connection\n");
        exit(1);
    }
}

/**
 * Helper method that prompts for the user's name from stdin
 * 
 * input: An empty char array with a length of 10 chars
*/
void grabUserName(char * userInput) {
    printf("Please enter a username of 10 characters or less: ");
    scanf("%s", userInput);
}

/**
 * Starts the chat between only two hosts through a socket connection.
 * 
 * socketFileDesc: A socket file descriptor describing the socket connection
 * userName: Char array of the chat client's username
 * serverName: Char array of the chat server's username
*/
void chat(int socketFileDesc, char * userName, char * serverName) {
    char inputBuffer[messageLength];
    char outputBuffer[messageLength];

    // Clear both buffers
    memset(inputBuffer, 0, sizeof(inputBuffer));
    memset(outputBuffer, 0, sizeof(outputBuffer));

    int data = 0;
    int socketStatus;

    fgets(inputBuffer, messageLength, stdin);

    // Loop until break statement or SIGINT encountered
    while (1) {
        printf("%s> ", userName);
        fgets(inputBuffer, messageLength, stdin);

        if (strcmp(inputBuffer, "\\quit\n") == 0) {
            break;
        }

        data = send(socketFileDesc, inputBuffer, strlen(inputBuffer), 0);

        // Checking if data transmission failed
        if (data == -1) {
            fprintf(stderr, "Could not send data to server\n");
            exit(1);
        }

        // Saving status in a variable to check if the connection succeeded below
        socketStatus = recv(socketFileDesc, outputBuffer, messageLength, 0);

        if (socketStatus == -1) {
            fprintf(stderr, "Could not fetch data from server\n");
            exit(1);
        } else if (socketStatus == 0) {
            printf("Server closed the connection\n");
            break;
        } else {
            printf("%s> %s\n", serverName, outputBuffer);
        }

        // Re-flushing the buffers so new messages can be properly appended
        memset(inputBuffer, 0, sizeof(inputBuffer));
        memset(outputBuffer, 0, sizeof(outputBuffer));
    }

    close(socketFileDesc);
    printf("Connection has closed\n");
}

/**
 * Main routine for the chat client
 * 
 * argc: Integer that denotes how many arguments were passed to chat_client
 * argv: Char array containing the arguments passed to chat_client
 * 
 * returns: Integer denoting successful execution
*/
int main(int argc, char * argv[]) {
    // If the wrong amount of args are provided output errors to the user
    // and end the program
    if (argc != 3) {
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(1);
    }

    // Declare the char array for the chat client username
    char userName[userNameLength];
    // Accept input from user to define the client username
    grabUserName(userName);

    struct addrinfo * response = generateAddressPtr(argv[1], argv[2]);

    // Creating the socket file descriptor
    int socketFileDesc = generateSocket(response);

    establishConnection(socketFileDesc, response);

    // Initialize the server-side username char array
    char serverName[userNameLength];
    serverHandshake(socketFileDesc, userName, serverName);

    chat(socketFileDesc, userName, serverName);

    // Free the memory allocated for the address port pair
    freeaddrinfo(response);

    return 0;
}

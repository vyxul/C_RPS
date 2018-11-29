#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
//#define PORT 8080

void *chooseRPS(void *port) {
    int *port_ptr = (int *)port;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *rock = "Rock", *paper = "Paper", *scissors = "Scissors";
    
    time_t t;
    srand((unsigned) time(&t));
    int choice = rand() % 3;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error in chooseRPS\n");
        return NULL;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(*port_ptr);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported \n");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed\n");
        return NULL;
    }

    if (choice == 0)
        send(sock, rock, strlen(rock), 0);
    else if (choice == 1)
        send(sock, paper, strlen(paper), 0);
    else if (choice == 2)
        send(sock, scissors, strlen(scissors), 0);

    pthread_exit(NULL);
}

void *test(void *arg) {
    char *test = "Test message from test function. If this works then thank god.";
    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error in test \n");
        return NULL;
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported in test \n");
        return NULL;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed in test\n");
        return NULL;
    }
    printf("Sending message from test\n");
    send(sock, test, strlen(test), 0);
    printf("Test message sent!\n");

    pthread_exit(NULL);
}

int main () {
    pthread_t player_1, testThread;
    int PORT = 8080, testPort = 8081;
    int returnCode1, returnCode2;
    int server_1, socket_1;
    int server_2, socket_2;
    int opt = 1;
    char buffer[1024] = {0};
    char rock[5] = "Rock", paper[6] = "Paper", scissors[9] = "Scissors";
    char request[31] = "Choose rock, paper, or scissors";

    if ((server_1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if ((server_2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("2nd socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt for 2nd socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address, testAddr;
    int addrlen = sizeof(address), testlen = sizeof(testAddr);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    testAddr.sin_family = AF_INET;
    testAddr.sin_addr.s_addr = INADDR_ANY;
    testAddr.sin_port = htons( 8081 );

    if (bind(server_1, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (bind(server_2, (struct sockaddr *)&testAddr, sizeof(testAddr)) < 0) {
        perror("bind failed on 2nd socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_1, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", request);

    returnCode1 = pthread_create(&player_1, NULL, chooseRPS, &PORT);

    if (returnCode1) {
        printf("ERROR; return code from pthread_create() is %d\n", returnCode1);
        exit(-1);
    }

    if ((socket_1 = accept(server_1, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    read(socket_1, buffer, 1024);
    printf("Thread 1 chose: %s\n", buffer);

    if (strcmp(buffer, rock) == 0)
        printf("Paper wraps rock\n");
    else if (strcmp(buffer, paper) == 0)
        printf("Scissors cuts paper\n");
    else if (strcmp(buffer, scissors) == 0)
        printf("Rock breaks scissors\n");

    pthread_join(player_1, NULL);

    //using testThread
    if (listen(server_2, 3) < 0) {
        perror("listening to 2nd socket");
        exit(EXIT_FAILURE);
    }
    returnCode2 = pthread_create(&testThread, NULL, chooseRPS, &testPort);
    if (returnCode2) {
        printf("ERROR on 2nd thread; return code from pthread_create() is %d\n", returnCode2);
        exit(-1);
    }
    if ((socket_2 = accept(server_2, (struct sockaddr *)&testAddr, (socklen_t*)&testlen)) < 0) {
        perror("accept test");
        exit(EXIT_FAILURE);
    }
    read(socket_2, buffer, 1024);
    printf("Thread 2 sent: %s\n", buffer);
    pthread_join(testThread, NULL);

    return 0;
}

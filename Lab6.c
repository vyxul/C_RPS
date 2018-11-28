#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#define PORT 8080

void *helloclient(void *arg) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *rock     = "Rock";
    char *paper    = "Paper";
    char *scissors = "Scissors";
    char buffer[1024] = {0};
    time_t t;
    srand((unsigned) time(&t));
    int choice = rand() % 3;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error \n");
        return NULL;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported \n");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed\n");
        return NULL;
    }

    printf("Choice is: %d\n", choice);
    if (choice == 0)
        send(sock, rock, strlen(rock), 0);
    else if (choice == 1)
        send(sock, paper, strlen(paper), 0);
    else if (choice == 2)
        send(sock, scissors, strlen(scissors), 0);

    printf("Sent choice.\n");
    pthread_exit(NULL);
}

int main () {
    pthread_t thread_1, thread_2;
    int rc;
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char rock[5] = "Rock";
    char paper[6] = "Paper";
    char scissors[9] = "Scissors";
    char request[31] = "Choose rock, paper, or scissors";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", request);

    rc = pthread_create(&thread_1, NULL, helloclient, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);

    if (strcmp(buffer, rock) == 0)
        printf("Paper wraps rock\n");
    else if (strcmp(buffer, paper) == 0)
        printf("Scissors cuts paper\n");
    else if (strcmp(buffer, scissors) == 0)
        printf("Rock breaks scissors\n");

    pthread_join(thread_1, NULL);
    return 0;
}

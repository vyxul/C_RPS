#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

void *chooseRPS(void *port) {
    int *port_ptr = (int *)port;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *rock = "Rock", *paper = "Paper", *scissors = "Scissors";
    
    int choice = random() % 3;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error in chooseRPS\n");
        return NULL;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(*port_ptr);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address_1 / Address not supported \n");
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

int main () {
    pthread_t player_1, player_2;
    int port_1 = 8080, port_2 = 8081;
    int returnCode1, returnCode2;
    int server_1, socket_1;
    int server_2, socket_2;
    int opt = 1;
    char buffer[1024] = {0};
    char rock[5] = "Rock", paper[6] = "Paper", scissors[9] = "Scissors";
    int choice_1 = 0, choice_2 = 0, tie = 1;

    srand(time(NULL));

    printf("Rock, Paper, Scissors\n=====================\n\n");

    //creating socket 1
    if ((server_1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //creating socket 2
    if ((server_2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("2nd socket failed");
        exit(EXIT_FAILURE);
    }

    //setting options for socket 1
    if (setsockopt(server_1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //setting options for socket 2
    if (setsockopt(server_2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt for 2nd socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address_1, address_2;
    int addrlen_1 = sizeof(address_1), addrlen_2 = sizeof(address_2);
    address_1.sin_family = AF_INET;
    address_1.sin_addr.s_addr = INADDR_ANY;
    address_1.sin_port = htons( port_1 );
    address_2.sin_family = AF_INET;
    address_2.sin_addr.s_addr = INADDR_ANY;
    address_2.sin_port = htons( 8081 );

    //binding sockets 1 and 2 to correct address
    if (bind(server_1, (struct sockaddr *)&address_1, addrlen_1) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (bind(server_2, (struct sockaddr *)&address_2, addrlen_2) < 0) {
        perror("bind failed on 2nd socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_1, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if (listen(server_2, 3) < 0) {
        perror("listening to 2nd socket");
        exit(EXIT_FAILURE);
    }


    int round = 1;
    while (tie) {
        printf("Round %d\n", round);
        memset(buffer, 0, sizeof(buffer));

        //starting player_1
        returnCode1 = pthread_create(&player_1, NULL, chooseRPS, &port_1);

        if (returnCode1) {
            printf("ERROR; return code from pthread_create() is %d\n", returnCode1);
            exit(-1);
        }
        if ((socket_1 = accept(server_1, (struct sockaddr *)&address_1, (socklen_t*)&addrlen_1)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        read(socket_1, buffer, 1024);
        //calling player_1 back
        pthread_join(player_1, NULL);

        printf("Player 1: %s\n", buffer);

        if (strcmp(buffer, rock) == 0)
            choice_1 = 1;
        else if (strcmp(buffer, paper) == 0)
            choice_1 = 2;
        else if (strcmp(buffer, scissors) == 0)
            choice_1 = 3;
    
        //starting player_2
        memset(buffer, 0, sizeof(buffer));
        returnCode2 = pthread_create(&player_2, NULL, chooseRPS, &port_2);
        if (returnCode2) {
            printf("ERROR on 2nd thread; return code from pthread_create() is %d\n", returnCode2);
            exit(-1);
        }
        if ((socket_2 = accept(server_2, (struct sockaddr *)&address_2, (socklen_t*)&addrlen_2)) < 0) {
            perror("accept test");
            exit(EXIT_FAILURE);
        }

        read(socket_2, buffer, 1024);
        //calling player_2 back
        pthread_join(player_2, NULL);

        printf("Player 2: %s\n", buffer);

        if (strcmp(buffer, rock) == 0)
            choice_2 = 1;
        else if (strcmp(buffer, paper) == 0)
            choice_2 = 2;
        else if (strcmp(buffer, scissors) == 0)
            choice_2 = 3;
       
        //display tie or winner
        if (choice_1 == choice_2)
            printf("Tie!\n====\n\n");

        else {
            //if player_1 won
            if ((choice_1 == 1 && choice_2 == 3) || (choice_1 == 2 && choice_2 == 1) || (choice_1 == 3 && choice_2 == 2)) {
                printf("The winner is Player 1!\n\n");
                tie = 0;
            }
            //if player_2 won
            else {
                printf("The winner is Player 2!\n\n");
                tie = 0;
            }
        }

        round++;
    }

    return 0;
}

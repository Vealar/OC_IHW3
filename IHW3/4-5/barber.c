//
// Created by Александр Артемьев on 20.05.2024.
//
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile int working = 1;

void handler(int x) {
    working = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_socket;
    struct sockaddr_in server_address;
    if ((client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() error");
        exit(1);
    }
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = SA_RESTART;
    sigaction(SIGINT, &act, NULL);
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);
    if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("connect() error");
        exit(1);
    }
    int client_data[3] = {0, 0, 0}; // [type, data, optional]
    int client_id = 0;
    printf("Waiting new customers\n");
    while (working) {
        recv(client_socket, client_data, sizeof(client_data), MSG_DONTWAIT);
        if (client_data[0] == -1) {
            printf("Server dropped\n");
            exit(0);
        }
        if (client_data[0] == 0) continue;
        client_id = client_data[0];
        printf("New client in process #%d\n", client_id);
        sleep(2 + rand() % 2);
        send(client_socket, client_data, sizeof(client_data), 0);
        client_data[0] = 0;
        sleep(1);
    }
    client_data[0] = -1;
    send(client_socket, client_data, sizeof(client_data), 0);
    close(client_socket);
    exit(0);
}

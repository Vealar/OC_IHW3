//
// Created by Александр Артемьев on 20.05.2024.
//
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <number of clients>\n", argv[0]);
        exit(1);
    }
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_count = atoi(argv[3]);

    int client_socket;
    struct sockaddr_in server_address;
    if ((client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() error");
        exit(1);
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);
    if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        perror("connect() error");
        exit(1);
    }
    int client_data[3] = {0};
    int i = 0;
    while(i<client_count) {
        client_data[0] = ++i;
        printf("Customer input in barbershop #%d\n", i);
        send(client_socket, client_data, sizeof(client_data), 0);
        sleep(rand()%2+1);
    }
    client_data[1] = -1;
    send(client_socket, client_data, sizeof(client_data), 0);
    close(client_socket);
    exit(0);
}

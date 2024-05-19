//
// Created by Александр Артемьев on 20.05.2024.
//
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

static volatile int working = 1;
static volatile size_t size = 0;

void handler(int x) {
    working = 0;
}

typedef struct node {
    int val;
    struct node *next;
} node_t;

void add(node_t **head, int val) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
}
int pop(node_t **head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}
typedef struct thread_args {
    int socket;
    void (*handler)(int);
} thread_args;
node_t *queue = NULL;

void handleCustomer(int socket) {
    int client_data[3];
    while(1>0){
        recv(socket, client_data, sizeof(client_data), 0);
        if (client_data[1] == -1) {
            break;
        }
        if(size>5){
            printf("Client left, because queue is full #%d\n", client_data[0]);
        }else{
            ++size;
            printf("Client in the queue #%d\n", client_data[0]);
            add(&queue, client_data[0]);
        }
    }
    close(socket);
}


void handleBarber(int socket) {
    int client_data[3];
    client_data[0] = 0;
    while(1>0) {
        recv(socket, client_data, sizeof(client_data), MSG_DONTWAIT);
        if (client_data[0] == -1) {
            --size;
            close(socket);
            return;
        }
        if (queue == NULL) {
            sleep(1);
            continue;
        }
        client_data[0] = pop(&queue);
        send(socket, client_data, sizeof(client_data), 0);
        printf("Client #%d with barber\n", client_data[0]);
        recv(socket, client_data, sizeof(client_data), 0);
        if (client_data[0] == -1) {
            --size;
            return;
        }
        client_data[0] = 0;
        --size;
    }
}


void *serviceThread(void *args) {
    int server_socket;
    int client_socket;
    unsigned client_length;
    struct sockaddr_in client_addr;
    pthread_detach(pthread_self());
    server_socket = ((thread_args*)args)->socket;
    void (*handler)(int) = ((thread_args*)args)->handler;
    free(args);
    listen(server_socket, 10);
    while(1>0) {
        client_length = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length);
        printf("New connection from %s\n", inet_ntoa(client_addr.sin_addr));
        handler(client_socket);
    }
}

void createServiceOnPort(char* name, void(*handler)(int), unsigned short server_port) {
    pthread_t serviceThreadId;
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("socket() error");
        exit(1);
    };
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("bind() error");
        exit(1);
    }
    printf("Service '%s' is running on %s:%d\n", name, inet_ntoa(server_addr.sin_addr), server_port);
    thread_args *args = (thread_args*) malloc(sizeof(thread_args));
    args->socket = server_socket;
    args->handler = handler;
    if (pthread_create(&serviceThreadId, NULL, serviceThread, (void*) args) != 0){
        perror("pthread_create() error");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage:  %s <barber_port> <customer_port>\n", argv[0]);
        exit(1);
    }
    unsigned short barber_port = atoi(argv[1]);
    unsigned short customer_port = atoi(argv[2]);
    createServiceOnPort("Barber", handleBarber, barber_port);
    createServiceOnPort("Customer", handleCustomer, customer_port);
    while(1>0) {
        sleep(1);
    }
    return 0;
}

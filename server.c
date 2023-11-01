#include "message_handle_tools.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_REQUEST_SIZE 1024
#define PORT 8080
#define NR_OF_THREADS 30

static void *handle_connection(void *arg) {
    int client_socket = (int*) arg;

    // create buffer for request
    char request_buffer[MAX_REQUEST_SIZE];
    int request_size;

    // receive the request
    request_size = recv(client_socket, request_buffer, MAX_REQUEST_SIZE, 0);
    if (request_size == -1) {
        perror("recv");
        return NULL;
    }
    puts("Request received!");

    // parse the request
    struct message new_request;
    if (parse_request(&new_request, request_buffer, request_size) == -1) {
        fprintf(stderr, "parse_request: error");
        
        // send bad request message
        char *bad_request_response = 
        "HTTP/1.1 400 Bad Request\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 11\r\n"
        "\r\n\r\n"
        "Bad Request";

        if (send(client_socket, bad_request_response, strlen(bad_request_response), 0) == -1) {
            perror("send");
            return NULL;
        }

        message_cleanup(&new_request);
        close(client_socket);
        return NULL;
    }
    puts("Request parsed!");

    // create response
    struct message new_response;
    if (create_response(&new_request, &new_response) == -1) {
        fprintf(stderr, "create_response: error");
        message_cleanup(&new_request);
        message_cleanup(&new_response);
        close(client_socket);

        return NULL;
    }
    puts("Response created!");

    puts(new_response.line);
    puts(new_response.headers);
    puts(new_response.body);

    // 6 is the number of '\r' and '\n' characters that are needed in the response
    int response_size = strlen(new_response.line) + strlen(new_response.headers) 
                        + strlen(new_response.body) + 6;
    char response_buffer[response_size];
    sprintf(response_buffer, "%s\r\n"
                                "%s\r\n"
                                "\r\n"
                                "%s",
                                new_response.line, new_response.headers, new_response.body);
    send(client_socket, response_buffer, response_size, 0);

    message_cleanup(&new_request);
    message_cleanup(&new_response);
    close(client_socket);

    return NULL;
}

int main(void) {
    // create the socket
    int main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
    puts("Socket Created!");

    int reuse = 1;
    if (setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        return EXIT_FAILURE;
    }

    // set up port and address for the socket
    struct sockaddr_in host;
    socklen_t host_len = sizeof(host);

    host.sin_family = AF_INET;
    host.sin_port = htons(PORT);
    host.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind the socket to that port and address
    if (bind(main_socket, (struct sockaddr *) &host, host_len) == -1) {
        perror("bind");
        return EXIT_FAILURE;
    }
    puts("Socket Binded!");


    while (1) {
        // wait for connection request
        if (listen(main_socket, NR_OF_THREADS) == -1) {
            perror("listen");
            return EXIT_FAILURE;
        }

        // accept the connection request
        int client_socket = accept(main_socket, (struct sockaddr *)&host, &host_len);
        if (client_socket == -1) {
            perror("accept");
            return NULL;
        }
        puts("Connection accepted!");

        pthread_t tids[NR_OF_THREADS];
        for (int i = 0; i < NR_OF_THREADS; i++) {
            if (pthread_create(&tids[i], NULL, handle_connection, &client_socket)) {
                perror("pthread_create");
                return EXIT_FAILURE;
            }
        }

        for (int i = 0; i < NR_OF_THREADS; i++) {
            if (pthread_join(tids[i], NULL)) {
                perror("pthread_join");
                return EXIT_FAILURE;
            }
        }

    }
    close(main_socket);
    return EXIT_SUCCESS;
}
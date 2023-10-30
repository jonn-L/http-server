#include "message_handle_tools.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(void) {
    // create the socket
    int main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
    puts("Socket Created!");

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
        if (listen(main_socket, 10) == -1) {
            perror("listen");
            return EXIT_FAILURE;
        }

        // accept the connection request
        int client_socket = accept(main_socket, (struct sockaddr *)&host, &host_len);
        if (client_socket == -1) {
            perror("accept");
            return EXIT_FAILURE;
        }
        puts("Connection accepted!");

        // create buffer for request
        char request_buffer[MAX_REQUEST_SIZE];
        int request_size;

        // receive the request
        request_size = recv(client_socket, request_buffer, MAX_REQUEST_SIZE, 0);
        if (request_size == -1) {
            perror("recv");
            return EXIT_FAILURE;
        }
        puts("Request received!");

        // parse the request
        struct message new_request;
        int parsing_result = parse_request(&new_request, request_buffer, request_size);
        if (parsing_result == -1) {
            perror("parse_request");
            
            // send bad request message
            char *bad_request_response = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 11\r\n"
            "\r\n\r\n"
            "Bad Request";

            if (send(client_socket, bad_request_response, strlen(bad_request_response), 0) == -1 ) {
                perror("send");
                return EXIT_FAILURE;
            }; 
        }
        else if (parsing_result == -2) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        puts("Request parsed!");

        // create response
        struct message new_response;
        if (create_response(&new_request, &new_response) == -1) {
            perror("create_response");
        }
        puts("Response created!");

        message_cleanup(&new_request);
        message_cleanup(&new_response);
        close(client_socket);
        close(main_socket);
    }


    return EXIT_SUCCESS;
}
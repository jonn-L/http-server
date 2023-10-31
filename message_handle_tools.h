#ifndef MESSAGE_HANDLE_TOOLS
#define MESSAGE_HANDLE_TOOLS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    GET, 
    POST
} request_types;

struct message {
    char *line;
    char *headers;
    char *body;
    request_types request_type;
};

int parse_request(struct message *req, char *req_buff, int req_size);
void message_cleanup(struct message *msg);
int create_response(struct message *req, struct message *resp);

#endif
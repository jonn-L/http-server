#ifndef MESSAGE_HANDLE_TOOLS
#define MESSAGE_HANDLE_TOOLS

#include <stdlib.h>
#include <string.h>

#define MAX_REQUEST_SIZE 1024
#define PORT 8080

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

int parse_request(struct message *new_req, char *req_buff, int req_size);
void message_cleanup(struct message *msg);
int create_response(struct message *new_req, struct message *new_resp);

#endif
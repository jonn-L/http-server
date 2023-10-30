#include "message_handle_tools.h"
#include <stdio.h>

void message_cleanup(struct message *msg) {
    free(msg->line);
    free(msg->headers);
    free(msg->body);
}

int parse_request(struct message *new_req, char *req_buff, int req_size) {
    char *end_request_line = strstr(req_buff, "\r\n");
    if (end_request_line == NULL) {
        return -1;
    }
    
    int request_line_length = end_request_line - req_buff;
    new_req->line = (char *)malloc(request_line_length + 1);
    if (new_req->line == NULL) {
        return -2;
    }
    strncpy(new_req->line, req_buff, request_line_length);
    new_req->line[request_line_length] = '\0';

    if (strncmp(new_req->line, "GET", 3) == 0) {
        new_req->request_type = GET;
    }
    else {
        new_req->request_type = POST;
    }

    char *end_headers = strstr(end_request_line + 2, "\r\n\r\n");
    if (end_headers == NULL) {
        return -2;
    }

    int headers_length = end_headers - end_request_line - 2;
    new_req->headers = (char *)malloc(headers_length + 1);
    if (new_req->headers == NULL) {
        return -2;
    }
    strncpy(new_req->headers, end_request_line + 2, headers_length);
    new_req->headers[headers_length] = '\0';

    int body_length = req_size - (end_headers - req_buff) - 4;
    new_req->body = (char *)malloc(body_length + 1);
    if (new_req->body == NULL) {
        return -2;
    }
    strncpy(new_req->body, end_headers + 4, body_length);
    new_req->body[body_length] = '\0';

    return 0;
}


int create_response(struct message *new_req, struct message *new_resp) {
    puts(new_req->line);
    char *extenison = strchr(new_req->line, '.');
    puts(extenison);
    // if (new_req->request_type == GET) {

    // }
    // else if (new_req->request_type == POST) {

    // }
    return 0;
}


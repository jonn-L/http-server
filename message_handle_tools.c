#include "message_handle_tools.h"

void message_cleanup(struct message *msg) {
    free(msg->line);
    free(msg->headers);
    free(msg->body);
}

int parse_request(struct message *req, char *req_buff, int req_size) {
    // parse the request line of the request
    char *end_request_line = strstr(req_buff, "\r\n");
    if (end_request_line == NULL) {
        return -1;
    }
    int request_line_length = end_request_line - req_buff;
    req->line = (char *)malloc(request_line_length + 1);
    if (req->line == NULL) {
        perror("malloc");
        return -2;
    }
    strncpy(req->line, req_buff, request_line_length);
    req->line[request_line_length] = '\0';

    // parse the headers of the request
    if (strncmp(req->line, "GET", 3) == 0) {
        req->request_type = GET;
    }
    else {
        req->request_type = POST;
    }
    char *end_headers = strstr(end_request_line + 2, "\r\n\r\n");
    if (end_headers == NULL) {
        return -1;
    }
    int headers_length = end_headers - end_request_line - 2;
    req->headers = (char *)malloc(headers_length + 1);
    if (req->headers == NULL) {
        perror("malloc");
        return -2;
    }
    strncpy(req->headers, end_request_line + 2, headers_length);
    req->headers[headers_length] = '\0';

    // parse the body of the request
    int body_length = req_size - (end_headers - req_buff) - 4;
    req->body = (char *)malloc(body_length + 1);
    if (req->body == NULL) {
        perror("malloc");
        return -2;
    }
    strncpy(req->body, end_headers + 4, body_length);
    req->body[body_length] = '\0';

    return 0;
}

char *get_content_type(char *resource) {
    char *extension = strchr(resource, '.');

    if (extension != NULL) {
        if (strcmp(extension, ".html") == 0) {
            return "text/html";
        }
        else if (strcmp(extension, ".css") == 0) {
            return "text/css";
        }
        else if ((strcmp(extension, ".jpeg") == 0) || (strcmp(extension, ".png") == 0)) {
            return "image/jpeg";
        }
    }

    return "application/octet-stream";
}

int create_response(struct message *req, struct message *resp) {
    // extract the request-uri from the request
    char *uri_start = strchr(req->line, ' ');
    uri_start++;
    char *uri_end = strrchr(req->line, ' ');
    int uri_length = uri_end - uri_start;
    char uri[uri_length];
    strncpy(uri, uri_start, uri_length);
    uri[uri_length] = '\0';

    // extaract the requested resource
    char *resource = strrchr(uri, '/');
    resource++;
    
    if (req->request_type == GET) {
        // open the file
        char path[] = "server_resources";
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, resource);
        FILE *fp = fopen(filepath, "rb");
        if (fp == NULL) {
            perror("fopen"); // resource is not found
            return -2;
        }

        // get the length of the file
        fseek(fp, 0, SEEK_END);
        int content_length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // allocate memory for the contnent and read it
        char *content = malloc(content_length);
        if (content == NULL) {
            perror("malloc");
            fclose(fp);
            return -3;
        }
        int bytes_read = fread(content, 1, content_length, fp);
        if (bytes_read != content_length) {
            perror("fread");
            free(content);
            return -3;
        }

        // set the body for the response message
        resp->body = malloc(content_length + 1);
        strcpy(resp->body, content);
        resp->body[content_length] = '\0';
        free(content);

        // set the headers for the response message
        char *content_type = get_content_type(resource);
        int headers_length = snprintf(NULL, 0, "Content-Type: %s\r\n"
                                               "Server: jlumi/1.0\r\n"
                                               "Content-Length: %d",
                                               content_type, content_length) + 1;
        if (headers_length < 0) {
            perror("snprintf");
        }
        resp->headers = malloc(headers_length);
        if (sprintf(resp->headers, "Content-Type: %s\r\n"
                                   "Server: jlumi/1.0\r\n"
                                   "Content-Length: %d", 
                                   content_type, content_length) == -1) {
            perror("asprintf");
            return -3;
        }

        // set the line for the response message
        char *line = "HTTP/1.1 200 OK";
        resp->line = malloc(strlen(line) + 1);
        strcpy(resp->line, line);
        resp->line[strlen(line)] = '\0';
    }

    return 0;
}
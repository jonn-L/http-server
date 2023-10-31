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

char *read_resource(FILE *fp) {
    // get the length of the file
    fseek(fp, 0, SEEK_END);
    int content_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // allocate memory for the content and read it
    char *content = malloc(content_length);
    if (content == NULL) {
        perror("malloc");
        fclose(fp);
        return NULL;
    }
    if (fread(content, 1, content_length, fp) != content_length) {
        perror("fread");
        fclose(fp);
        free(content);
        return NULL;
    }
    return content;
}

int GET_response(struct message *resp, char *resource) {
    // open the file
    char path[] = "server_resources";
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", path, resource);
    FILE *fp = fopen(filepath, "rb");

    // if the resource is not found, read not_found page instead
    int isNULL = 0;
    if (fp == NULL) {
        fp = fopen("server_resources/not_found.html", "rb");
        isNULL = 1;
    }

    // read the resource and get the content
    char *body = read_resource(fp);
    if (body == NULL) {
        return -1;
    }
    fclose(fp);
    int body_length = strlen(body);

    // set the body component
    resp->body = malloc(body_length + 1);
    strcpy(resp->body, body);
    resp->body[body_length] = '\0';
    free(body);

    // find length of header component
    char *content_type = get_content_type(resource);
    int headers_length = snprintf(NULL, 0, "Content-Type: %s\r\n"
                                            "Server: jlumi/1.0\r\n"
                                            "Content-Length: %d",
                                            content_type, body_length) + 1;
    if (headers_length < 0) {
        perror("snprintf");
        return -1;
    }

    // set the header component
    resp->headers = malloc(headers_length);
    if (sprintf(resp->headers, "Content-Type: %s\r\n"
                                "Server: jlumi/1.0\r\n"
                                "Content-Length: %d", 
                                content_type, body_length) == -1) {
        perror("sprintf");
        return -1;
    }

    // set the line component
    if (isNULL == 0) {
        char *line = "HTTP/1.1 200 OK";
        resp->line = malloc(strlen(line) + 1);
        strcpy(resp->line, line);
        resp->line[strlen(line)] = '\0';
    }
    else {
        char *line = "HTTP/1.1 404 Not Found";
        resp->line = malloc(strlen(line) + 1);
        strcpy(resp->line, line);
        resp->line[strlen(line)] = '\0';
    }

    return 0;
}

int POST_response(struct message *resp, char *action) {
    if (strcmp(action, "submit-user") == 0) {

    }
    else if (strcmp(action, "create-user") == 0) {

    }
    else if (strcmp(action, "upload-file") == 0) {

    }
    else {
        // not implemented
    }

    return 0;
}

int create_response(struct message *req, struct message *resp) {
    // extract the request-uri from the request
    char *uri_start = strchr(req->line, ' ') + 1;
    char *uri_end = strrchr(req->line, ' ');
    int uri_length = uri_end - uri_start;
    char uri[uri_length + 1];
    strncpy(uri, uri_start, uri_length);
    uri[uri_length] = '\0';

    // extaract the requested resource
    char *action = strrchr(uri, '/');
    action++;
    
    if (req->request_type == GET) {
        return GET_response(resp, action);
    }
    else if (req->request_type == POST) {
        return POST_response(resp, action);
    }

    return 0;
}
#include "stratum.h"
#include "config.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <jansson.h>

int connect_stratum(const char *host, int port) {
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-] socket");
        return -1;
    }

    server = gethostbyname(host);
    if (!server) {
        fprintf(stderr, "[-] No such host: %s\n", host);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[-] connect");
        close(sockfd);
        return -1;
    }

    printf("[+] Connected to %s:%d\n", host, port);
    return sockfd;
}

void send_json(int sockfd, json_t *jmsg) {
    char *message = json_dumps(jmsg, JSON_COMPACT);
    write(sockfd, message, strlen(message));
    write(sockfd, "\n", 1); // stratum uses \n
    free(message);
}

json_t *recv_json(int sockfd) {
    static char buffer[4096];
    ssize_t len = read(sockfd, buffer, sizeof(buffer)-1);
    if (len <= 0) return NULL;
    buffer[len] = '\0';
    json_error_t error;
    return json_loads(buffer, 0, &error);
}

void stratum_subscribe(int sockfd) {
    json_t *req = json_object();
    json_object_set_new(req, "id", json_integer(1));
    json_object_set_new(req, "method", json_string("mining.subscribe"));
    json_object_set_new(req, "params", json_array());
    send_json(sockfd, req);
    json_decref(req);
}

void stratum_authorize(int sockfd, const char *user, const char *pass) {
    json_t *params = json_array();
    json_array_append_new(params, json_string(user));
    json_array_append_new(params, json_string(pass));

    json_t *req = json_object();
    json_object_set_new(req, "id", json_integer(2));
    json_object_set_new(req, "method", json_string("mining.authorize"));
    json_object_set_new(req, "params", params);
    send_json(sockfd, req);
    json_decref(req);
}

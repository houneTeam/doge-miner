#ifndef STRATUM_H
#define STRATUM_H

#include <jansson.h>

int connect_stratum(const char *host, int port);
void send_json(int sockfd, json_t *jmsg);
json_t *recv_json(int sockfd);

void stratum_subscribe(int sockfd);
void stratum_authorize(int sockfd, const char *user, const char *pass);

#endif

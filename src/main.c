// main.c — Dogecoin Stratum miner with quiet/debug toggle, hash rate, logging, and intensity control

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "config.h"
#include "stratum.h"
#include "scrypt.h"
#include "utils.h"

#include <jansson.h>

#define HEADER_SIZE 80
#define HASH_SIZE 32

int main(int argc, char *argv[]) {
    bool debug_mode = false;
    int intensity = 100; // default 100% CPU usage

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        } else if (strcmp(argv[i], "--intensity") == 0 && i + 1 < argc) {
            intensity = atoi(argv[++i]);
            if (intensity < 1) intensity = 1;
            if (intensity > 100) intensity = 100;
        }
    }

    int sockfd = connect_stratum(POOL_HOST, POOL_PORT);
    if (sockfd < 0) return 1;

    stratum_subscribe(sockfd);
    stratum_authorize(sockfd, USERNAME, PASSWORD);

    char job_id[64] = {0};
    uint8_t target_bin[HASH_SIZE];
    unsigned int accepted_shares = 0;
    uint64_t hashes_done = 0;
    uint64_t t_start = current_timestamp_ms();

    while (1) {
        json_t *resp = recv_json(sockfd);
        if (!resp || !json_is_object(resp)) {
            log_time("ERROR", "recv_json returned NULL or non-object");
            continue;
        }

        if (debug_mode) {
            char *dbg = json_dumps(resp, JSON_INDENT(2));
            printf("[debug] Received JSON:\n%s\n", dbg);
            free(dbg);
        }

        json_t *method = json_object_get(resp, "method");
        if (!method || !json_is_string(method)) {
            json_decref(resp);
            continue;
        }

        const char *m = json_string_value(method);
        if (strcmp(m, "mining.notify") == 0) {
            json_t *params = json_object_get(resp, "params");
            if (!json_is_array(params) || json_array_size(params) < 9) {
                log_time("ERROR", "Invalid mining.notify params");
                json_decref(resp);
                continue;
            }

            const char *job      = json_string_value(json_array_get(params, 0));
            const char *prevhash = json_string_value(json_array_get(params, 1));
            const char *version  = json_string_value(json_array_get(params, 5));
            const char *nbits    = json_string_value(json_array_get(params, 6));
            const char *ntime    = json_string_value(json_array_get(params, 7));

            if (!job || !prevhash || !nbits || !ntime || !version) {
                log_time("ERROR", "Null fields in notify");
                json_decref(resp);
                continue;
            }

            strncpy(job_id, job, sizeof(job_id) - 1);
            nbits_to_target(nbits, target_bin);

            if (!debug_mode) printf("[+] New job: %s\n", job_id);

            uint8_t header[HEADER_SIZE] = {0};
            hex2bin(header, version, 4);
            hex2bin(header + 4, prevhash, 32);
            hex2bin(header + 72, ntime, 4);
            hex2bin(header + 76, nbits, 4);

            uint32_t nonce = 0;
            uint8_t hash[HASH_SIZE];
            for (; nonce < MAX_NONCE; nonce++) {
                if (intensity < 100) {
                    usleep((100 - intensity) * 100); // slowdown factor
                }

                if (nonce % 100000 == 0 && !debug_mode) {
                    printf("... nonce: %u\r", nonce);
                    fflush(stdout);
                }

                memcpy(header + 80 - 4, &nonce, 4);
                scrypt_hash(header, hash);

                if (memcmp(hash, target_bin, HASH_SIZE) < 0) {
                    uint64_t elapsed = current_timestamp_ms() - t_start;
                    double khs = (hashes_done + nonce) / (elapsed / 1000.0) / 1000.0;

                    accepted_shares++;
                    printf("[\u2713] Submitted share #%u | ~%.2f kH/s\n", accepted_shares, khs);

                    char logmsg[256];
                    snprintf(logmsg, sizeof(logmsg),
                             "Share #%u accepted at %.2f kH/s", accepted_shares, khs);
                    log_to_file(logmsg);

                    json_t *params = json_array();
                    json_array_append_new(params, json_string(USERNAME));
                    json_array_append_new(params, json_string(job_id));
                    char nstr[16]; snprintf(nstr, sizeof(nstr), "%08x", nonce);
                    json_array_append_new(params, json_string(nstr));
                    json_array_append_new(params, json_string(ntime));
                    json_array_append_new(params, json_string(nbits));

                    json_t *req = json_object();
                    json_object_set_new(req, "id", json_integer(4));
                    json_object_set_new(req, "method", json_string("mining.submit"));
                    json_object_set_new(req, "params", params);

                    send_json(sockfd, req);
                    json_decref(req);
                    break;
                }
            }

            json_decref(resp);
        } else {
            json_decref(resp);
        }
    }

    close(sockfd);
    return 0;
}

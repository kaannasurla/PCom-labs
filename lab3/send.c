#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "link_emulator/lib.h"
#include "include/utils.h"
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 10000

#define ACK  0xFF
#define NACK 0x00

/* Send a frame and retry until ACK */
static void send_reliable(struct l3_msg *t, int data_len) {
    uint8_t reply;
    do {
        t->hdr.sum = 0;
        t->hdr.sum = htonl(crc32((void *)t, sizeof(t->hdr) + data_len));
        t->hdr.len = data_len;

        link_send(t, sizeof(t->hdr) + data_len);

        int r = link_recv(&reply, 1);
        if (r == 1 && reply == ACK) {
            printf("[SEND] ACK received\n");
            return;
        }
        printf("[SEND] Got NACK (frame was corrupted at receiver), retransmitting...\n");
    } while (1);
}

int main(int argc, char **argv) {
    init(HOST, PORT);

    struct l3_msg t;
    char line[256];

    printf("Available commands: REGISTER <user> <pass> | LOGIN <user> <pass> | CONNECT\n");

    while (fgets(line, sizeof(line), stdin)) {
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;

        char cmd[32], user[128], pass[128];
        int n = sscanf(line, "%31s %127s %127s", cmd, user, pass);

        if (n < 1) continue;

        memset(&t, 0, sizeof(t));

        if (strcasecmp(cmd, "REGISTER") == 0 && n == 3) {
            t.hdr.type = MSG_REGISTER;
            snprintf(t.payload, sizeof(t.payload), "%s %s", user, pass);
            int data_len = strlen(t.payload) + 1;
            send_reliable(&t, data_len);

        } else if (strcasecmp(cmd, "LOGIN") == 0 && n == 3) {
            t.hdr.type = MSG_LOGIN;
            snprintf(t.payload, sizeof(t.payload), "%s %s", user, pass);
            int data_len = strlen(t.payload) + 1;
            send_reliable(&t, data_len);

        } else if (strcasecmp(cmd, "CONNECT") == 0) {
            t.hdr.type = MSG_CONNECT;
            t.payload[0] = 0;
            int data_len = 1;
            send_reliable(&t, data_len);

        } else {
            printf("Unknown command. Available: REGISTER <user> <pass> | LOGIN <user> <pass> | CONNECT\n");
            continue;
        }

        /* Receive response from server */
        struct l3_msg resp;
        memset(&resp, 0, sizeof(resp));
        int r = link_recv(&resp, sizeof(resp));
        if (r > 0) {
            printf("[RECV] %s\n", resp.payload);
        }
    }

    return 0;
}
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"
#include "link_emulator/lib.h"
#include "include/utils.h"

#define HOST "127.0.0.1"
#define PORT 10001

#define ACK  0xFF
#define NACK 0x00

#define MAX_USERS 100

typedef struct {
    char username[128];
    char password[128];
} User;

static User users[MAX_USERS];
static int user_count = 0;
static int logged_in = 0;
static char logged_user[128];

static int find_user(const char *uname) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, uname) == 0)
            return i;
    }
    return -1;
}

int main(int argc, char **argv) {
    init(HOST, PORT);

    struct l3_msg t;
    uint8_t reply;

    printf("[RECV] Server running\n");

    while (1) {
        memset(&t, 0, sizeof(t));
        int len = link_recv(&t, sizeof(t));
        if (len <= 0) continue;
        if (len < (int)sizeof(t.hdr)) {
            reply = NACK;
            link_send(&reply, 1);
            continue;
        }

        /* Check CRC */
        uint32_t recv_sum = ntohl(t.hdr.sum);
        t.hdr.sum = 0;
        uint32_t computed = crc32((void *)&t, sizeof(t.hdr) + t.hdr.len);

        if (computed != recv_sum) {
            printf("[RECV] CRC bad, sending NACK\n");
            reply = NACK;
            link_send(&reply, 1);
            continue;
        }

        /* CRC good, send ACK first */
        reply = ACK;
        link_send(&reply, 1);

        char user[128] = {0}, pass[128] = {0};
        char response[256] = {0};

        switch (t.hdr.type) {

        case MSG_REGISTER:
            sscanf(t.payload, "%127s %127s", user, pass);
            if (find_user(user) >= 0) {
                snprintf(response, sizeof(response),
                         "User '%s' already exists.", user);
            } else if (user_count < MAX_USERS) {
                strcpy(users[user_count].username, user);
                strcpy(users[user_count].password, pass);
                user_count++;
                snprintf(response, sizeof(response),
                         "Registered successfully. You can now LOGIN.");
            }
            printf("[RECV] REGISTER ok: %d users total\n", user_count);
            printf("[RECV] %s\n", response);
            break;

        case MSG_LOGIN: {
            sscanf(t.payload, "%127s %127s", user, pass);
            int idx = find_user(user);
            if (idx < 0) {
                snprintf(response, sizeof(response),
                         "LOGIN failed: user '%s' not found", user);
                logged_in = 0;
            } else if (strcmp(users[idx].password, pass) != 0) {
                snprintf(response, sizeof(response),
                         "LOGIN failed: wrong password");
                logged_in = 0;
            } else {
                snprintf(response, sizeof(response),
                         "Login successful. You can now CONNECT.");
                logged_in = 1;
                strcpy(logged_user, user);
            }
            printf("[RECV] %s\n", response);
            break;
        }

        case MSG_CONNECT:
            if (!logged_in) {
                snprintf(response, sizeof(response),
                         "CONNECT failed: not logged in.");
            } else {
                snprintf(response, sizeof(response),
                         "Connected to the internet!");
            }
            printf("[RECV] %s\n", response);
            break;

        default:
            snprintf(response, sizeof(response), "Unknown command.");
            break;
        }

        /* Send response back as a reliable frame (no retransmit needed
           since corruption is only sender->receiver direction per spec) */
        struct l3_msg resp;
        memset(&resp, 0, sizeof(resp));
        resp.hdr.type = MSG_RESPONSE;
        strncpy(resp.payload, response, sizeof(resp.payload) - 1);
        resp.hdr.len = strlen(resp.payload) + 1;
        resp.hdr.sum = 0;
        resp.hdr.sum = htonl(crc32((void *)&resp,
                                   sizeof(resp.hdr) + resp.hdr.len));
        link_send(&resp, sizeof(resp.hdr) + resp.hdr.len);
    }

    return 0;
}
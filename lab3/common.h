#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "link_emulator/lib.h"

uint8_t simple_csum(uint8_t *buf, size_t len);
uint32_t crc32(uint8_t *buf, size_t len);

/* Message types */
#define MSG_REGISTER  1
#define MSG_LOGIN     2
#define MSG_CONNECT   3
#define MSG_RESPONSE  4

/* Layer 3 header */
struct l3_msg_hdr {
    uint8_t  type;
    uint16_t len;
    uint32_t sum;
};

/* Layer 3 frame */
struct l3_msg {
    struct l3_msg_hdr hdr;
    char payload[MAX_LEN - sizeof(struct l3_msg_hdr)];
};
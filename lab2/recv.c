#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "include/utils.h"
#include <sys/time.h>

/**
 * You can change these to communicate with another colleague.
 * There are several factors that could stop this from working over the
 * internet, but if you're on the same network it should work.
 * Just fill in their IP here and make sure that you use the same port.
 */
#define HOST "127.0.0.1"
#define PORT 10001

#include "common.h"

/* Our unique layer 2 ID */
static int ID = 123131;

/* Function which our protocol implementation will provide to the upper layer. */
int recv_frame(char *buf, int size)
{
	/* TODO 1.1: Call recv_byte() until we receive the frame start
	 * delimitator. This operation makes this function blocking until it
	 * receives a frame. */
	char c1, c2;
	c1 = recv_byte();
	c2 = recv_byte();
	
	while(!(c1 == DLE && c2 == STX)) {
		c1 = c2;
		c2 = recv_byte();
	}
	
	/* TODO 2.1: The first two 2 * sizeof(int) bytes represent sender and receiver ID */
	int i;
	char src[4];
	for (i = 0; i < 4; i++) {
		char byte = recv_byte();

		if (byte == DLE) {
			byte = recv_byte();

			if (byte == ETX)
				return i;

			else if (byte != DLE)
				return -1;
		}
		src[i] = byte;
	}
	int source;
	memcpy(&source, src, sizeof(int));

	char dst[4];
	for (i = 0; i < 4; i++) {
		char byte = recv_byte();

		if (byte == DLE) {
			byte = recv_byte();

			if (byte == ETX)
				return i;

			else if (byte != DLE)
				return -1;
		}
		dst[i] = byte;
	}
	int dest;
	memcpy(&dest, dst, sizeof(int));


	/* TODO 2.2: Check that the frame was sent to me */
	if (ID == dest)
		printf("The frame was sent to me\n");

	/* TODO 1.2: Read bytes and copy them to buff until we receive the end of the frame */
	for (i = 0; i < size; i++) {
		char byte = recv_byte();

		if (byte == DLE) {
			byte = recv_byte();

			if (byte == ETX)
				break;

			else if (byte != DLE)
				return -1;
		}
		buf[i] = byte;
	}
	
	/* If everything went well return the number of bytes received */
	int payload_len = i;

	if (payload_len < sizeof(struct timeval))
		return -1;
	struct timeval sent, now;
	memcpy(&sent, buf, sizeof(struct timeval));
	gettimeofday(&now, NULL);

	long seconds = now.tv_sec - sent.tv_sec;
	long  microsecond = now.tv_usec - sent.tv_usec;
	double elapsed = seconds + microsecond * 1e-6;

	printf("Latency: %.6f seconds, frame_size=%d bytes\n", elapsed, payload_len);
	return payload_len;
}

int main(int argc,char** argv){
	/* Don't modify this */
	init(HOST,PORT);

        // TODO remove these recives, whih are hardcoded to receive a "Hello"
	// message, and replace them with code that can receive any message.
	// char c;

	/* Wait for the start of a frame 
	char c1,c2;
	c1 = recv_byte();
	c2 = recv_byte(); */

	/* Cat timp nu am primit DLE STX citim bytes 
	while(((c1 != DLE) && (c2 != STX)) || (c1 == DLE && c2 != STX)
        || (c1 != DLE && c2 == STX)) {
		c1 = c2;
		c2 = recv_byte();
	}


	printf("%d ## %d\n",c1, c2);
	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c); */

	/* TODO 1.0: Allocate a buffer and call recv_frame */
	char buf[1024];
	int n = recv_frame(buf, sizeof(buf));
	if (n > sizeof(struct timeval)) {
		int msg_len = n - sizeof(struct timeval);
		char *msg = buf + sizeof(struct timeval);
		printf("%s\n", msg);
	}

	/* TODO 3: Measure latency in a while loop for any frame that contains
	 * a timestamp we receive, print frame_size and latency */

	printf("[RECEIVER] Finished transmission\n");
	return 0;
}

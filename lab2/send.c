#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include "include/utils.h"
#include <string.h>
#include <sys/time.h>

/**
 * You can change these to communicate with another colleague.
 * There are several factors that could stop this from working over the
 * internet, but if you're on the same network it should work.
 * Just fill in their IP here and make sure that you use the same port.
 */
#define HOST "127.0.0.1"
#define PORT 10000

/* Here we have the Frame structure */
#include "common.h"

/* Our unqiue layer 2 ID */
static int ID = 123131;

/* Function which our protocol implementation will provide to the upper layer. */
int send_frame(char *buf, int size)
{

	/* TODO 1.1: Create a new frame. */
	struct Frame frame;
	/* TODO 1.2: Copy the data from buffer to our frame structure */
	memcpy(frame.payload, buf, size);

	/* TODO 2.1: Set the destination and source */
	frame.dest = 12345; //orice identificator;
	frame.source = ID;

	/* TODO 1.3: We can cast the frame to a char *, and iterate through sizeof(struct Frame) bytes
	 calling send_bytes. */
	frame.frame_delim_start[0] = DLE;
	frame.frame_delim_start[1] = STX;
	frame.frame_delim_end[0] = DLE;
	frame.frame_delim_end[1] = ETX;
	send_byte(frame.frame_delim_start[0]);
	send_byte(frame.frame_delim_start[1]);

	char src[4];
	memcpy(src, &frame.source, sizeof(int));

	for (int i = 0; i < 4; i++) {
		if (src[i] == DLE)
			send_byte(src[i]);
		send_byte(src[i]);
	}

	char dest[4];
	memcpy(dest, &frame.dest, sizeof(int));
	for (int i = 0; i < 4; i++) {
		if (dest[i] == DLE)
			send_byte(dest[i]);
		send_byte(dest[i]);
	}

	struct timeval now;
	gettimeofday(&now, NULL);

	//dupa timestamp, punem mesajul
	int copy_size = size;
	int max_payload = sizeof(frame.payload - sizeof(struct timeval));
	if (copy_size > max_payload)
		copy_size = max_payload;
	// copiem timestamp-ul la inceputul payload-ului
	memcpy(frame.payload, &now, sizeof(struct timeval));
	memcpy(frame.payload + sizeof(struct timeval), buf, copy_size);		

	int total = sizeof(struct timeval) + copy_size;

	char *bytes = (char *)(frame.payload);

	for (int i = 0; i < total; i++) {
		char byte = bytes[i];
		if (byte == DLE)
			send_byte(byte);

		send_byte(byte);
	}

	send_byte(frame.frame_delim_end[0]);
	send_byte(frame.frame_delim_end[1]);

	/* if all went all right, return 0 */
	return 0;
}

int main(int argc,char** argv){
	// Don't touch this
	init(HOST,PORT);

	// TODO remove these sends, whih are hardcoded to send a "Hello"
	// message, and replace them with code that can send any message.
	/* Send Hello 
	send_byte(DLE);
	send_byte(STX);
	send_byte('H');
	send_byte('e');
	send_byte('l');
	send_byte('l');
	send_byte('o');
	send_byte('!');
	send_byte(DLE);
	send_byte(ETX); */

	/* TODO 1.0: Get some input in a buffer and call send_frame with it */
	char buf[1024];
	fgets(buf, sizeof(buf), stdin);
	send_frame(buf, strlen(buf));
	/* TODO 3.1: Get a timestamp of the current time copy it in the the payload */

	/* TODO 3.0: Update the maximum size of the payload in Frame to 100 (in common.h), send the frame */

	/* TODO 3.0: Update the maximum size of the payload in Frame to 300, send the frame */

	return 0;
}

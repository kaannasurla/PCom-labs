/*
Copyright 2025 University POLITEHNICA of Bucharest

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This is a single file redistibution of
https://github.com/cs-pub-ro/operating-systems/blob/main/chapters/io/ipc/drills/tasks/network-socket/solution/src/udp_socket.c
*/

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef BUFSIZ
#define BUFSIZ 256
#endif


#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(errno);                                                             \
    }                                                                          \
  } while (0)

static const char IP[] = "127.0.0.1";
static const int PORT = 5000;

/**
 * Create a sockaddr_in structure with the given IP and port.
 */
struct sockaddr_in get_sockaddr(const char *ip, const int port) {
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);

  return addr;
}

static void receiver_loop(void) {
  struct sockaddr_in addr = get_sockaddr(IP, PORT);
  char output[BUFSIZ];
  int listenfd;
  int rc;

  /* TODO 2: Create a network socket with SOCK_STREAM type. */
  // Inconsistenta in comentarii - SOCK_STREAM = TCP și SOCK_DGRAM = UDP
  listenfd = socket(AF_INET, SOCK_DGRAM, 0);
  DIE(listenfd < 0, "socket");

  /* TODO 2: Bind the socket to the addr. */
  rc = bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
  DIE(rc < 0, "bind");

  while (1) {
    memset(output, 0, BUFSIZ);

    /* TODO 2: Receive data from the connected socket */
    rc = recvfrom(listenfd, output, BUFSIZ, 0, NULL, NULL);
    DIE(rc < 0, "recv");

    if (strcmp(output, "exit") == 0)
      break;

    printf("[Receiver]: %s\n", output);
    fflush(stdout);
  }

  /* TODO 2: Close listenfd. */
  rc = close(listenfd);
  DIE(rc < 0, "close");
}

static void sender_loop(void) {
  struct sockaddr_in addr = get_sockaddr(IP, PORT);
  char input[BUFSIZ];
  int sockfd;
  int rc;

  /* TODO 2: Create a network socket with SOCK_STREAM type. */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  DIE(sockfd < 0, "socket");

  while (1) {
    memset(input, 0, BUFSIZ);
    fgets(input, BUFSIZ, stdin);
    // Remove trailing newline
    if (input[strlen(input) - 1] == '\n')
      input[strlen(input) - 1] = '\0';

    printf("[Sender]: %s\n", input);

    /* TODO 2: Send input to socket. */
    rc = sendto(sockfd, input, strlen(input), 0, (struct sockaddr *)&addr,
                sizeof(addr));
    DIE(rc < 0, "send");

    if ((strcmp(input, "exit") == 0 || strlen(input) == 0))
      break;
  }

  /* TODO 2: Close socket. */
  rc = close(sockfd);
  DIE(rc < 0, "close");
}

/**
 * Simulate a sender-receiver communication using a named pipe.
 * Run the program as a receiver by default, or as a sender if the -s or
 * --sender.
 */
int main(int argc, char *argv[]) {
  if (argc == 1) { // Run as receiver by default
    receiver_loop();
  } else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--sender") == 0) {
    sender_loop();
  } else {
    fprintf(stderr, "Usage: %s [-s|--sender]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  return 0;
}
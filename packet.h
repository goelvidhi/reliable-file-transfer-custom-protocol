#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define PAYLOAD_SIZE 1450
#define HEADER_SIZE 4
#define PACKET_SIZE 1454

typedef struct timeval timestamp;

typedef struct packet_t{
	//uint8_t type;  // 0 for DATA, 1 for ACK, 2 for NACK, 3 for NOTIFY
	int seq_num;
	//uint16_t length_payload;
	//timestamp timestamp1;
	//uint64_t file_offset;
	char payload[PAYLOAD_SIZE+1];
}packet;

#endif // PACKET_H

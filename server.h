#include "packet.h"
#define MAX_SIZE 1000000000
#define TRACK_ARRAY_LENGTH 8000000

int error;
FILE* fp_s;
char filename[50];
size_t filesize;

int start_index, last_index;
int *track_packets;
int packets_num;
unsigned int last_packet_size;
int startCallback;
int loop_index;

int total;

char *filedata;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// UDP socket variables
socklen_t fromlen;
struct sockaddr_in serv_addr, from;
int sockfd_s, portno;
int sock_options_s, sock_options_s_t;



//methods
void errorMsg(const char *msg);
void init();
void receive_file_info_udp_s();
void receive_file_info_tcp_s();
void receive_packet_s();
int updateTrackPacketsArray();
void* handleFailures(void *);
//void send_nack_s(int end_index);

int getNackSeqNum();
void send_nack_to_client(int);
int check_all_pckt_rcvd();
void send_end_s();

//threads
pthread_t nack_thread_s;


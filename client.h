#ifndef CLIENT_H
#define CLIENT_H
#include "packet.h"

#define FILENAME_SIZE 50
double timeout = 400000.0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char *filename,*data;
int fp,error,sockfd,no_of_packets,portno,off=0,seqNum=0;
struct stat statbuf;
size_t filesize;

//Socket variables
struct sockaddr_in serv_addr;
socklen_t fromlen;
struct hostent *server,*server_t;;

//Function declaration
void errorMsg(const char *msg);
void send_file_info();//use either this or void send_file_info_tcp()
void send_file_info_tcp();
void send_packets(packet);
void mapfile();
void makesocket();
void* resend_packet(void* a);

//threads
pthread_t resend_thread;
#endif

/*
 * client.c(Sender)
 * damini and vidhi
 */
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define USAGE "Usage: ./client [filename] [hostname] [portno] "

int main(int argc, char *argv[])
{
    if (argc < 4) {
       fprintf(stderr,"%s", USAGE);
       exit(0);
    }
    server_t = gethostbyname(argv[2]);
    server = gethostbyname(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    if (server_t == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    filename = (char *) malloc(FILENAME_SIZE);
    strcpy(filename,argv[1]);
    portno = atoi(argv[3]);
    makesocket();
    mapfile();
    //send_file_info();
    send_file_info_tcp();
    if((error=pthread_create(&resend_thread,NULL,resend_packet,NULL))){
        fprintf(stderr, "error in creating pthread: %s\n",strerror(error));
        exit(1);
    }
    packet packet1;
    memset(packet1.payload,'\0',PAYLOAD_SIZE+1);
    if((filesize % PAYLOAD_SIZE) != 0)
        no_of_packets = (filesize/PAYLOAD_SIZE)+1;
    else
        no_of_packets = (filesize/PAYLOAD_SIZE);
    while(seqNum < no_of_packets){
        packet1.seq_num = seqNum;
        if((seqNum == (no_of_packets-1)) && ((filesize % PAYLOAD_SIZE) != 0)){
        	memcpy(packet1.payload,data+off,(filesize % PAYLOAD_SIZE));
        	}
        else{
        	memcpy(packet1.payload,data+off,PAYLOAD_SIZE);
        	}
        seqNum++;
        memcpy(packet1.payload,data+off,PAYLOAD_SIZE);
        off = off + PAYLOAD_SIZE;
        send_packets(packet1);
    }
    pthread_join(resend_thread,NULL);
    munmap(data, filesize);
    close(fp);
	return 1;
}

void* resend_packet(void* a)
{
    while(1){
        int n,seq,size=PAYLOAD_SIZE;
        n = recvfrom(sockfd,&seq,sizeof(int),0,(struct sockaddr *)&serv_addr,&fromlen);
        if (n < 0) errorMsg("recvfrom");
        if(seq == -1){
            printf("Entire file transmitted\n");
            pthread_exit(0);
        }
        if((seq == (no_of_packets-1)) && (0 != filesize % PAYLOAD_SIZE))
            size = filesize % PAYLOAD_SIZE;
        packet packet2;
        memset(packet2.payload,'\0',PAYLOAD_SIZE+1);
        packet2.seq_num = seq;
        memcpy(packet2.payload,data+(seq*PAYLOAD_SIZE),size);
        send_packets(packet2);
    }
}

void makesocket()
{
    //create a udp socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int sock_options;
    if (sockfd < 0)
        errorMsg("ERROR opening socket");
    uint64_t sock_buffer_size = 500000000;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    fromlen=(sizeof(serv_addr));
    if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&sock_options,sizeof (int))) == -1){
        errorMsg("ERROR setting socket opt");
    }
    if((setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,&sock_buffer_size, sizeof(uint64_t))) == -1){
        errorMsg("ERROR setting socket opt");
    }
    if((setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&sock_buffer_size, sizeof(uint64_t))) == -1){
        errorMsg("ERROR setting socket opt");
    }
}

void errorMsg(const char *msg)
{
    perror(msg);
    exit(0);
}

void send_file_info_tcp(){
    int sockfd_t, n_t, portno_t = 51615;
    sockfd_t = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_t < 0)
        errorMsg("ERROR opening socket");
    struct sockaddr_in serv_addr_t;
    bzero((char *) &serv_addr_t, sizeof(serv_addr_t));
    serv_addr_t.sin_family = AF_INET;
    bcopy((char *)server_t->h_addr,(char *)&serv_addr_t.sin_addr.s_addr,server_t->h_length);
    serv_addr_t.sin_port = htons(portno_t);
    if (connect(sockfd_t,(struct sockaddr *) &serv_addr_t,sizeof(serv_addr_t)) < 0)
        errorMsg("ERROR connecting");
    n_t = write(sockfd_t,(void *)&filesize,sizeof(filesize));
    if (n_t < 0)
         errorMsg("ERROR writing to socket");
    close(sockfd_t);
}

void mapfile(){
    pthread_mutex_lock(&lock);
    if ((fp = open (filename, O_RDONLY)) < 0){
        fprintf(stderr,"can't open %s for reading", filename);
        pthread_mutex_unlock(&lock);
        exit(0);
    }
    filesize = lseek(fp, 0, SEEK_END);
    printf("Filesize is %zu\n",filesize);
    data = mmap((caddr_t)0, filesize, PROT_READ, MAP_SHARED, fp, 0);
    if (data == (caddr_t)(-1)) {
        perror("mmap");
        pthread_mutex_unlock(&lock);
        exit(0);
    }
    pthread_mutex_unlock(&lock);
}

void send_packets(packet temp_packet)
{
    usleep(100);
    int n;
    n = sendto(sockfd,&temp_packet,PACKET_SIZE, 0,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (n < 0)  errorMsg("sendto");
}
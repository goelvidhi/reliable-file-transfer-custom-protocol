/*
 * server.c(Receiver)
 * damini and vidhi
 */
#include "server.h"

#define USAGE "Usage: ./server [portno] [filename]"

int total;
void errorMsg(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    uint64_t sock_buffer_size_s = 100000000;
    if (argc < 3) {
         fprintf(stderr,"%s", USAGE);
         exit(1);
    }
    sockfd_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_s < 0)
        errorMsg("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if((setsockopt(sockfd_s,SOL_SOCKET,SO_REUSEADDR,&sock_options_s,sizeof (int))) == -1){
        errorMsg("ERROR setting socket opt");
    }
    if((setsockopt(sockfd_s,SOL_SOCKET,SO_SNDBUF,&sock_buffer_size_s, sizeof(uint64_t))) == -1){
        errorMsg("ERROR setting socket opt");
    }
    if((setsockopt(sockfd_s,SOL_SOCKET,SO_RCVBUF,&sock_buffer_size_s, sizeof(uint64_t))) == -1){
        errorMsg("ERROR setting socket opt");
    }
    if (bind(sockfd_s, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        errorMsg("ERROR on binding");
     fromlen = sizeof(from);
    receive_file_info_tcp_s();
    filedata = (char *) malloc (filesize);
    fp_s = fopen(argv[2] , "w+");
    init();
    if(filesize%PAYLOAD_SIZE != 0){
        packets_num = filesize/PAYLOAD_SIZE + 1;
        last_packet_size = filesize%PAYLOAD_SIZE;
    }
    else{
        packets_num = filesize/PAYLOAD_SIZE;
        last_packet_size = PAYLOAD_SIZE;
    }
     //thread to handle failures
    if((errno = pthread_create(&nack_thread_s, NULL, handleFailures, NULL ))){
        fprintf(stderr, "pthread_create[0] %s\n",strerror(errno));
        pthread_exit(0);
    }
    track_packets = (int *)calloc(packets_num, sizeof (int));
    receive_packet_s();
    pthread_join(nack_thread_s, NULL);
    fclose(fp_s);
    close(sockfd_s);
    return 0;
}

void receive_file_info_tcp_s()
{
    int sockfd_t, newsockfd_t, portno_t;
     socklen_t clilen_t;
     struct sockaddr_in serv_addr_t, cli_addr_t;
     int n_t;
    //printf("server :: receive_file_info_tcp_s\n");
     sockfd_t = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd_t < 0)
        errorMsg("ERROR opening socket");
    if((setsockopt(sockfd_t,SOL_SOCKET,SO_REUSEADDR,&sock_options_s_t,sizeof (int))) == -1){
        errorMsg("ERROR setting socket opt");
    }
     bzero((char *) &serv_addr_t, sizeof(serv_addr_t));
     portno_t = 51615;
     serv_addr_t.sin_family = AF_INET;
     serv_addr_t.sin_addr.s_addr = INADDR_ANY;
     serv_addr_t.sin_port = htons(portno_t);
     if (bind(sockfd_t, (struct sockaddr *) &serv_addr_t,
              sizeof(serv_addr_t)) < 0)
              errorMsg("ERROR on binding");
     listen(sockfd_t,5);
     clilen_t = sizeof(cli_addr_t);
     newsockfd_t = accept(sockfd_t,
                 (struct sockaddr *) &cli_addr_t,
                 &clilen_t);
    if (newsockfd_t < 0)
          errorMsg("ERROR on accept");
    n_t = read(newsockfd_t,&filesize,255);
    if (n_t < 0) errorMsg("ERROR reading from socket");
    close(newsockfd_t);
    close(sockfd_t);
}

void receive_packet_s()
{
    int n = 0;
    packet rcvPacket;
    long write_pos;
    while (1)
    {
        n = recvfrom(sockfd_s,&rcvPacket,1500,0,(struct sockaddr *) &from,&fromlen);
        if (n < 0) errorMsg("ERROR in recv ");
        pthread_mutex_lock(&lock);
        if (updateTrackPacketsArray(rcvPacket.seq_num)){
            track_packets[rcvPacket.seq_num] = 1;
                if(rcvPacket.seq_num > last_index)
                    last_index = rcvPacket.seq_num;
                write_pos = rcvPacket.seq_num * PAYLOAD_SIZE;
                fseek( fp_s , write_pos , SEEK_SET  );
                if(rcvPacket.seq_num == (packets_num - 1) ){
                    fwrite(&rcvPacket.payload , last_packet_size , 1 , fp_s);
                    fflush(fp_s);
                 }
                else{
                    fwrite(&rcvPacket.payload , PAYLOAD_SIZE , 1 , fp_s);
                    fflush(fp_s);
                 }
                 total ++;
        }
        pthread_mutex_unlock(&lock);
        if(check_all_pckt_rcvd() == 1){
            printf("Entire file received\n");
            send_end_s();
            break;
        }
       if(last_index >= 0.8 * packets_num){
            startCallback = 1;
       }
       }

}

int updateTrackPacketsArray(int seq_num){
    if(seq_num>= 0 && seq_num < packets_num)
    {
        if(track_packets[seq_num] == 0){
            track_packets[seq_num] = 1;
            return 1;
        }
        else return 0;

    }
    return 0;
}

void* handleFailures(void *a)
{
    while(1)
    {
        if(startCallback){
            usleep(100);
            int actual_last_index = 0;
            if(last_index > 0.7 * packets_num)
                actual_last_index = last_index + 0.3 * packets_num;
            else
                actual_last_index = last_index;
            if(check_all_pckt_rcvd() == 1){
                pthread_exit(0);
            }
            int i;
            for(i =  start_index; i<= actual_last_index && i < packets_num ; i++)
                {
                if(track_packets[i] == 1){
                start_index ++;
                }
            else break;
            }

            int reqSeqNum = getNackSeqNum();

            if(reqSeqNum >= 0 && reqSeqNum < packets_num){
            send_nack_to_client(reqSeqNum);
            }
        }
    }
}
int loop_index;

int getNackSeqNum(){

    if (track_packets == NULL) return -1;
    int i;

    for (i = loop_index; i < packets_num ; i++)
    {
        if(track_packets[i] == 0){
            if( i == packets_num - 1) loop_index = start_index;
            else loop_index = i+1;
            return i;
        }
    }
    loop_index = start_index;
    return -1;

}
void send_nack_to_client(int seqNum)
{
    int n;
    n = sendto(sockfd_s, &seqNum, sizeof(int), 0,(struct sockaddr *) &from,fromlen);
    if (n < 0) errorMsg("sendto");
}

int check_all_pckt_rcvd()
{

if(total == packets_num) return 1;
else return 0;

}

void send_end_s(){

int end_data = -1;
int n,i;

        for (i = 0 ; i < 10 ; i++){
        n = sendto(sockfd_s,&end_data,sizeof(int), 0,(struct sockaddr *) &from,fromlen);
        if (n < 0) errorMsg("sendto");
        }
        close(sockfd_s);
}

void init(){
    start_index = 0, last_index = 0, total =0, loop_index = 0;
    packets_num = 0;
    startCallback = 0;
}

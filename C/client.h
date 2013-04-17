#include <stdio.h>      //just the basics...
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>     //library for getting command line arguments..great tool!
#include <string.h> //memset
#include <sys/socket.h>    //for socket ofcourse
#include "tcp.h" //tcp header
#include "ip.h" //ip header
#define PCKT_LEN 8192

/*
  Caculate's the TCP checksum
*/
unsigned short calculate_checksum(unsigned short *buf, int len);

void fill_iphdr(struct ip* ip_header, char* source_ip, char* dest_ip);

/* 
    96 bit (12 bytes) pseudo header needed for tcp header checksum calculation 
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

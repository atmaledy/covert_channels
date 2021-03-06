//
//  server.c
//
//    The server application implementation
//
//
//    Created by Andrew Maledy on 2013-04-10.
//    COMP 8505 - Assignment 1
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>            //library for getting command line arguments..great tool!
#include <sys/socket.h> //for socket function
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> //memset
#include "tcp.h" //tcp header
#include "ip.h" //ip header
#include "server.h"
#define TH_SIN 0x06
#define END_SEGMENT 0x01 //end of segment character
#define TRUE 1
#define FALSE 0
void print_usage()
{
    printf("Usage: ./server -s <source ip (the client ip)> -p <port to listen on> ");
}
int main(int argc, char *argv[])
{
    
    char* client_ip;
    int sfd,port, option, long_index, verbose = FALSE;
    
    char packet[4096];
    
    if(getuid() != 0) {
        printf("You must be root to run this.\n");
        exit(EXIT_SUCCESS);
    }
    
    static struct option long_options[] = {
        {"client-ip",      required_argument, 0,  's' },
        {"port",      required_argument, 0,  'p' },
        {"help",      optional_argument, 0,  'h' }
    };
    
    while ((option = getopt_long (argc, argv, "s:p:hv",long_options, &long_index)) != -1)
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
            case 'v':
            verbose = TRUE;
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
    if(verbose == FALSE)
    printf("Starting server... Listening for packets from %s on port %d\n", client_ip, port);
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
    while(1){
        memset(filename, 0, sizeof(filename));
        //until the whole filename comes down the line...
        while(1)
        {
            read(sfd, &packet, sizeof(packet));
            
            //    printf("%d, %d\n", tcph->th_dport, htons(port));
            if(tcph->th_flags = TH_SIN && iph->ip_src == inaddr.s_addr && tcph->th_dport == htons(port))
            {
                
                if(iph->ip_tos == END_SEGMENT) //this is our EOF char to indicate that we are done recieving the filename
                break;
                else
                {
                    if(index < 254)
                    { //exceeded max file name length
                        
                        //printf("%X\n", iph->ip_id);
                        filename[index] = iph->ip_tos;
                        index++;
                    }
                }
            }
            
        }
        index = 0;
        FILE *file;
        if((file = fopen(filename, "wb")) == NULL)
        {
            if((file = fopen(filename, "wb+")) == NULL)
            {
                printf("Can't write the file: %s\n", filename);
                exit(EXIT_FAILURE);
            }
        }
        while(1)
        {
            //we have the filename now get the actual file
            read(sfd, &packet, sizeof(packet));
            
            
            if(tcph->th_flags = TH_SIN && iph->ip_src == inaddr.s_addr && tcph->th_dport == htons(port)) {
                if(iph->ip_tos == END_SEGMENT) //got whole file, next thing to read() is a filename (until we get another END_SEGMENT symbol.)
                break;
                else
                fputc(iph->ip_tos, file);
            }
            
        }
        fclose(file);
        if(verbose == FALSE)
        printf("Recieved file: %s \n", filename);
        
        
    }
}

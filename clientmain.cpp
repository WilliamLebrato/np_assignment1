#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG
#define BUFFER_SIZE 2000


// Included to get the support library
#include <calcLib.h>

int main(int argc, char *argv[]){

    /*
        Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
        Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
    */
    char delim[]=":";
    char *Desthost=strtok(argv[1],delim);
    char *Destport=strtok(NULL,delim);
    // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
    // *Dstport points to whatever string came after the delimiter. 

    /* Do magic */
    int port=atoi(Destport);

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char buf[BUFFER_SIZE];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    if ((rv = getaddrinfo(Desthost, Destport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return -1;
    }

    freeaddrinfo(servinfo);

    // Connect to the server
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        fprintf(stderr, "talker: failed to connect\n");
        return -1;
    }




#ifdef DEBUG 
    printf("Host %s, and port %d.\n",Desthost,port);
#endif

  
}
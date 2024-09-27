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
#define BUFFER_SIZE 128


using namespace std;


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
#ifdef DEBUG  
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

// Implement the socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket() failed");
    return -1;
  }

  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) 
  {
    perror("setsockopt() failed");
    return -1;
  }

    // Step 3: Bind the socket to an IP address and port
    struct sockaddr_in address;
    address.sin_family = AF_UNSPEC;
    address.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
    address.sin_port = htons(port); // Convert port number to network byte order

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }
    printf("Socket bound to port %d successfully.\n", port);

    // Step 4: Listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        close(sockfd);
        return -1;
    }

    // Step 5: Accept a connection
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_sockfd = accept(sockfd, (struct sockaddr *)&client_address, &client_address_len);

    if (client_sockfd < 0) {
        perror("Accept failed");
        close(sockfd);
        return -1;
    }

    const char *protocol_msg = "TEXT TCP 1.0\n\n";
    if (send(client_sockfd, protocol_msg, strlen(protocol_msg), 0) < 0) {
        perror("Send failed");
        close(client_sockfd);
        close(sockfd);
        return -1;
}

    // Read the response
    char buf[BUFFER_SIZE];
    memset(buf, 0, sizeof buf);
    int numbytes;

    if ((numbytes = recv(client_sockfd, buf, BUFFER_SIZE - 1, 0)) == -1) {
        perror("recv");
        close(client_sockfd);
        close(sockfd);
        return -1;
    }

    if (strstr(buf, "OK\n") == NULL) {
        printf("ERROR: Expected 'OK\\n' response\n");
        close(client_sockfd);
        close(sockfd);
        return -1;
    }

    

close(sockfd);
return 0;
}
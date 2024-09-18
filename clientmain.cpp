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


// gcc -o np_assignment1/clientmain np_assignment1/clientmain.cpp -I. -L. -lcalc

// ./np_assignment1/clientmain 13.53.76.30:5000

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

    #ifdef DEBUG 
    printf("Host %s, and port %d.\n",Desthost,port);
    #endif

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

    // Loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;  // If we get here, we have successfully connected
    }

    if (p == NULL) {
        fprintf(stderr, "ERROR: CANT CONNECT TO %s\n", Desthost);
        return -1;
    }

    freeaddrinfo(servinfo);


    // Receive the protocol information from the server
    memset(buf, 0, sizeof buf); // Clear the buffer
    int numbytes;
    if ((numbytes = recv(sockfd, buf, BUFFER_SIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0'; // Null-terminate the received data

    #ifdef DEBUG
        printf("Received: %s", buf);
    #endif

    // Check if the data contains one of the expected protocols
    if (strstr(buf, "TEXT TCP 1.0") != NULL || strstr(buf, "TEXT TCP 1.1") != NULL) {
        // If protocol is supported, send "OK"
        if (send(sockfd, "OK\n", 3, 0) == -1) {
            perror("send");
            exit(1);
        }
    } else {
        // If the data doesn't match any known protocol, it's unexpected (likely CHARGEN stream)
        fprintf(stderr, "ERROR\n");
        close(sockfd);
        return -1;
    }

    // After sending "OK\n" for the protocol, we now receive the next message from the server
    memset(buf, 0, sizeof buf);  // Clear the buffer
    if ((numbytes = recv(sockfd, buf, BUFFER_SIZE-1, 0)) == -1) {
        perror("recv");
        close(sockfd);
        return -1;
    }

    buf[numbytes] = '\0'; // Null-terminate the received data

    char operation[10];  // Declare a buffer for operation
    float value1, value2, result_f;

    // Scan the operation message, treating values as floats initially
    if (sscanf(buf, "%s %f %f", operation, &value1, &value2) != 3) {
        printf("ERROR: Failed to parse operation message\n");
        close(sockfd);
        return -1;
    }


    // Check if the operation is a floating-point operation (starts with 'f')
    if (operation[0] == 'f') {
        if (strcmp(operation, "fadd") == 0) {
            result_f = value1 + value2;
        } else if (strcmp(operation, "fsub") == 0) {
            result_f = value1 - value2;
        } else if (strcmp(operation, "fmul") == 0) {
            result_f = value1 * value2;
        } else if (strcmp(operation, "fdiv") == 0) {
            result_f = value1 / value2;
        } else {
            printf("ERROR: Unsupported floating-point operation\n");
            close(sockfd);
            return -1;
        }
    } else {
        // Convert floats to integers for integer operations
        int int_value1 = (int)value1;
        int int_value2 = (int)value2;

        if (strcmp(operation, "add") == 0) {
            result_f = int_value1 + int_value2;
        } else if (strcmp(operation, "sub") == 0) {
            result_f = int_value1 - int_value2;
        } else if (strcmp(operation, "mul") == 0) {
            result_f = int_value1 * int_value2;
        } else if (strcmp(operation, "div") == 0) {
            if (int_value2 == 0) {
                printf("ERROR: Division by zero\n");
                close(sockfd);
                return -1;
            }
            result_f = int_value1 / int_value2;
        } else {
            printf("ERROR: Unsupported integer operation\n");
            close(sockfd);
            return -1;
        }
    }
    // Send the result back to the server
    char result_str[50];
    snprintf(result_str, sizeof(result_str), "%8.8g\n", result_f);
    send(sockfd, result_str, strlen(result_str), 0);

  close(sockfd);
}
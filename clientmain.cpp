#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>  
#include <netdb.h>   

// gcc -o np_assignment1/clientmain np_assignment1/clientmain.cpp -I. -L. -lcalc

// ./np_assignment1/clientmain 13.53.76.30:5000


/* You will to add includes here */

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG


// Included to get the support library
#include <calcLib.h>



int main(int argc, char *argv[]){

    if (argc != 2) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return -1;
    }

    char delim[]=":";
    char *Desthost=strtok(argv[1],delim);
    char *Destport=strtok(NULL,delim);

    int port=atoi(Destport);

    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // Allow both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    getaddrinfo(Desthost, Destport, &hints, &servinfo);
    int socket_desc;

    for (p = servinfo; p != NULL; p = p->ai_next) {
        socket_desc = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        connect(socket_desc, p->ai_addr, p->ai_addrlen);
    }

    freeaddrinfo(servinfo);


    // Buffer to hold the server's response
    char server_reply[2000];
    int read_size;
    int supported_protocol = 0;
    // Receive data from server
    read_size = recv(socket_desc, server_reply, sizeof(server_reply) - 1, 0);
    if (read_size > 0) 
    {
        server_reply[read_size] = '\0'; // Null terminate the received data

        // Split the received data by newline
        char *line = strtok(server_reply, "\n");

        // Iterate over each line
        while (line != NULL) 
        {
            printf("Received Line: %s\n", line);

            // Process each line (e.g., check if it matches a supported protocol)
            if (strcmp(line, "TEXT TCP 1.0") == 0 || strcmp(line, "TEXT TCP 1.1") == 0) 
            {
                printf("Supported protocol found: %s\n", line);
                send(socket_desc, "OK\n", strlen("OK\n"), 0);
                supported_protocol = 1;
            }

            // Move to the next line
            line = strtok(NULL, "\n");
        }

        if (supported_protocol == 0) 
        {
            printf("ERROR: No supported protocol found\n");
            return -1;
        }
    } 
    else 
    {
        printf("ERROR: Issue receiving data from server\n");
    }


    // Step 2: Now receive the operation message after the protocol confirmation
    memset(server_reply, 0, sizeof(server_reply)); // Clear the buffer for new data
    read_size = recv(socket_desc, server_reply, sizeof(server_reply) - 1, 0);

    if (read_size > 0) 
    {
        server_reply[read_size] = '\0'; // Null terminate the new received data
        printf("Operation Message Received: %s\n", server_reply); // Print the operation message
    } 
    else 
    {
        printf("ERROR: Issue receiving operation message\n");
        close(socket_desc);
        return -1;
    }



    char operation[10];  // Declare a buffer for operation
    float value1, value2, result_f;

    // Scan the operation message, treating values as floats initially
    if (sscanf(server_reply, "%s %f %f", operation, &value1, &value2) != 3) {
        printf("ERROR: Failed to parse operation message\n");
        close(socket_desc);
        return -1;
    }

    printf("Operation: %s\n", operation);
    printf("Value1: %f\n", value1);
    printf("Value2: %f\n", value2);

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
            close(socket_desc);
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
                close(socket_desc);
                return -1;
            }
            result_f = int_value1 / int_value2;
        } else {
            printf("ERROR: Unsupported integer operation\n");
            close(socket_desc);
            return -1;
        }
    }

    printf("Result: %8.8g\n", result_f);  // Always print result as float

    // Send the result back to the server
    char result_str[50];
    snprintf(result_str, sizeof(result_str), "%8.8g\n", result_f);
    send(socket_desc, result_str, strlen(result_str), 0);

    // Receive server's response (OK or ERROR)
    memset(server_reply, 0, sizeof(server_reply));  // Clear the buffer again
    read_size = recv(socket_desc, server_reply, sizeof(server_reply) - 1, 0);

    if (read_size > 0) {
        server_reply[read_size] = '\0'; // Null-terminate the received data
        printf("Server Response: %s\n", server_reply);
    } else {
        printf("ERROR: Failed to receive server response\n");
    }

    close(socket_desc);
    printf("Connection closed\n");

    return 0;
}

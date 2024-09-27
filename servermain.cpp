#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h> // Required for floating-point comparison
#include <time.h>

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG
#define BUFFER_SIZE 128


using namespace std;
struct assignment {
    int int1;             // First integer value
    int int2;             // Second integer value
    float float1;         // First floating-point value
    float float2;         // Second floating-point value
    char operation[4];    // Operation (e.g., "add", "fdiv")
    int int_result;       // Result for integer operations
    float float_result;   // Result for floating-point operations
};
// Function to generate a random assignment
void generate_assignment(struct assignment *task) {
    // List of possible operations
    const char *operations[] = {"add", "sub", "mul", "div", "fadd", "fsub", "fmul", "fdiv"};
    int operation_count = 8;

    // Seed the random number generator
    srand(time(NULL));

    // Select a random operation
    int index = rand() % operation_count;
    strcpy(task->operation, operations[index]);

    // Generate random values based on the operation type
    if (task->operation[0] == 'f') {
        // Floating-point operation
        task->float1 = (float)(rand() % 1000) / 100.0;  // Random float between 0 and 10
        task->float2 = (float)(rand() % 1000) / 100.0;  // Random float between 0 and 10

        // Calculate the expected result
        if (strcmp(task->operation, "fadd") == 0) {
            task->float_result = task->float1 + task->float2;
        } else if (strcmp(task->operation, "fsub") == 0) {
            task->float_result = task->float1 - task->float2;
        } else if (strcmp(task->operation, "fmul") == 0) {
            task->float_result = task->float1 * task->float2;
        } else if (strcmp(task->operation, "fdiv") == 0) {
            task->float_result = task->float1 / task->float2;
        }
    } else {
        // Integer operation
        task->int1 = rand() % 100;  // Random integer between 0 and 99
        task->int2 = rand() % 100;  // Random integer between 0 and 99

        // Calculate the expected result
        if (strcmp(task->operation, "add") == 0) {
            task->int_result = task->int1 + task->int2;
        } else if (strcmp(task->operation, "sub") == 0) {
            task->int_result = task->int1 - task->int2;
        } else if (strcmp(task->operation, "mul") == 0) {
            task->int_result = task->int1 * task->int2;
        } else if (strcmp(task->operation, "div") == 0) {
            task->int_result = task->int1 / task->int2; // Integer division
        }
    }
}

// Function to check the client's response against the expected result
const char* check_task(struct assignment *task, char *client_response) {
    // Remove the newline character from the client's response
    client_response[strcspn(client_response, "\n")] = '\0';

    if (task->operation[0] == 'f') {
        // Floating-point comparison
        float client_result = atof(client_response); // Convert client response to float
        float difference = fabs(task->float_result - client_result); // Calculate the difference

        // If the difference is within the acceptable range, return "OK"
        if (difference < 0.0001) {
            return "OK\n";
        } else {
            return "ERROR\n";
        }
    } else {
        // Integer comparison
        int client_result = atoi(client_response); // Convert client response to int

        // Compare integer results directly
        if (task->int_result == client_result) {
            return "OK\n";
        } else {
            return "ERROR\n";
        }
    }
}

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
  int sockfd = socket(AF_UNSPEC,SOCK_STREAM, 0);
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
    while(1){
      // Step 5: Accept a connection
      struct sockaddr_in client_address;
      socklen_t client_address_len = sizeof(client_address);
      int client_sockfd = accept(sockfd, (struct sockaddr *)&client_address, &client_address_len);

      if (client_sockfd < 0) {
          perror("Accept failed");
          close(sockfd);
          return -1;
      }
        // Set a 5-second timeout for receiving data from the client
      struct timeval timeout;
      timeout.tv_sec = 5;  // 5 seconds timeout
      timeout.tv_usec = 0;

      if (setsockopt(client_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
          perror("setsockopt() for timeout failed");
          close(client_sockfd);
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

      // Generate a random assignment
      struct assignment task;
      generate_assignment(&task);

      // Prepare the assignment string
      char assignment_str[BUFFER_SIZE];
      if (task.operation[0] == 'f') {
          snprintf(assignment_str, sizeof assignment_str, "%s %8.8g %8.8g\n", task.operation, task.float1, task.float2);
      } else {
          snprintf(assignment_str, sizeof assignment_str, "%s %d %d\n", task.operation, task.int1, task.int2);
      }

      // Send the assignment to the client
      if (send(client_sockfd, assignment_str, strlen(assignment_str), 0) < 0) {
          perror("Send failed");
          close(client_sockfd);
          close(sockfd);
          return -1;
      }

      // Receive the client's response
      memset(buf, 0, sizeof buf);
      if ((numbytes = recv(client_sockfd, buf, BUFFER_SIZE - 1, 0)) == -1) {
          perror("recv");
          close(client_sockfd);
          close(sockfd);
          return -1;
      }

      // Check the client's response
      const char *response = check_task(&task, buf);

      // Send the result back to the client
      if (send(client_sockfd, response, strlen(response), 0) < 0) {
          perror("Send failed");
          close(client_sockfd);
          close(sockfd);
          return -1;
      }

      // Close the client socket
      close(client_sockfd);
    }

close(sockfd);
return 0;
}
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void Usage(char *progname);
void PrintOut(int fd, struct sockaddr *addr, size_t addrlen);
void PrintReverseDNS(struct sockaddr *addr, size_t addrlen);
void PrintServerSide(int client_fd, int sock_family);
int  Listen(char *portnum, int *sock_family);
void HandleClient(void* args);
struct handler{
  int c_fd;
  struct sockaddr *addr;
  size_t addrlen; 
  int sock_family; 
};

int 
main(int argc, char **argv) {
  struct handler h; 
  // Expect the port number as a command line argument.
  if (argc != 2) {
    Usage(argv[0]);
  }

  
  int listen_fd = Listen(argv[1], &h.sock_family);
  if (listen_fd <= 0) {
    // We failed to bind/listen to a socket.  Quit with failure.
    printf("Couldn't bind to any addresses.\n");
    return EXIT_FAILURE;
  }

  // Loop forever, accepting a connection from a client and doing
  // an echo trick to it.
  while (1) {
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    h.addrlen = (size_t)caddr_len;
    h.addr = (struct sockaddr *)(&caddr); 
    h.c_fd = accept(listen_fd,
                           (struct sockaddr *)(&caddr),
                           &caddr_len);
    if (h.c_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
        continue;
      printf("Failure on accept:%s \n ", strerror(errno));
      break;
    }
    pthread_t handle;
    pthread_create(&handle, NULL, (void*)HandleClient, &h);
    pthread_join(handle, NULL); 

    //HandleClient(client_fd,
                 //(struct sockaddr *)(&caddr),
                 //caddr_len,
                 //sock_family);
  }

  // Close socket
  close(listen_fd);
  return EXIT_SUCCESS;
}

void Usage(char *progname) {
  printf("usage: %s port \n", progname);
  exit(EXIT_FAILURE);
}

void PrintOut(int fd, struct sockaddr *addr, size_t addrlen) {
  printf("Socket [%d] is bound to: \n", fd);
  if (addr->sa_family == AF_INET) {
    // Print out the IPV4 address and port

    char astring[INET_ADDRSTRLEN];
    struct sockaddr_in *in4 = (struct sockaddr_in *)(addr);
    inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
    printf(" IPv4 address %s", astring);
    printf(" and port %d\n", ntohs(in4->sin_port));

  } else if (addr->sa_family == AF_INET6) {
    // Print out the IPV6 address and port

    char astring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)(addr);
    inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    printf("IPv6 address %s", astring);
    printf(" and port %d\n", ntohs(in6->sin6_port));

  } else {
    printf(" ???? address and port ???? \n");
  }
}

void PrintReverseDNS(struct sockaddr *addr, size_t addrlen) {
  char hostname[1024];  // ought to be big enough.
  if (getnameinfo(addr, addrlen, hostname, 1024, NULL, 0, 0) != 0) {
    sprintf(hostname, "[reverse DNS failed]");
  }
  printf("DNS name: %s \n", hostname);
}

void PrintServerSide(int client_fd, int sock_family) {
  char hname[1024];
  hname[0] = '\0';

  printf("Server side interface is ");
  if (sock_family == AF_INET) {
    // The server is using an IPv4 address.
    struct sockaddr_in srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
    inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
    printf("%s", addrbuf);
    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    getnameinfo((const struct sockaddr *) &srvr,
                srvrlen, hname, 1024, NULL, 0, 0);
    printf(" [%s]\n", hname);
  } else {
    // The server is using an IPv6 address.
    struct sockaddr_in6 srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET6_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
    inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    printf("%s", addrbuf);
    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    getnameinfo((const struct sockaddr *) &srvr,
                srvrlen, hname, 1024, NULL, 0, 0);
    printf(" [%s]\n", hname);
  }
}

int Listen(char *portnum, int *sock_family) {

  // Populate the "hints" addrinfo structure for getaddrinfo().
  // ("man addrinfo")
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv6 (also handles IPv4 clients)
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use wildcard "in6addr_any" address
  hints.ai_flags |= AI_V4MAPPED;    // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  // Use argv[1] as the string representation of our portnumber to
  // pass in to getaddrinfo().  getaddrinfo() returns a list of
  // address structures via the output parameter "result".
  struct addrinfo *result;
  int res = getaddrinfo(NULL, portnum, &hints, &result);

  // Did addrinfo() fail?
  if (res != 0) {
	printf( "getaddrinfo failed: %s", gai_strerror(res));
    return -1;
  }

  // Loop through the returned address structures until we are able
  // to create a socket and bind to one.  The address structures are
  // linked in a list through the "ai_next" field of result.
  int listen_fd = -1;
  struct addrinfo *rp;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    listen_fd = socket(rp->ai_family,
                       rp->ai_socktype,
                       rp->ai_protocol);
    if (listen_fd == -1) {
      // Creating this socket failed.  So, loop to the next returned
      // result and try again.
      printf("socket() failed:%s \n ", strerror(errno));
      listen_fd = -1;
      continue;
    }

    // Configure the socket; we're setting a socket "option."  In
    // particular, we set "SO_REUSEADDR", which tells the TCP stack
    // so make the port we bind to available again as soon as we
    // exit, rather than waiting for a few tens of seconds to recycle it.
    int optval = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof(optval));

    // Try binding the socket to the address and port number returned
    // by getaddrinfo().
    if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Bind worked!  Print out the information about what
      // we bound to.
      PrintOut(listen_fd, rp->ai_addr, rp->ai_addrlen);

      // Return to the caller the address family.
      *sock_family = rp->ai_family;
      break;
    }

    // The bind failed.  Close the socket, then loop back around and
    // try the next address/port returned by getaddrinfo().
    close(listen_fd);
    listen_fd = -1;
  }

  // Free the structure returned by getaddrinfo().
  freeaddrinfo(result);

  // If we failed to bind, return failure.
  if (listen_fd == -1)
    return listen_fd;

  // Success. Tell the OS that we want this to be a listening socket.
  if (listen(listen_fd, SOMAXCONN) != 0) {
    printf("Failed to mark socket as listening:%s \n ", strerror(errno));
    close(listen_fd);
    return -1;
  }

  // Return to the client the listening file descriptor.
  return listen_fd;
}

void HandleClient(void* args) {
  struct handler *ha = (struct handler*)args;
  FILE *ptr; 
  // Print out information about the client.
  printf("\nNew client connection \n" );
  PrintOut(ha->c_fd, ha->addr, ha->addrlen);
  PrintReverseDNS(ha->addr, ha->addrlen);
  PrintServerSide(ha->c_fd, ha->sock_family);

  // Loop, reading data and echo'ing it back, until the client
  // closes the connection.
  char delimit[] = "-";
  char buff[256];
  ptr = fopen("database.txt", "a+"); 
  while (1) {
    bzero(buff, 256);
    read(ha->c_fd, buff, sizeof(buff));
    printf("From client: %s", buff);
    //char ten[100] = "1 Rohan 6666";  
    char *getinfo = strtok(buff, delimit);
    printf("CHoice: %s", buff);
    printf("Get info: %s\n", getinfo);
    //getinfo = strtok(NULL, delimit);
    printf("getinfo2: %s\n", getinfo); 
    if(buff[0] == '1'){
      printf("In choice 1\n " ); 
      fseek(ptr, 0, SEEK_END);
      getinfo = strtok(NULL, delimit);
      char *information1 = malloc(sizeof(getinfo) *10); 
      strcpy(information1, getinfo);
      if(fputs(information1, ptr) != feof(ptr)){
        char success[] = "Put successful";
        write(ha->c_fd, success, sizeof(success));
      }
      char *information2 = malloc(sizeof(getinfo) * 10);
      getinfo = strtok(NULL, delimit);
      strcpy(information2, getinfo);
      if(fputs(information2, ptr) != feof(ptr)){
        char success[] = "Put successful";
        write(ha->c_fd, success, sizeof(success));
      }
      char success[] = "Put failed";
      write(ha->c_fd, success, sizeof(success)); 
    }
    else if(buff[0] == '2'){
      FILE *f;
      f = fopen("database.txt", "r+"); 
      char *line = NULL; 
      size_t len = 0;
      getinfo = strtok(NULL, delimit);
      printf("ID is %s\n", getinfo); 
      printf("In choice 2\n");
      while(getline(&line, &len, f) != -1){
        printf("I am in while loop\n");
        printf("This is getinfo: %s\n", getinfo); 
        if(strcmp(getinfo, line) == 0){
          printf("I am in if statement\n");
          getline(&line, &len, f);
          printf("This is line %s\n", line); 
          write(ha->c_fd, line, sizeof(line));
          break; 
        }
      }
    }
    else if(buff[0] == '3'){
      int ctr = 0, counter = 0, id = 0;
      char *line = NULL;
      size_t len = 0; 
      FILE *fptr1, *fptr2;
      char str[256], temp[] = "tempdatabase.txt";
      fptr1 = fopen("database.txt", "r");
      if(!fptr1){
        char success[] = "Unable to open file";
        write(ha->c_fd, success, sizeof(success)); 
      }
      fptr2 = fopen(temp, "w");
      if(!fptr2){
        char success[] = "Unable to open a temporary file to write!!\n"; 
        fclose(fptr1);
        write(ha->c_fd, success, sizeof(success)); 
      }
      getinfo = strtok(NULL, delimit); 
      while(getline(&line, &len, fptr1) != -1){
        counter++; 
        if(strcmp(getinfo, line) == 0){
          counter++; 
          break; 
        }
      }
      id = counter--;
      while(!feof(fptr1)){
        strcpy(str, "\0");
        fgets(str, 256, fptr1);
        if(!feof(fptr1)){
          ctr++;
          if(ctr != id){
            fprintf(fptr2, "%s", str); 
          }
          else if(ctr != counter){
            fprintf(fptr2, "%s", str); 
          }
        }
      }
      char success[] = "Delete success";
      write(ha->c_fd, success, sizeof(success)); 
      fclose(fptr1);
      fclose(fptr2);
      remove("database.txt");
      rename(temp, "database.txt"); 
    }
    else if(buff[0] == '0'){
      char su[] = "Disconnected";
      write(ha->c_fd, su, sizeof(su));
      exit(EXIT_SUCCESS); 
    }

  }

  close(ha->c_fd);
}




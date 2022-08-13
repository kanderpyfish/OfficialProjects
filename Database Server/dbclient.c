#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define BUF 256
void Usage(char *progname);

int LookupName(char *name, unsigned short port, struct sockaddr_storage *ret_addr, size_t *ret_addrlen);

int Connect(const struct sockaddr_storage *addr, const size_t addrlen, int *ret_fd);

void Communication(int socketfd){
	char comm[BUF];
	int n;  
	while(1){
		bzero(comm, sizeof(comm));
		printf("Enter your choice (1 to put, 2 to get, 3 to delete, 0 to quit): ");
		n = 0;
		while((comm[n++] = getchar()) != '\n');
		if(comm[0] == '1'){
			comm[n++] = '-'; 
			printf("Enter ID: ");
			while((comm[n++] = getchar()) != '\n');
			comm[n++] = '-'; 
			printf("Enter Name: ");
			while((comm[n++] = getchar()) != '\n');
			write(socketfd, comm, sizeof(comm));
			read(socketfd, comm, sizeof(comm));
			printf("%s\n", comm);
		}
		else if(comm[0] == '2'){
			comm[n++] = '-';
			printf("Enter ID: ");
			while((comm[n++] = getchar()) != '\n');
			write(socketfd, comm, sizeof(comm));
			read(socketfd, comm, sizeof(comm));
			printf("%s\n", comm); 
		}
		else if(comm[0] == '3'){
			comm[n++] = '-';
			printf("Enter ID: ");
			while((comm[n++] = getchar()) != '\n');
			write(socketfd, comm, sizeof(comm));
			read(socketfd, comm, sizeof(comm));
			printf("%s\n", comm); 
		}
		else if(comm[0] == '0'){
			write(socketfd, comm, sizeof(comm)); 
			read(socketfd, comm, sizeof(comm));
			printf("%s\n", comm); 
			break;
		}
	}
}

int main(int argc, char **argv){
	if(argc != 3){
		Usage(argv[0]); 
	}
	unsigned short port = 0;
	if(sscanf(argv[2], "%hu", &port) != 1){
		Usage(argv[0]);
	}
	struct sockaddr_storage addr;
	size_t addlren;
	if(!LookupName(argv[1], port, &addr, &addlren)){
		Usage(argv[0]);
	}
	int socket_fd;
	if(!Connect(&addr, addlren, &socket_fd)){
		Usage(argv[0]);
	}
	Communication(socket_fd);
	close(socket_fd); 

}
int Connect(const struct sockaddr_storage *addr, const size_t addrlen, int *ret_fd){
	int sock = socket(addr->ss_family, SOCK_STREAM, 0);
	if(sock == -1){
		printf("Socket failed to create: %s", strerror(errno));
		return 0; 
	}
	int con = connect(sock, (const struct sockaddr *)(addr), addrlen);
	if(con == -1){
		printf("Connection failed: %s", strerror(errno));
		return 0; 
	}
	*ret_fd = sock;
	return 1; 
}
void Usage(char *progname){
	printf("usage: %s hostname port \n", progname);
	exit(EXIT_FAILURE); 
}
int LookupName(char *name, unsigned short port, struct sockaddr_storage *ret_addr, size_t *ret_addrlen){
	struct addrinfo hints, *results;
	int rtrval;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if((rtrval = getaddrinfo(name, NULL, &hints, &results)) != 0){
		printf("getaddrinfo failed: %s", gai_strerror(rtrval));
		return 0; 
	}
	if(results->ai_family == AF_INET){
		struct sockaddr_in *v4addr = (struct sockaddr_in *)(results->ai_addr);
		v4addr->sin_port = htons(port);
	}
	else{
		printf("getaddrinfo failed to provide an IPv4 or IPv6 address \n");
		freeaddrinfo(results);
		return 0; 
	}
	assert(results != NULL);
	memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
	*ret_addrlen = results->ai_addrlen; 
	freeaddrinfo(results);
	return 1; 
}

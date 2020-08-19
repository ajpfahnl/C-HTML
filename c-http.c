#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <poll.h>

#define BUFSIZE 4096

char pounds[] = "###############################################\n";
char usage[] = "usage: ./html_retrieve GET --portnum=<#> --host=<host>\n";

// user-defined
char * http_request = NULL;
char * address;
int portnum;
int verbose = 0;

// runtime defined
static int sockfd;
static struct sockaddr_in serv_addr;
struct hostent * server;
struct pollfd fds[1];

void openTCP()
{
    // create new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
        exit(2);
    }
    
    // get network host entry
    server = gethostbyname(address);
    if (server == NULL) {
        fprintf(stderr, "Invalid host: %s\n", hstrerror(h_errno));
        exit(1);
    }
    
    // set fields in serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portnum);
    
    // connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error initiating connection on socket: %s\n", strerror(errno));
        exit(2);
    }
}

void setpoll()
{
    fds[0].fd = sockfd;
    fds[0].events = POLLIN | POLLHUP | POLLERR;
}

void request() {
	char * header;
	header = malloc(strlen(address)+32);
	sprintf(header, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", address);
	if (verbose) {
		printf("%s", header);
	}
	int bytes = send(sockfd, header, strlen(header), 0);
	if (bytes == -1) {
		fprintf(stderr, "Error sending HTTP request: %s\n", strerror(errno));
		exit(1);
	}
	char * buf[BUFSIZE];
	
	while(1) {
		if (fds[0].events & POLLIN) {
			int rcount = read(sockfd, buf, sizeof(buf));
			if (verbose) {
				printf("%s### read %d bytes of data in buf\n%s", pounds, rcount, pounds);
			}

			int wcount = write(1, buf, rcount);
			if (wcount == -1) {
				fprintf(stderr, "Error writing: %s\n", strerror(errno));
				exit(1);
			}
		}

		if (fds[0].events & (POLLHUP | POLLERR)) { break; }
	}
}


static struct option longopts[] = 
{
    {"host", required_argument, NULL, 'h'},
    {"portnum", required_argument, NULL, 'p'},
	{"verbose", no_argument, NULL, 'v'},
	{NULL,0,NULL,0}
};

void getopts(int argc, char * argv[]) {	
	int host_set=0, port_set=0;
	int opt;
	while ((opt = getopt_long(argc, argv, "h:p:v", longopts, NULL)) != -1) {
		switch(opt) {
			case 'h':
				address = optarg;
				host_set = 1;
				break;
			case 'p':
				portnum = atoi(optarg);	
				port_set = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				fprintf(stderr, "%s", usage);
				exit(1);
		}
	}

	if (!(host_set && port_set)) {
		fprintf(stderr, "%s", usage);
		exit(1);
	}
    
	int firstarg = 1;
    int nonswitch_i;
    for(nonswitch_i=optind; nonswitch_i<argc; nonswitch_i++)
    {
        if (firstarg == 0)
        {
            fprintf(stderr, "%s", usage);
            exit(1);
        }
        http_request = argv[nonswitch_i];
        firstarg = 0;
    }
	
	if (http_request == NULL) {
		fprintf(stderr, "%s", usage);
		exit(1);
	}
}

int main(int argc, char * argv[]) {
	getopts(argc, argv);
	if (verbose) {
		printf("HTTP method: %s\n", http_request);
	}
	openTCP();
	if (verbose) {
		printf("%s### Connected!\n%s", pounds, pounds);
	}
	setpoll();
	request();
	if (verbose) {
		printf("%s### Program exiting...\n%s", pounds, pounds);
	}
}

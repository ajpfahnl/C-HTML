#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include "common.h"

#define BUFSIZE 4096

char pounds[] = "###############################################\n";

// user-defined
char * http_request = NULL;
char * address;
int portnum = -1;
char * path = NULL;
int verbose = 0;

// runtime-defined
static int sockfd;

// TCP or TLS...
int useTLS = 1;
SSL_CTX * new_context;
SSL * ssl_client;

void usage_error() {
	char usage[] = "usage: ./html_retrieve GET --host=<host> [--portnum=<#>] [--path=<path>] [--verbose] [--useTCP]\n";
	fprintf(stderr, "%s", usage);
	exit(1);
}

void check_method() {
	if(strcmp(http_request, "GET")) {}
	else if(strcmp(http_request, "POST")) {}
	else {
		usage_error();
	}
}

struct html_end {
	int content_len;
	int chunked; // 0 or 1
};

struct html_end check_msg_end_condition(char * header) {
	struct html_end end_cond;
	end_cond.content_len = -1;
	end_cond.chunked = 0;
	
	char * content_len_str = "Content-Length: ";
	// char * chunked_str = "Transfer-Encoding: Chunked";
	
	int header_len = strlen(header), i = 0;
	for(;i<header_len; i++) {
		if (strncmp(&header[i], "\r\n", 2) == 0) {
			i += 2;
			if (strncmp(&header[i], content_len_str, strlen(content_len_str)) == 0) {
				i += strlen(content_len_str);
				end_cond.content_len = atoi(&header[i]);
				if (verbose)
					printf("%s### Will read %d bytes\n", pounds, end_cond.content_len);
				break;
			}
		}
	}
	return end_cond;
}

void htmlGET() {
	struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN | POLLHUP | POLLERR;
	
	char * header;
	header = malloc(strlen(address)+50);
	sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, address);
	if (verbose) {
		printf("%s", header);
	}
	
	if (useTLS)
		SSL_write_wrap(ssl_client, header, strlen(header));
	else
		write_wrap(sockfd, header, strlen(header), "HTTP header");
	
	char buf[BUFSIZE];
	int rcount;
	int firstpass = 1;
	int use_content_len = 1;
	struct html_end end_cond;
	while(1) {
		// check for disconnection
        if (poll(fds, 1, -1) == -1)
        {
            fprintf(stderr, "Error polling: %s\n", strerror(errno));
            exit(2);
        }
		if (fds[0].revents & (POLLHUP | POLLERR)) {
			break;
		}
		
		// read
		if (useTLS)
			rcount = SSL_read_wrap(ssl_client, buf, sizeof(buf));
		else
			rcount = read_wrap(sockfd, buf, sizeof(buf), "from socket");
		
		// check header
		if (firstpass) {
			end_cond = check_msg_end_condition(buf);
			if (end_cond.content_len == -1) {
				use_content_len = 0;
			}
			int i; // header length
			for(i = 0; i < rcount; i++) {
				if (strncmp(&buf[i], "\r\n\r\n", 4) == 0) {
					i += 4;
					end_cond.content_len -= (rcount - i);
					if (verbose)
						printf("%s### Header length is %d bytes\n", pounds, i);
					break;
				}
			}
			firstpass = 0;
		}
		// check data read
		else if (use_content_len) {
			end_cond.content_len -= rcount;
		}
		
		if (verbose) {
			printf("%s### read %d bytes of data into buffer\n%s", pounds, rcount, pounds);
		}
		write_wrap(1, buf, rcount, "to stdout");
		
		// end conditions
		if (use_content_len && (end_cond.content_len <= 0)) {
			if (verbose)
				printf("%s### Reached end of message\n", pounds);
			break;
		}
		else if ((rcount > 5) && (strncmp(&buf[rcount-5], "0\r\n\r\n", 5) == 0)) {
			if (verbose)
				printf("%s### Reached end of message\n", pounds);
			break;
		}
	}
}


static struct option longopts[] = 
{
    {"host", required_argument, NULL, 'h'},
    {"portnum", required_argument, NULL, 'n'},
	{"verbose", no_argument, NULL, 'v'},
	{"path", required_argument, NULL, 'p'},
	{"useTCP", no_argument, NULL, 't'},
	{NULL,0,NULL,0}
};

void getopts(int argc, char * argv[]) {	
	int host_set=0;
	int opt;
	while ((opt = getopt_long(argc, argv, "h:n:v:p", longopts, NULL)) != -1) {
		switch(opt) {
			case 'h':
				address = optarg;
				host_set = 1;
				break;
			case 'n':
				portnum = atoi(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'p':
				path = optarg;
				break;
			case 't':
				useTLS = 0;
				break;
			default:
				usage_error();
		}
	}

	if (!(host_set)) {
		usage_error();
	}
	
	if (useTLS && (portnum < 0)) {
		portnum = 443; // HTTPS
	}
	else if (portnum < 0) {
		portnum = 80; // HTTP
	}
	
	if (path == NULL) {
		path = malloc(2);
		strcpy(path, "/");
	}
    
	int firstarg = 1;
    int nonswitch_i;
    for(nonswitch_i=optind; nonswitch_i<argc; nonswitch_i++)
    {
        if (firstarg == 0) {
            usage_error();
        }
        http_request = argv[nonswitch_i];
        firstarg = 0;
    }
	
	if (http_request == NULL) {
		usage_error();
	}
}

int main(int argc, char * argv[]) {
	getopts(argc, argv);
	check_method();
	if (verbose) {
		printf("HTTP method: %s\n", http_request);
	}
	
	if (useTLS) {
		SSL_objects * ssl_params = openTLS(address, portnum, verbose);
		new_context = ssl_params->new_context;
		ssl_client = ssl_params->ssl_client;
		sockfd = ssl_params->sockfd;
	}
	else {
		sockfd = openTCP(address, portnum);
	}
	
	if (verbose) {
		printf("%s### Connected!\n%s", pounds, pounds);
	}
	
	htmlGET();
	if (verbose) {
		printf("%s### Program exiting...\n%s", pounds, pounds);
	}
}

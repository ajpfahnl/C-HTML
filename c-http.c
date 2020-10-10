#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include <openssl/x509.h>
#include "common.h"
#include <time.h>

#define BUFSIZE 4096
#define SAVESIZE 20

void usage_help();
void usage_error();
void check_method();
struct html_end {
	int content_len;
	int chunked; // 0 or 1
};
struct html_end check_msg_end_condition(char * header);
void htmlGET();
void htmlresponse();
void getopts(int argc, char * argv[]);

// user-defined
int profile = 0;
int profile_set = 0;
char * url = NULL;
char * http_request = NULL;
char * address;
int portnum = -1;
char * path = "/";
int verbose = 0;
char * msgPOST = "";
char * otherHeader = "cache-control: max-age=0\r\n"
					"user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36\r\n"
					"accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
					"sec-fetch-site: same-origin\r\n"
					"sec-fetch-mode: navigate\r\n"
					"sec-fetch-user: ?1\r\n"
					"sec-fetch-dest: document\r\n"
					"accept-language: en-US,en;q=0.9";

					

// runtime-defined
static int sockfd;

// TCP or TLS...
int useTLS = 1;
SSL_CTX * new_context;
SSL * ssl_client;

// for performance measurement
int rcount_min;
int rcount_max;


char *pounds = "###############################################\n";
char usage[] = "usage: ./c-http [GET/POST] --host=<host> [--port=<#>] [--path=<path>] [--verbose] [--useTCP] [--msg=<string>]\n"
               "       ./c-http --url=<url> --profile=<n>\n";

void usage_help() {
    fprintf(stderr, "%s", usage);
    exit(0);
}

void usage_error() {
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
	char * header;
	header = malloc(strlen(address) + strlen(otherHeader) + 50);
    if (header == NULL) {
        fprintf(stderr, "Error with malloc(): %s\n", strerror(errno));
        exit(1);
    }
	sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n", path, address, otherHeader);
	if (verbose) {
		printf("%s### Sending request...\n%s", pounds, pounds);
		printf("%s", header);
	}
	
	if (useTLS)
		SSL_write_wrap(ssl_client, header, strlen(header));
	else
		write_wrap(sockfd, header, strlen(header), "HTTP header");
	htmlresponse();
	free(header);
}

void htmlPOST(char * msg) {
	char * header;
	header = malloc(strlen(address)+strlen(otherHeader)+strlen(msg)+100);
    if (header == NULL) {
        fprintf(stderr, "Error with malloc(): %s\n", strerror(errno));
        exit(1);
    }
	sprintf(header,
			"POST %s HTTP/1.1\r\nHost: %s\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: %lu\r\n"
			"%s\r\n\r\n%s",
			path, address, strlen(msg), otherHeader, msg);
	if (verbose) {
		printf("%s### Sending request...\n%s", pounds, pounds);
		printf("%s\n", header);
	}
	
	if (useTLS)
		SSL_write_wrap(ssl_client, header, strlen(header));
	else
		write_wrap(sockfd, header, strlen(header), "HTTP header");
	htmlresponse();
	free(header);
}

void htmlresponse() {
	struct pollfd fds[1];			// polling for connection issues
    fds[0].fd = sockfd;
    fds[0].events = POLLHUP | POLLERR | POLLIN;
	
	char buf[BUFSIZE]; 				// Buffer to read in data from socket
	char lastbuf[SAVESIZE*2+1];		// Buffer to store last few bytes for message end check -
									// 		Sometimes the last read only takes in a couple bytes,
									// 		so we need to know what the last few bytes in the
									//		previous buffer were to determine if the message
									// 		end condition has been met
	int rcount, lastbufcount = 0;
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
        
        // performance metrics
        if ((rcount_min == -1) || (rcount < rcount_min)) {
            rcount_min = rcount;
        }
        if (rcount > rcount_max) {
            rcount_max = rcount;
        }
		
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
						printf("### Header length is %d bytes\n%s", i, pounds);
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
			printf("\n%s### read %d bytes of data into buffer\n%s", pounds, rcount, pounds);
		}
		write_wrap(1, buf, rcount, "to stdout");
		
		// end conditions
		if (use_content_len && (end_cond.content_len <= 0)) {
			if (verbose)
				printf("%s### Reached end of message\n", pounds);
			break;
		}
		else {
			int last_bytes_to_check = SAVESIZE; // will never be larger than SAVESIZE * 2
			if (rcount >= SAVESIZE) {
				memcpy(lastbuf, &buf[rcount-SAVESIZE], SAVESIZE);
			}
			else {
				memcpy(&lastbuf[lastbufcount], buf, rcount);
				last_bytes_to_check = lastbufcount + rcount;
			}
			
			int j = last_bytes_to_check-1;
			int chunk_end = 0;
			for (; j >= 0; j--) {
				if (strncmp(&lastbuf[j], "0\r\n\r\n", 5) == 0) {
					chunk_end = 1;
				}
			}
			if (chunk_end) {
				if (verbose)
					printf("%s### Reached end of message\n", pounds);
				break;
			}
		}
		
		// copy the last SAVESIZE bytes or less to lastbuf
		if (rcount >= SAVESIZE) {
			lastbufcount = SAVESIZE;
		}
		else {
			lastbufcount = rcount;
		}
		memcpy(lastbuf, &buf[rcount-lastbufcount], lastbufcount);
	}
}


static struct option longopts[] = 
{
    {"host", required_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'n'},
	{"verbose", no_argument, NULL, 'v'},
	{"path", required_argument, NULL, 'p'},
	{"useTCP", no_argument, NULL, 't'},
	{"msg", required_argument, NULL, 'm'},
    {"url", required_argument, NULL, 'u'},
    {"help", no_argument, NULL, 'x'},
    {"profile", required_argument, NULL, 'r'},
	{NULL,0,NULL,0}
};

void getopts(int argc, char * argv[]) {	
	int host_set=0;
	int opt;
	while ((opt = getopt_long(argc, argv, "h:n:v:p:m:", longopts, NULL)) != -1) {
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
			case 'm':
				msgPOST = optarg;
				break;
            case 'u':
                url = optarg;
                break;
            case 'x':
                usage_help();
                break;
            case 'r':
                profile = atoi(optarg);
                profile_set = 1;
                break;
			default:
				usage_error();
		}
	}
	if (!(host_set) && (url == NULL)) {
		usage_error();
	}
	
    // parse url if option specified
    if (url) {
        int it = 5+3;
        if (strncmp("https", url, 5) != 0) {
            useTLS = 0;
            it = 4+3;
        }
        char * url_host = malloc(strlen(url));
        char * url_path = malloc(strlen(url));
        if ((url_path == NULL) || (url_host == NULL)) {
            fprintf(stderr, "Error with malloc(): %s\n", strerror(errno));
            exit(1);
        }
        int url_it = 0;
        while (url[it] != '/') {
            url_host[url_it] = url[it];
            url_it++;
            it++;
        }
        url_it = 0;
        while (url[it] != '\0') {
            url_path[url_it] = url[it];
            url_it++;
            it++;
        }
        address = url_host;
        path = url_path;
    }
    
	if (useTLS && (portnum < 0)) {
		portnum = 443; // HTTPS
	}
	else if (portnum < 0) {
		portnum = 80; // HTTP
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
        http_request = "GET";
	}
}

// for use with finding the median
int cmpfunc (const void * a, const void * b) {
   return ( *(double*)a - *(double*)b );
}

int main(int argc, char * argv[]) {
	// setup
	getopts(argc, argv);
	check_method();
	if (verbose) {
		printf("HTTP method: %s\n", http_request);
	}
	
	// connection
	SSL_objects * ssl_params;
	if (useTLS) {
		ssl_params = openTLS(address, portnum, verbose);
		new_context = ssl_params->new_context;
		ssl_client = ssl_params->ssl_client;
		sockfd = ssl_params->sockfd;
	}
	else {
		sockfd = openTCP(address, portnum);
	}
	if (verbose) {
		if (useTLS) {
			printf("%s### Connected! Displaying certificate subject data...\n%s", pounds, pounds);
			FILE* fout = stdout;
			X509_NAME_print_ex_fp(fout, ssl_params->certname, 0, 0);
			printf("\n");
		}
		else{
			printf("%s### Connected!\n%s", pounds, pounds);
		}
	}
	
    // requesting
    if (profile > 0) {
        // perform GET request with metrics on
        int requests = profile;
        double * request_time = malloc(sizeof(double) * requests);
        // int * request_successes = malloc(sizeof(int) * requests); /* still TODO */
        if (request_time == NULL) {
            fprintf(stderr, "Error with malloc(): %s\n", strerror(errno));
            exit(1);
        }
        double max_time = -1;
        double min_time = -1;
        double mean = 0;
        int rcount_min_g = -1;
        int rcount_max_g = 0;
        clock_t start;
        int i;
        for (i=0; i<requests; i++) {
            rcount_min = -1;
            rcount_max = 0;
            start = clock();
            
            htmlGET();
            
            request_time[i] = ((double) (clock() - start)) / CLOCKS_PER_SEC;
            if ((max_time == -1) || (request_time[i] > max_time)) {
                max_time = request_time[i];
            }
            if ((min_time == -1) || (request_time[i] < min_time)) {
                min_time = request_time[i];
            }
            if ((rcount_min_g == -1) || (rcount_min < rcount_min_g)) {
                rcount_min_g = rcount_min;
            }
            if (rcount_max > rcount_max_g) {
                rcount_max_g = rcount_max;
            }
            mean += request_time[i];
        }
        mean /= requests;
        
        // find median
        qsort(request_time, requests, sizeof(double), cmpfunc);
        int n = (requests) / 2 - 1;
        double median = request_time[n];
        if ((requests % 2 != 0) && (requests > 2)) {
            median = (median + request_time[n+1])/2;
        }
        
        printf("\n>> Profile:\n"
               "   number of requests: %d\n"
               "   fastest time (s): %f\n"
               "   slowest time (s): %f\n"
               "   mean time: %f\n"
               "   median time: %f\n"
               "   percentage of successes: 100\n" /* still TODO */
               "   error codes: normal\n" /* still TODO */
               "   size of smallest response: %d\n"
               "   size of largest response: %d\n",
               requests, min_time, max_time, mean, median, rcount_min_g, rcount_max_g);
    }
    else {
        // perform request
        if (strcmp(http_request, "GET") == 0) {
            htmlGET();
        }
        if (strcmp(http_request, "POST") == 0) {
            htmlPOST(msgPOST);
        }
    }
	// end
	if (verbose) {
		printf("### Program exiting...\n%s", pounds);
	}
	
	// free things
	if (useTLS) {
		SSL_free(ssl_client);
		X509_free(ssl_params->cert);
		SSL_CTX_free(ssl_params->new_context);
	}
	close(sockfd);
}

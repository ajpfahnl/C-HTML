#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <openssl/ssl.h>

char *pounds = "###############################################\n";

int openTCP(char * address, int portnum)
{
	char portnumstr[256];
	int sockfd, err;
	struct addrinfo hints, *res, *res0;
	
	sprintf(portnumstr, "%d", portnum);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((err = getaddrinfo(address, portnumstr, &hints, &res0))) {
		fprintf(stderr, "Error with getaddrinfo(): %s", gai_strerror(err));
		exit(2);
	}
	const char * cause = NULL;
	sockfd = -1;
	for (res = res0; res; res = res->ai_next) {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			cause = "socket";
			continue;
		}
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
			cause = "connect";
			close(sockfd);
			sockfd = -1;
			continue;
		}
		break;  /* okay we got one */
	}
	if (sockfd < 0) {
		fprintf(stderr, "No connection: %s", cause);
	}
	freeaddrinfo(res0);
	
	return sockfd;
}

SSL_objects *  openTLS(char * address, int portnum, int verbose)
{
	SSL_CTX * new_context;
	SSL * ssl_client;
	int sockfd, err;
	
	X509 *cert = NULL;
	X509_NAME *certname = NULL;
	
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
	SSL_library_init();
    
    // create context
    new_context = SSL_CTX_new(SSLv23_client_method());
    if (new_context == NULL) {
        fprintf(stderr, "Error with SSL_CTX_new()\n");
        exit(2);
    }
	SSL_CTX_set_options(new_context, SSL_OP_NO_SSLv2);
    
    // create struct to hold connection data
    ssl_client = SSL_new(new_context);
    if (ssl_client == NULL)
    {
        fprintf(stderr, "Error with SSL_new()\n");
        exit(2);
    }
	
	// TCP connection
	sockfd = openTCP(address, portnum);
	if (verbose) printf("TCP connection established\n");
	
    // SSL connection
    if (SSL_set_fd(ssl_client, sockfd) == 0)
    {
        fprintf(stderr, "Error setting SSL socket fd\n");
        exit(2);
    }
	
	if (SSL_set_tlsext_host_name(ssl_client, address) == 0) {
		// note this is necessary for Server Name Indication (SNI)
		fprintf(stderr, "Error with SSL_set_tlsext_host_name()\n");
		exit(2);
	}
	
    if ((err = SSL_connect(ssl_client)) != 1)
    {
		err = SSL_get_error(ssl_client, err);
        fprintf(stderr, "Error with SSL_connect(): %d\n", err);
        exit(2);
    }
	if (verbose) printf("SSL/TLS session established\n");
	
	// get certificate
	if ((cert = SSL_get_peer_certificate(ssl_client)) == NULL) {
		fprintf(stderr, "Error retrieving certificate\n");
		exit(2);
	}
	
	certname = X509_NAME_new();
	certname = X509_get_subject_name(cert);
	
	// combine into object
	SSL_objects * params = malloc(sizeof(SSL_objects));
	params->new_context = new_context;
	params->ssl_client = ssl_client;
	params->certname = certname;
	params->cert = cert;
	return params;
}

int read_wrap(int fildes, void *buf, size_t nbyte, char * msg) {
	int rcount = read(fildes, buf, nbyte);
	if (rcount == -1) {
		fprintf(stderr, "Error reading (%s): %s\n", msg, strerror(errno));
		exit(1);
	}
	return rcount;
}

int write_wrap(int fildes, void *buf, size_t nbyte, char * msg) {
	int wcount = write(fildes, buf, nbyte);
	if (wcount == -1) {
		fprintf(stderr, "Error writing (%s): %s\n", msg, strerror(errno));
		exit(1);
	}
	return wcount;
}

int SSL_write_wrap(SSL *ssl, const void *buf, int num)
{
    int wb;
    if ((wb = SSL_write(ssl, buf, num)) <= 0)
    {
        fprintf(stderr, "Error with SSL_write()\n");
        exit(2);
    }
    return wb;
}
int SSL_read_wrap(SSL *ssl, void *buf, int num)
{
    int rb;
    if ((rb = SSL_read(ssl, buf, num)) <= 0)
    {
        fprintf(stderr, "Error with SSL_read()\n");
        exit(2);
    }
    return rb;
}

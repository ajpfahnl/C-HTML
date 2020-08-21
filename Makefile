default:
	gcc -o c-http c-http.c common.c -Wall -Wextra -lssl -lcrypto

clean:
	rm c-http

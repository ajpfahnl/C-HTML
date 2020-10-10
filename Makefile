default:
	gcc -o c-http c-http.c commonio.c -Wall -Wextra -lssl -lcrypto

clean:
	rm c-http

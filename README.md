# C-HTTP

A C program that performs HTTP requests. The program checks for `Content-Length: <#>`, and assumes `Transfer-Encoding: Chunked` otherwise to determine the end of the message body. Polling is also implemented to check if the connection closes or errors out.

By default, the program will try to connect with OpenSSL. To only use TCP, specify `--useTCP`.

Simply compile with `make` and then run the program as follows
```
./html_retrieve GET --host=<host> [--portnum=<#>] [--path=<path>] [--verbose] [--useTCP]
```
**This is currently a WIP. Only GET is currently supported.**

For example, running
```
./c-http GET --host www.berkshirehathaway.com --path /message.html --useTCP --verbose
```
will return: 
```
HTTP method: GET
###############################################
### Connected!
###############################################
GET /message.html HTTP/1.1
Host: www.berkshirehathaway.com

###############################################
### Will read 162 bytes
###############################################
### Header length is 301 bytes
###############################################
### read 463 bytes of data into buffer
###############################################
HTTP/1.1 301 Moved Permanently
Server: Sucuri/Cloudproxy
Date: Fri, 21 Aug 2020 00:55:39 GMT
Content-Type: text/html
Content-Length: 162
Connection: keep-alive
X-Sucuri-ID: 12018
Host-Header: e172abecbd394f56a1a2479517f27fbfe05ff815
Location: https://www.berkshirehathaway.com/message.html

<html>
<head><title>301 Moved Permanently</title></head>
<body>
<center><h1>301 Moved Permanently</h1></center>
<hr><center>nginx</center>
</body>
</html>
###############################################
### Reached end of message
###############################################
### Program exiting...
###############################################
```
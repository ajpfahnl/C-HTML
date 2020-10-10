# C-HTTP

A C program that performs HTTP requests. The program checks for `Content-Length: <#>`, and assumes `Transfer-Encoding: Chunked` otherwise to determine the end of the message body. Polling is also implemented to check if the connection closes or errors out.

By default, the program will try to connect with OpenSSL. To only use TCP, specify `--useTCP`.

Simply compile with `make` and then run the program with either of the following methods:
```
usage: ./c-http [GET/POST] --host=<host> [--port=<#>] [--path=<path>] [--verbose] [--useTCP] [--msg=<string>]
       ./c-http --url=<url> --profile=<n>
```
For the first method, the program defaults to the `GET` method and the `HTTPS`.

**This is currently a WIP. GET and POST are currently supported.**

## Troubleshooting

On MacOS, you might find compilation issues everywhere even after religiously `brew install`ing. These steps helped fix OpenSSL compilation issues for me:
* `ln -s /usr/local/opt/openssl/include/openssl /usr/local/include`
* `export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/opt/openssl/lib/`

## Running the Program
Running,
```
./c-http --url https://www.berkshirehathaway.com/message.html --profile 10
```
returns
```
<HTTP header>
<HTTP Body>

>> Profile:
   number of requests: 10
   fastest time (s): 0.000105
   slowest time (s): 0.000354
   mean time: 0.000166
   median time: 0.000128
   percentage of successes: 100
   error codes: None
   size of smallest response: 3515
   size of largest response: 3515
```

Another example,
```
./c-http --url https://cloudfare-general.ajpfahnl.workers.dev/lists --profile 10
```
```
<HTTP header>
<HTTP body>

>> Profile:
   number of requests: 10
   fastest time (s): 0.000140
   slowest time (s): 0.000566
   mean time: 0.000277
   median time: 0.000169
   percentage of successes: 100
   error codes: None
   size of smallest response: 5
   size of largest response: 4096
```

As another example, running
```
./c-http --url http://www.berkshirehathaway.com/message.html --verbose
```
or
```
./c-http GET --host www.berkshirehathaway.com --path /message.html --useTCP --verbose
```
returns
```
HTTP method: GET
###############################################
### Connected!
###############################################
###############################################
### Sending request...
###############################################
GET /message.html HTTP/1.1
Host: www.berkshirehathaway.com

###############################################
### Will read 162 bytes
### Header length is 301 bytes
###############################################

###############################################
### read 463 bytes of data into buffer
###############################################
HTTP/1.1 301 Moved Permanently
Server: Sucuri/Cloudproxy
Date: Fri, 21 Aug 2020 06:06:41 GMT
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
### Program exiting...
###############################################
```

## Useful Resources

* http://fm4dd.com/openssl/sslconnect.shtm
* https://aticleworld.com/http-get-and-post-methods-example-in-c/

## For Future Consideration

* https://www.libhttp.org/
* https://uriparser.github.io/ used in https://github.com/reagent/http
* https://curl.haxx.se/libcurl/c/
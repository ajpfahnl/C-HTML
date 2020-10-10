# C-HTTP

A C program that performs HTTP requests and prints the full HTTP response to standard output. The program checks for `Content-Length: <#>`, and assumes `Transfer-Encoding: Chunked` otherwise to determine the end of the message body. Polling is also implemented to check if the connection closes or errors out.

Simply compile with `make` and then run the program as follows:
```
recommended usage: ./c-http --url=<url> [--profile=<#>] [--help]
alternate usage:  ./c-http --host=<host> [--port=<#>] [--path=<path>] [--useTCP]
options common to both:
       [GET/POST]        specify HTTP method (default GET)
       [--verbose]       print out steps taken in a verbose way
       [--msg=<string>]  body of POST message (will be ignored with GET)
       [--profile=<#>]   print metrics
       [--help]          display this usage/help message
```

The first method of running the program automatically configures for SSL/TCP or TCP-only based on the URL, and it is the recommended way for running the program. By default, the second method will use `GET` amd `HTTPS`. To only use TCP, specify `--useTCP`.

The `--profile=<#>` option prints out performance metrics for `#` requests for a site. See the example outputs below for the format.

## Troubleshooting

On MacOS, you might find compilation issues everywhere even after religiously `brew install`ing. These steps helped fix OpenSSL compilation issues for me:
* `ln -s /usr/local/opt/openssl/include/openssl /usr/local/include`
* `export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/opt/openssl/lib/`

## Metrics with `--profile`
Running,
```
./c-http --url https://www.berkshirehathaway.com/message.html --profile 10
```
returns
```
...
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
...
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
This time with `google.com`,
```
./c-http --url https://www.google.com/ --profile 10   
```
```
...
>> Profile:
   number of requests: 10
   fastest time (s): 0.001364
   slowest time (s): 0.004120
   mean time: 0.002678
   median time: 0.003033
   percentage of successes: 100
   error codes: None
   size of smallest response: 5
   size of largest response: 4096
```

## Running with `--verbose`
As an example, running
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
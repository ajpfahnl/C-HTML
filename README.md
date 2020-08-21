# C-HTTP

A C program that performs HTTP requests. The program checks for `Content-Length: <#>`, and assumes `Transfer-Encoding: Chunked` otherwise to determine the end of the message body. Polling is also implemented to check if the connection closes or errors out.

By default, the program will try to connect with OpenSSL. To only use TCP, specify `--useTCP`.

Simply compile with `make` and then run the program as follows
```
./c-http GET --host=<host> [--portnum=<#>] [--path=<path>] [--verbose] [--useTCP]
```
**This is currently a WIP. Only GET is currently supported.**

For example, running
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

Here's another example with a TLS connection (HTTPS). Running
```
./c-http GET --host www.example.com --verbose
```
returns
```
HTTP method: GET
TCP connection established
SSL/TLS session established
###############################################
### Connected! Displaying certificate subject data...
###############################################
C=US, ST=California, L=Los Angeles, O=Internet Corporation for Assigned Names and Numbers, OU=Technology, CN=www.example.org
###############################################
### Sending request...
###############################################
GET / HTTP/1.1
Host: www.example.com

###############################################
### Will read 1256 bytes
### Header length is 351 bytes
###############################################

###############################################
### read 351 bytes of data into buffer
###############################################
HTTP/1.1 200 OK
Accept-Ranges: bytes
Age: 592764
Cache-Control: max-age=604800
Content-Type: text/html; charset=UTF-8
Date: Fri, 21 Aug 2020 06:02:23 GMT
Etag: "3147526947"
Expires: Fri, 28 Aug 2020 06:02:23 GMT
Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT
Server: ECS (dna/63A8)
Vary: Accept-Encoding
X-Cache: HIT
Content-Length: 1256


###############################################
### read 1256 bytes of data into buffer
###############################################
<!doctype html>
<html>
<head>
    <title>Example Domain</title>

    <meta charset="utf-8" />
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style type="text/css">
    body {
        background-color: #f0f0f2;
        margin: 0;
        padding: 0;
        font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", "Open Sans", "Helvetica Neue", Helvetica, Arial, sans-serif;
        
    }
    div {
        width: 600px;
        margin: 5em auto;
        padding: 2em;
        background-color: #fdfdff;
        border-radius: 0.5em;
        box-shadow: 2px 3px 7px 2px rgba(0,0,0,0.02);
    }
    a:link, a:visited {
        color: #38488f;
        text-decoration: none;
    }
    @media (max-width: 700px) {
        div {
            margin: 0 auto;
            width: auto;
        }
    }
    </style>    
</head>

<body>
<div>
    <h1>Example Domain</h1>
    <p>This domain is for use in illustrative examples in documents. You may use this
    domain in literature without prior coordination or asking for permission.</p>
    <p><a href="https://www.iana.org/domains/example">More information...</a></p>
</div>
</body>
</html>
###############################################
### Reached end of message
### Program exiting...
###############################################
```
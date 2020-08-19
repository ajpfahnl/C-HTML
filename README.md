# C-HTTP

A C program that performs HTTP requests. Simply compile with `make` and then run the program as follows
```
./c-http GET --portnum=<#> --host=<host> [--verbose]
```
**This is currently a WIP. Only GET is currently supported.**

For example, running
```
./c-http GET --portnum 80 --host example.com --verbose
```
will return: 
```
###############################################
### Connected!
###############################################
GET / HTTP/1.1
Host: example.com

###############################################
### read 1607 bytes of data in buf
###############################################
HTTP/1.1 200 OK
Accept-Ranges: bytes
Age: 443527
Cache-Control: max-age=604800
Content-Type: text/html; charset=UTF-8
Date: Wed, 19 Aug 2020 07:00:16 GMT
Etag: "3147526947"
Expires: Wed, 26 Aug 2020 07:00:16 GMT
Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT
Server: ECS (dna/63A8)
Vary: Accept-Encoding
X-Cache: HIT
Content-Length: 1256

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
### Program exiting...
###############################################
```
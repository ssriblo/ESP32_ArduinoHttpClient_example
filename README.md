Ported from https://github.com/arduino-libraries/ArduinoHttpClient


# How To run test example

## Hardware setup
    Ethernet controller connected by SPI. VSPI (SPI3) default pins used as below:
        sck=GPIO18
        miso=GPIO19
        mosi=GPIO23
Chip Select pin can be probrammable by following define (currently GPIO15):
```
#define SPI_CS_PIN        15
```

## MAC setup
W5500 Ethernet controller has no own MAC address and it defined as following example:
```
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
```

## Test options
* Comment all options except one at file example_ping_client_server.ino:
```  
// Choos only one test ones below!
// #define WEB_CLIENT_TEST   1
// #define WEB_SERVER_TEST   1
#define DOWNLOAD_TEST     1

There are 3 test options:
* (1) Web Cient, should use WEB_CLIENT_TEST
* (2) Web Server, should use WEB_SERVER_TEST
* (3) Client to dowunload data from Server, should use DOWNLOAD_TEST

## (1) Web Cliend
Used google.com ping. Print time to access and print returned raw htmp page if set:
```
#define PRINT_WEB_DATA    true 

## (2) Web Server 
Use browser to takes server's IP and Web Server returns ADC input values. Browser indicates those values and refresh after every F5

## (3) Download data from server
Assume that additional python server starts and returns test.txt content
Steps to reproduce:
* start scritp from here: example/test_server/server_download.sh
* note: server Port=8000
* note: test data placed at example/test_server/test.txt
* find out what is host IP
* edit following at example_ping_client_server.ino:
```
char *downloadUrl = "http://192.168.1.33:8000/test.txt"; // "\<IP\>":8000/test.txt

* start example and check print dump as below: 
```
```
20:06:00.390 > host=192.168.1.33 port=8000
20:06:00.390 > HTTP/1.0 200 OK
20:06:00.390 > Server: SimpleHTTP/0.6 Python/3.8.10
20:06:00.390 > Date: Mon, 18 Apr 2022 17:05:59 GMT
20:06:00.390 > Content-type: text/plain
20:06:00.390 > Content-Length: 11
20:06:00.390 > Last-Modified: Mon, 18 Apr 2022 17:04:31 GMT
20:06:00.390 > 
20:06:00.390 > 61 A 62 A 63 20 64 65 66 A A 
20:06:00.390 > progressf = 100
````

## IP for Ethernet connection

### DHCP option
If local router supports DHCP then comment USE_HARDCODED_IP option as below
```
//#define USE_HARDCODED_IP  1   /* alternative - IF sets by DHCP */
#ifndef USE_HARDCODED_IP
  #define USE_DHCP
#endif
```

### Hardcoded IP option
Uncomment USE_HARDCODED_IP option
and adjust following lines for you actual values:
```
  IPAddress ip(192, 168, 1, 60);
  IPAddress myDns(91, 219, 56, 96); //91.219.56.96  DNS
  IPAddress gateway(192, 168, 1, 11);
  IPAddress subnet(255, 255, 255, 0);  
```




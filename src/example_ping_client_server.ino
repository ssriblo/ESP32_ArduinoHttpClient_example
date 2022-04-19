// from https://github.com/arduino-libraries/Ethernet/blob/master/examples/WebServer/WebServer.ino

//#include <SPI.h>
#include "Ethernet.h"
#include "UDHttp.h"

///////////////////// SETUP SECTION START //////////////////////////
#define SPI_CS_PIN        15

// #define USE_HARDCODED_IP  1   /* alternative - IF sets by DHCP */
#ifndef USE_HARDCODED_IP
  #define USE_DHCP
#endif

// Choos only one test ones below!
#define WEB_CLIENT_TEST   1
// #define WEB_SERVER_TEST   1
// #define DOWNLOAD_TEST     1

#define PRINT_WEB_DATA    false  // set to false for better speed measurement

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
///////////////////// SETUP SECTION END   //////////////////////////

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;

///////////////////////////////////////////////////////////////////////////////
void ethCheck(EthernetClass Ethernet){
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My IP dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.print("My IP gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("My subnetMask: ");
  Serial.println(Ethernet.subnetMask());
}

///////////////////////////////////////////////////////////////////////////////
int wdataf(uint8_t *buffer, int len){
  for(int i=0; i<len; i++){
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  printf("write end");
  return len;
}

///////////////////////////////////////////////////////////////////////////////
void progressf(int percent){
  Serial.printf("progressf = %d\n", percent);
}

///////////////////////////////////////////////////////////////////////////////
void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  // Ethernet.init(SPI_CS_PIN);
  /*    SPI default configured as VSPI=SPI3
        cs=GPIO15 (see .init(15) above
        sck=GPIO18
        miso=GPIO19
        mosi=GPIO23    */

	int8_t sck = 18;
	int8_t miso = 19;
	int8_t mosi = 23;
	int8_t ss = 15;
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(500);
  Ethernet.init_spi_pins(sck, miso, mosi, ss);

  #ifdef USE_DHCP
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  Ethernet.begin(mac);
  ethCheck(Ethernet);
#endif // USE_DHCP

  delay(1000);

#ifdef USE_HARDCODED_IP
  Serial.println("Initialize Ethernet hardcoded ip, dns, gw, mask");
  IPAddress ip(192, 168, 1, 60);
  IPAddress myDns(91, 219, 56, 96); //91.219.56.96  DNS
  IPAddress gateway(192, 168, 1, 11);
  IPAddress subnet(255, 255, 255, 0);  
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  ethCheck(Ethernet);
#endif  

#ifdef WEB_CLIENT_TEST
  Serial.println("Ethernet WebClient Example");
  //IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS). Adjust for your cunnet site ip
  char server[] = "www.google.com";    // name address for Google (using DNS)
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed"); 
  }
#endif // WEB_CLIENT_TEST 

#ifdef DOWNLOAD_TEST
    char *downloadUrl = "http://192.168.1.33:8000/test.txt";
    UDHttp udh;
    int r = udh.download(client, downloadUrl, wdataf, progressf);
    if(r == -1)
    {
      Serial.println("error");
    }
#endif // DOWNLOAD_TEST

}

///////////////////////////////////////////////////////////////////////////////
void loop() {
#ifdef WEB_CLIENT_TEST
  // if there are incoming bytes available
  // from the server, read them and print them:
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (PRINT_WEB_DATA) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();

    // do nothing forevermore:
    while (true) {
      delay(1000);
    }
  }
#endif // WEB_CLIENT_TEST

#ifdef WEB_SERVER_TEST
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
#endif // WEB_SERVER_TEST  
}
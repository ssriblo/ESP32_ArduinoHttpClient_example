// Ported from https://github.com/nhatuan84/esp32-upload-download-multipart-http UDhttp library
// Note: only download test used. Upload test not works
/*
    Tuan Nguyen (http://www.iotsharing.com) or (nha.tuan84@gmail.com)
*/

#include "Ethernet.h"
#include "UDHttp.h"
#include <base64.h>

#define HEADER "POST %s HTTP/1.1\r\n" \
                "Host: %s:%d\r\n"\
                "Connection: keep-alive\r\n"\
                "Accept: */*\r\n"\
                "Content-Length: %d\r\n"\
                "Expect: \r\n"\
                "Content-Type: multipart/form-data; boundary=------------------------%s\r\n\r\n"
#define OPEN  "--------------------------%s\r\n"\
                "Content-Disposition: form-data; name='data'; filename='%s'\r\n"\
                "Content-Type: application/octet-stream\r\n\r\n"
#define CLOSE   "\r\n--------------------------%s--\r\n"

#define GETR  "GET %s HTTP/1.1\r\nHost: %s:%d\r\nAccept: */*\r\n\r\n"

UDHttp::UDHttp(){ 
}

UDHttp::~UDHttp(){
}

// EthernetClient client; 

// void networkInit(){
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//     // EthernetClient client; 
//     Ethernet.init(15);  
//  // start the Ethernet connection:
//   Serial.println("Initialize Ethernet with DHCP:");
//   if (Ethernet.begin(mac) == 0) {
//     Serial.println("Failed to configure Ethernet using DHCP");
//     if (Ethernet.hardwareStatus() == EthernetNoHardware) {
//       Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
//     } else if (Ethernet.linkStatus() == LinkOFF) {
//       Serial.println("Ethernet cable is not connected.");
//     }
//     // no point in carrying on, so do nothing forevermore:
//     while (true) {
//       delay(1);
//     }
//   }
//   delay(3000);
//   Serial.print("My IP address: ");
//   Serial.println(Ethernet.localIP());
//   Serial.print("My IP dnsServerIP: ");
//   Serial.println(Ethernet.dnsServerIP());
//   Serial.print("My IP gatewayIP: ");
//   Serial.println(Ethernet.gatewayIP());
//   Serial.print("My subnetMask: ");
//   Serial.println(Ethernet.subnetMask());  
// }

//simple url parser
int UDHttp::simpleUrlParser(char *url, char *host, char *path, int &port){
    char *tmp = strstr(url, "http://");
    port = 80;
    char cport[6];
    char *tmp2;
    char *tmp3;
    if(tmp != NULL && host != NULL){
        tmp2 = tmp + strlen("http://");
        tmp = strchr(tmp2, '/');
        if(tmp != NULL){
            if(strlen(tmp) < HOST_LEN){
                memcpy(path, tmp, strlen(tmp));
            }
            tmp3 = strchr(tmp2, ':');
            if(tmp3 == NULL) {
                int len = tmp - tmp2;
                if(len < HOST_LEN){
                    memcpy(host, tmp2, len);
                    return 0;
                }
                printf("increase HOST_LEN\n");
            } else {
                int len = tmp3 - tmp2;
                if(len < HOST_LEN){
                    memcpy(host, tmp2, len);
                    memset(cport, 0, 6);
                    memcpy(cport, tmp3+1, tmp- (tmp3+1));
                    port = atoi(cport);
                    return 0;
                }
                printf("increase HOST_LEN\n");
            }
        }
    }
    return -1;
}

void UDHttp::sendChunk(Client *client, uint8_t *buf, int len){
	int idx = 0;
	size_t result;
	while(len > 0){
		if(len < CHUNK_SIZE){
			result = client->write(&buf[idx], len);
		    len -= result;
		    idx += result;
		} else {
			result = client->write(&buf[idx], CHUNK_SIZE);
		    len -= result;
		    idx += result;
		}
	}
}


int UDHttp::download(EthernetClient client, char *downloadUrl, DataCb dataCb, ProgressCb progressCb){
    char buf[HEADER_SIZE];
    char host[HOST_LEN];
    char path[HOST_LEN];
	int port = 80;

    char num[10];
    unsigned int bytes;
    unsigned int received = 0;
    char *tmp = NULL;
    char *data;
    unsigned int total;
    unsigned int len = strlen("\r\n\r\n");

    // networkInit();
	delay(1000);

    if(dataCb == NULL){
        printf("DataCb is NULL!\n");
        return -1;
    }
	if((strlen(GETR) + strlen(downloadUrl)) > HEADER_SIZE){
        printf("Increase HEADER_SIZE\n");
		return -1;
	}
    //parse url
    memset(host, 0, HOST_LEN);
    memset(path, 0, HOST_LEN);
    if(simpleUrlParser(downloadUrl, host, path, port) == -1){
        printf("url is wrong\n");
        return -1;
    }
    printf("host=%s port=%d\n", host, port);
	if (!client.connect(host, port)) {
        printf("Connection failed LET TRY AGAIN !!!!!!!!\n");
        delay(100);
        if (!client.connect(host, port)){
            printf("Connection failed AGAIN !!!!!!!!!!!!!!!\n");
            return -1;
        }

	}

    memset(buf, 0, HEADER_SIZE);
	// snprintf(buf, HEADER_SIZE, GETR, downloadUrl, host, port);
	snprintf(buf, HEADER_SIZE, GETR, path, host, port);

    sendChunk(&client, (uint8_t *)buf, strlen(buf));
    memset(buf, 0, HEADER_SIZE);
    //read response
    do {
        if(client.available()){
            bytes = client.read((uint8_t *)&buf[received], HEADER_SIZE);
            if(bytes != -1){
                received += bytes;
                tmp = strstr(buf, "\r\n\r\n");
            }
        }
    } while (tmp == NULL);
    //parse header
    data = tmp;
    tmp = strstr(buf, "200");
    if(tmp == NULL)
    {
        return -1;
    }
    tmp = strstr(buf, "Content-Length: ");
    if(tmp == NULL)
    {
        return -1;
    }
    char *tmp2 = strstr(tmp, "\r\n");
    memset(num, 0, 10);
    int clen = strlen("Content-Length: ");
    memcpy(num, tmp+clen, tmp2-tmp-clen);
    total = atoi(num);
    Serial.printf("%s", buf);
    //start downloading
    if(received > (data+len+1-buf)){
		dataCb((uint8_t *)(data+len), received - (data+len-buf));
		total = total - (received - (data+len-buf));
	}
    clen = total;
    received = 0;
	do {
		bytes = client.read((uint8_t *)buf, HEADER_SIZE);
        if(bytes != -1){
            received += bytes;
		    dataCb((uint8_t *)buf, bytes);
		    total = total - bytes;
            if(progressCb != NULL){
                progressCb(100*received/clen);
            }
        }
	} while (total > 0);
	return 0;	
}

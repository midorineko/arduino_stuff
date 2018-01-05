/*
 * IRremoteESP8266: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 Sept, 2015
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
 * Copyright 2009 Ken Shirriff, http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus
 *   (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 */
#include "FS.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <stdlib.h>     /* atoi */
// an IR detector/demodulator is connected to GPIO pin 2

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;
const char* ssid     = "Cats in Space";
const char* password = "meowmixer";
ESP8266WebServer server(80);
String webPage = "";

uint16_t RECV_PIN = 14;
uint16_t SEND_PIN = 12;

IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);

decode_results results;

char rawData[10000] = "8960, 4394, 604, 506, 608, 502, 604, 1616, 604, 504, 604, 506, 606, 504, 632, 478, 606, 504, 606, 1614, 606, 1614, 606, 502, 608, 1612, 608, 1614, 608, 1612, 606, 1614, 606, 1616, 602, 506, 606, 504, 606, 504, 608, 1612, 608, 502, 608, 502, 606, 504, 606, 504, 604, 1614, 606, 1614, 608, 1614, 604, 504, 606, 1614, 604, 1618, 628, 1592, 606, 1614, 630";
const char s[3] = ", ";
char *token;
uint16_t crawlCall[200] = {};
int while_count = 0;
   
   /* get the first token */

uint16_t blawData[67] = {9000, 4500, 650, 550, 650, 1650, 600, 550, 650, 550,
                        600, 1650, 650, 550, 600, 1650, 650, 1650, 650, 1650,
                        600, 550, 650, 1650, 650, 1650, 650, 550, 600, 1650,
                        650, 1650, 650, 550, 650, 550, 650, 1650, 650, 550,
                        650, 550, 650, 550, 600, 550, 650, 550, 650, 550,
                        650, 1650, 600, 550, 650, 1650, 650, 1650, 650, 1650,
                        650, 1650, 650, 1650, 650, 1650, 600};
                        
void setup(void) {

  webPage += "<h1>ESP8266 Web Server</h1><p>Socket #1 <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"/mana/0\"><button>OFF</button></a></p>";
  webPage += "<p>Socket #2 <a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"/health/0\"><button>OFF</button></a></p>";

  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin();

  bool result = SPIFFS.begin();
  Serial.println("SPIFFS ready: " + result);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });

  server.on("/sendit", [](){   
    crawl();
    server.send(200, "text/html", webPage);
  });
  server.begin();
};

void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  uint16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } else if (results->decode_type == RC5X) {
    Serial.print("Decoded RC5X: ");
  } else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  } else if (results->decode_type == RCMM) {
    Serial.print("Decoded RCMM: ");
  } else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  } else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  } else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  } else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  } else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  serialPrintUint64(results->value, 16);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");
  Serial.println();
  File f = SPIFFS.open("/f.txt", "w");
  for (uint16_t i = 1; i < count; i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    if (i & 1) {
      Serial.print(results->rawbuf[i] * RAWTICK, DEC);
      f.print(results->rawbuf[i] * RAWTICK, DEC);
    } else {
      //Serial.write('-');
      Serial.print((uint32_t) results->rawbuf[i] * RAWTICK, DEC);
      f.print((uint32_t) results->rawbuf[i] * RAWTICK, DEC);
      }
    Serial.print(", ");
    f.print(", ");
  }
  f.close();
  Serial.println("finished saving I hope!" );
}
char* copy(const char* orig) {
  char *res = new char[strlen(orig)+1];
  strcpy(res, orig);
  return res;
};
void crawl(){
  File f = SPIFFS.open("/f.txt", "r");
    // we could open the file
  while(f.available()) {
    //Lets read line by line from the file
    String line = f.readStringUntil('\n');
    const char *char_line = line.c_str();
    char *clean_line = copy(char_line);
   token = strtok(clean_line, s);
   while( token != NULL ) 
   {
      int i =  atoi (token);
      crawlCall[while_count] = i;
      token = strtok(NULL, s);
      while_count++;
   }
   delete[] clean_line;
  }
  f.close();
  irsend.sendRaw(crawlCall, 67, 38);
  crawlCall[200] = {};
}
void loop(void) {
  if (irrecv.decode(&results)) {
    Serial.println(irrecv.decode(&results));
    serialPrintUint64(results.value, 16);
    dump(&results);
    irrecv.resume();  // Receive the next value
  }
  server.handleClient(); 
  
}
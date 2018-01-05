#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

/*
 * Connect to Spider Bot! Wifi Network (pass: 12345678)
 * Navigate to http://192.168.4.1/
 * Control the Spider
 */

/* Set these to your desired softAP credentials. They are not configurable at runtime */
const char *softAP_ssid = "Spider Bot!";
const char *softAP_password = "12345678";

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "Spider Bot!";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

/** Last time I tried to connect to WLAN */
long lastConnectTry = 0;

/** Current WLAN status */
int status = WL_IDLE_STATUS;

/** Extra Variables */

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  delay(1000);
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */  
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  /* dnsServer.start(DNS_PORT, "*", apIP); */
  dnsServer.start(DNS_PORT, "www.example.com", apIP);

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleRoot);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  
  server.on("/moveUpSwitch", moveUpSwitch);
  server.on("/moveDownSwitch", moveDownSwitch);
  server.on("/moveRightSwitch", moveRightSwitch);
  server.on("/moveLeftSwitch", moveLeftSwitch);

   server.on("/moveOneSwitch", moveOneSwitch);
  server.on("/moveTwoSwitch", moveTwoSwitch);
  server.on("/moveThreeSwitch", moveThreeSwitch);
  server.on("/moveFourSwitch", moveFourSwitch);

  server.onNotFound ( handleNotFound );
  server.begin(); // Web server start
  Serial.println("HTTP server started");
}


void loop() {
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}

String htmlTemplateTop = "<html><head><style>"
  "* {box-sizing: border-box;}"
  "body {font-family: Sans-serif;background: #edece7;}"
  ".switch {display: block;position: relative;width: 70px;height: 100px;margin: 70px auto;border-radius: 50px;background: #e6e3da;background: linear-gradient(#e6e3da, #fff);border: 1px solid rgba(0,0,0,0.1);box-shadow: inset 0 7px 0 #fdfdfd,0 2px 3px rgba(170, 160, 140,.3);cursor: pointer;}"
  ".switch:before {content: '';position: absolute;top: -10px; bottom: -10px;left: -5px; right: -5px;z-index: -1;background: #f2f1ed;border-radius: inherit;box-shadow:0 1px 1px rgba(#aea391,0.2),0 3px 3px rgba(170, 160, 140, 0.4),inset 0 1px 0 rgba(255,255,255,0.8),0 0 5px rgba(170, 160, 140, 0.5);}"
  ".switch:after {content: "";position:absolute;width: 60px;height: 70px;border-radius: 50%;z-index: -1;left: 18px;top: 10px;background: linear-gradient(160deg, rgba(170, 160, 140, 0.7), rgba(170, 160, 140, 0));background: -webkit-linear-gradient(290deg, rgba(170, 160, 140, 0.75), rgba(170, 160, 140, 0));-webkit-filter: blur(1px);}"
  "#switch {clip: rect(0 0 0 0);position: absolute;visibility: hidden;}"
  "#switch:checked ~ .switch {background: linear-gradient(#f7f6f4, #fff);box-shadow:inset 0 -5px 0 #dbd3c8,0 6px 5px rgba(170, 160, 140, 0.75),3px 16px 5px rgba(170,160,140, 0.3);border-bottom: none;}"
  "#switch:checked ~ .switch:after {display: none;}"
  "a {color:#000000;}"
  "#title {width:100%;text-align:center;font-size:40px;padding:10px;background:#336699;border-bottom: 2px solid #333333;}"
  "#network {width:100%;text-align:center;border-bottom: 2px solid #333333;}"
  ".network-item {font-size:30px;}"
  "#lights {width:100%;text-align:center;background:#666666;padding:30px;border-bottom: 2px solid #333333;}"
  "#light-status {border: 2px solid #333333;width:200px;margin:0 auto;background: #cccccc;border-radius: 20px;}"
  ".light-item {font-size:30px;margin:0 10px 10px 10px;}"
  "</style></head><body><div id='page'><div id='title'>Spider Bot ESP8266</div>";
String htmlTemplateBottom = "</div></body></html>";
String htmlLight = "<div id='lights'><p id='light-label' class='light-item'>Light Switch</p><p id='light-status' class='light-item'><input type='checkbox' name='switch' id='switch' onclick='window.location = &apos;/lightSwitch&apos;'><label class='switch' for='switch'></label></p></div>";
String htmlNetworkOne = "<div id='network'><p id='current-connection' class='network-item'>You are connected through the wifi network: <b></br>";
String buttonsOne = "</br></br><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/moveUpSwitch'>Forward</a><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/moveDownSwitch'>Backward</a><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/moveRightSwitch'>Right</a><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/moveLeftSwitch'>Left</a> </br>";
String buttonsTwo = "</br></br><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/moveOneSwitch'>Move 1</a><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/moveTwoSwitch'>Move 2</a><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/moveThreeSwitch'>Move 3</a><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/moveFourSwitch'>Move 4</a>";

/** Handle root or redirect to captive portal */
void handleRoot() {
  //Start page content
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.

  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 404, "text/plain", message );
}

/** Handle Spider Controls */
void moveUpSwitch(){
  Serial.println("MoveUpSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveDownSwitch(){
  Serial.println("MoveDownSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveRightSwitch(){
  Serial.println("MoveRightSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveLeftSwitch(){
  Serial.println("MoveLeftSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveOneSwitch(){
  Serial.println("MoveOneSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveTwoSwitch(){
  Serial.println("MoveTwoSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveThreeSwitch(){
  Serial.println("MoveThreeSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}

void moveFourSwitch(){
  Serial.println("MoveFourSwitch");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  //Put together HTML Template & Light Color
  server.sendContent(htmlTemplateTop + htmlNetworkOne);
  server.sendContent(String(softAP_ssid));
  server.sendContent(buttonsOne);
  server.sendContent(buttonsTwo);
  server.sendContent(htmlTemplateBottom);
  server.client().stop(); // Stop is needed because we sent no content length
}
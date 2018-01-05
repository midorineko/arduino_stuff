#define FASTLED_ESP8266_RAW_PIN_ORDER


#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>

const char *softAP_ssid = "LightStrip Sex";
const char *softAP_password = "12345678";
const char *myHostname = "LightStrip Sex";

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

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    15
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

uint8_t BeatsPerMinute = 62;

int r = 0;
int g = 0;
int b = 0;
bool homemade_method = false;

void setup() {
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  delay(1000);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "www.example.com", apIP);

  server.on("/", handleRoot);
  server.on("/rainbow", rainbow_call);
  server.on("/rainbowWithGlitter", rainbowWithGlitter_call);
  server.on("/confetti", confetti_call);
  server.on("/sinelon", sinelon_call);

  server.on("/juggle", juggle_call);
  server.on("/next", next_call);

  server.on("/bpm", [](){
    int beats = server.arg("beats").toInt();
    if (beats){
        BeatsPerMinute = beats;
    }
    bpm_call();
  });

  server.on("/solid", [](){
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();
    homemade_method = true;
    ambient_loop(r, g, b);
    handleRoot();
  });


  server.onNotFound ( handleNotFound );
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  confetti_call();
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
  "</style></head><body><div id='page'><div id='title'>Ligh Strip ESP8266</div>";
String htmlTemplateBottom = "</div></body></html>";
String htmlLight = "<div id='lights'><p id='light-label' class='light-item'>LightStrip Sex</p><p id='light-status' class='light-item'><input type='checkbox' name='switch' id='switch' onclick='window.location = &apos;/lightSwitch&apos;'><label class='switch' for='switch'></label></p></div>";
String htmlNetworkOne = "<div id='network'><p id='current-connection' class='network-item'>You are connected through the wifi network: <b></br>";
String buttonsOne = "</br></br><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/rainbow'>Rainbow</a><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/rainbowWithGlitter'>Glitter</a><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/confetti'>confetti</a><a style='width: 20%; height 30px; border: 1px solid blue; padding: 2% 2%; margin: 3% 3%;' href='/sinelon'>sinelon</a> </br>";
String buttonsTwo = "</br></br><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/juggle'>Juggle</a><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/bpm?beats=116'>BPM</a><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/solid?r=0&g=255=&b=100'>Solid</a><a style='width: 20%; height 30px; border: 1px solid orange; padding: 2% 2%; margin: 3% 3%;' href='/next'>next</a>";

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

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, bpm, juggle };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void ambient_loop(int a, int b, int c){
    for(int k = 0; k < 60; k++) {
      leds[k].setRGB(a, b, c);FastLED.show();
    }
}


void loop()
{
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
  // Call the current pattern function once, updating the 'leds' array
  if (homemade_method == false){
     gPatterns[gCurrentPatternNumber]();
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  // EVERY_N_SECONDS( 100 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void rainbow_call() 
{
  homemade_method = false;
  gCurrentPatternNumber = (1-1) % ARRAY_SIZE( gPatterns);
  // FastLED's built-in rainbow generator
  handleRoot();
}

void rainbowWithGlitter_call() 
{
  homemade_method = false;
  gCurrentPatternNumber = (2-1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
  handleRoot();
}

void confetti_call() 
{
  homemade_method = false;
   gCurrentPatternNumber = (3-1) % ARRAY_SIZE( gPatterns);
  // random colored speckles that blink in and fade smoothly
  handleRoot();;
}

void sinelon_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (4-1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
  handleRoot();
}

void bpm_call() 
{
  homemade_method = false;
  gCurrentPatternNumber = (5-1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
  handleRoot();
}

void juggle_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (6-1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
  handleRoot();
}


void next_call() 
{
  homemade_method = false;
  nextPattern();
  handleRoot();
}



void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
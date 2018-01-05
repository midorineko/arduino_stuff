#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERNAL
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define DATA_PIN 15
#define NUM_LEDS 900
#define CALIBRATION_TEMPERATURE TypicalLEDStrip  // Color correction

CRGB leds[NUM_LEDS];
CRGB leds_new[NUM_LEDS];

WebSocketsServer webSocket = WebSocketsServer(81);
const char* ssid     = "Cats in Space";
const char* password = "meowmixer";

uint8_t BeatsPerMinute = 62;
int r = 0;
int g = 0;
int b = 0;
bool homemade_method = false;
int FRAMES_PER_SECOND = 90;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
      }
      break;
    case WStype_TEXT:
      {
        String text = String((char *) &payload[0]);
        Serial.println(text);
        if (text == "LED") {
          Serial.println("led just lit");
          webSocket.sendTXT(num, "led just lit", lenght);
        }
        if (text == "rainbow") {
          homemade_method = false;
          rainbow_call();
          webSocket.sendTXT(num, "rainbow_call", lenght);
        }
        if (text == "rainbowWithGlitter") {
          homemade_method = false;
          rainbowWithGlitter_call();
          webSocket.sendTXT(num, "rainbowWithGlitter_call", lenght);
        }
        if (text == "confetti") {
          homemade_method = false;
          confetti_call();
          webSocket.sendTXT(num, "confetti_call", lenght);
        }
        if (text == "sinelon") {
          homemade_method = false;
          sinelon_call();
          webSocket.sendTXT(num, "sinelon_call", lenght);
        }
        if (text == "bpm") {
          homemade_method = false;
          bpm_call();
          webSocket.sendTXT(num, "bpm_call", lenght);
        }
        if (text.startsWith("bpm=")) {
          String npm_int = text.substring(text.indexOf("=") + 1, text.length());
          int beats = npm_int.toInt();
          BeatsPerMinute = beats;
          Serial.println(npm_int);
          bpm_call();
          webSocket.sendTXT(num, "bpm set called", lenght);
        }
        if (text.startsWith("frames=")) {
          String npm_int = text.substring(text.indexOf("=") + 1, text.length());
          int beats = npm_int.toInt();
          FRAMES_PER_SECOND = beats;
          webSocket.sendTXT(num, "Frames set called", lenght);
        }
        if (text.startsWith("brightness=")) {
          String npm_int = text.substring(text.indexOf("=") + 1, text.length());
          int beats = npm_int.toInt();
          if (homemade_method == true) {
            FastLED.setBrightness(beats);
            for (int i = 0; i < NUM_LEDS; i++) {
              leds_new[i] = leds[i];
            };

            for (int i = 0; i < NUM_LEDS; i++) {
              leds[i] = leds_new[i];
            };
            FastLED.show();
          } else {
            FastLED.setBrightness(beats);
          }
          webSocket.sendTXT(num, "Brightness set called", lenght);
        }
        if (text == "juggle") {
          homemade_method = false;
          juggle_call();
          webSocket.sendTXT(num, "juggle_call", lenght);
        }
        if (text == "clear") {
          homemade_method = true;
          for (int k = 0; k < NUM_LEDS; k++) {
            Serial.println(k);
            leds[k] = CRGB::Black;
          }
          FastLED.show();
        }
        if(text.startsWith("{")){
            String first_word = text.substring( 2, text.indexOf(":") - 1);
            String second_word = text.substring( text.indexOf(":")+2, text.indexOf("}")-1);
            Serial.println(first_word);
            Serial.println(second_word);
            if(first_word=="solid"){
                homemade_method = true;
                long number = strtol( &second_word[1], NULL, 16);
                // Split them up into r, g, b values
                long r = number >> 16;
                long g = number >> 8 & 0xFF;
                long b = number & 0xFF;
                 Serial.println(r);
                 Serial.println(g);
                 Serial.println(b);
                for (int k = 0; k < NUM_LEDS; k++) {
                  Serial.println(k);
                  leds[k].setRGB(r, g, b);
                }
                FastLED.show();
            }
        }
        if (text.startsWith("[")) {
          homemade_method = true;
          int time_loop = 0;
          String times_text = text.substring(2, text.indexOf(",") - 1);
          String time_count = (times_text.substring(1, times_text.indexOf(":") - 1));
          int time_count_int = time_count.toInt();
          while (time_loop < time_count_int) {
            int text_length = text.length();
            String clean_text = text.substring(text.indexOf(",") + 1, text_length - 1);
            while ( clean_text.length() > 20 ) {
              String current_val = (clean_text.substring(1, clean_text.indexOf(",") - 1));
              String led_num = (current_val.substring(1, current_val.indexOf(":") - 1));
              int led_id = led_num.toInt();
              String hex_color = (current_val.substring(current_val.indexOf(":") + 2, current_val.length() - 1));

              if (hex_color == "delay") {
                delay(led_id);
              } else {
                if (hex_color == "clear") {
                  for (int k = 0; k < NUM_LEDS; k++) {
                    leds[k] = CRGB::Black;
                  }
                  FastLED.show();
                } else {
                  long number = strtol( &hex_color[1], NULL, 16);
                  // Split them up into r, g, b values
                  long r = number >> 16;
                  long g = number >> 8 & 0xFF;
                  long b = number & 0xFF;
                  leds[led_id].setRGB(r, g, b); FastLED.show();
                }
              }


              clean_text = (clean_text.substring(clean_text.indexOf(",") + 1, clean_text.length()));
            }
            String current_val = (clean_text.substring(1, clean_text.indexOf(",") - 1));
            String led_num = (current_val.substring(1, current_val.indexOf(":") - 1));
            int led_id = led_num.toInt();
            String hex_color = (current_val.substring(current_val.indexOf(":") + 2, current_val.length() - 1));

            long number = strtol( &hex_color[1], NULL, 16);
            // Split them up into r, g, b values
            long r = number >> 16;
            long g = number >> 8 & 0xFF;
            long b = number & 0xFF;
            leds[led_id].setRGB(r, g, b); FastLED.show();

            time_loop += 1;
          }


        }
        if (text.startsWith("led_set")) {
          bool homemade_method = true;
          String xVal = (text.substring(text.indexOf(":") + 1, text.length()));
          String hex_color = (xVal.substring(0, xVal.indexOf(":")));
          String id = (xVal.substring(xVal.indexOf(":") + 1, xVal.length()));
          int id_int = id.toInt();
          long number = strtol( &hex_color[1], NULL, 16);

          // Split them up into r, g, b values
          long r = number >> 16;
          long g = number >> 8 & 0xFF;
          long b = number & 0xFF;

          leds[id_int].setRGB(r, g, b); FastLED.show();
          webSocket.sendTXT(num, "red changed", lenght);
        }

      }


      webSocket.sendTXT(num, payload, lenght);
      webSocket.broadcastTXT(payload, lenght);
      break;

    case WStype_BIN:
      hexdump(payload, lenght);

      // echo data back to browser
      webSocket.sendBIN(num, payload, lenght);
      break;
  }

}


void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, bpm, juggle };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void loop() {
  webSocket.loop();

  if (homemade_method == false) {
    gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
    // EVERY_N_SECONDS( 100 ) { nextPattern(); } // change patterns periodically
  }
  // send the 'leds' array out to the actual LED strip
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
  gCurrentPatternNumber = (1 - 1) % ARRAY_SIZE( gPatterns);
  // FastLED's built-in rainbow generator
}

void rainbowWithGlitter_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (2 - 1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
}

void confetti_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (3 - 1) % ARRAY_SIZE( gPatterns);
  // random colored speckles that blink in and fade smoothly
}

void sinelon_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (4 - 1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
}

void bpm_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (5 - 1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
}

void juggle_call()
{
  homemade_method = false;
  gCurrentPatternNumber = (6 - 1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
}


void next_call()
{
  homemade_method = false;
  nextPattern();
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
  if ( random8() < chanceOfGlitter) {
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
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
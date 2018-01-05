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

WebSocketsServer webSocket = WebSocketsServer(8080);
const char* ssid     = "Cats in Space";
const char* password = "meowmixer";

uint8_t BeatsPerMinute = 30;
int r = 0;
int g = 0;
int b = 0;
bool homemade_method = false;
int FRAMES_PER_SECOND = 30;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

//start new methods
bool rainbowMarchOn = false;
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
bool beatWaveOn = false;
bool rainbowBeatOn = false;
bool twoSinPalOn = false;
unsigned long previousMillis;                                 // Store last time the strip was updated.
// Most of these variables can be mucked around with. Better yet, add some form of variable input or routine to change them on the fly. 1970's here I come. . 
uint8_t thishue;                                              // You can change the starting hue value for the first wave.
uint8_t rainbowHue;
uint8_t thathue;                                              // You can change the starting hue for other wave.
uint8_t thisrot;                                              // You can change how quickly the hue rotates for this wave. Currently 0.
uint8_t thatrot;                                              // You can change how quickly the hue rotates for the other wave. Currently 0.
uint8_t allsat;                                               // I like 'em fully saturated with colour.
uint8_t thisdir;
uint8_t thatdir;
uint8_t alldir;                                               // You can change direction.
int8_t thisspeed;                                             // You can change the speed.
int8_t thatspeed;                                             // You can change the speed.
uint8_t allfreq;                                              // You can change the frequency, thus overall width of bars.
int thisphase;                                                // Phase change value gets calculated.
int thatphase;                                                // Phase change value gets calculated.
uint8_t thiscutoff;                                           // You can change the cutoff value to display this wave. Lower value = longer wave.
uint8_t thatcutoff;                                           // You can change the cutoff value to display that wave. Lower value = longer wave.
int thisdelay;                                                // Standard delay. . 
uint8_t fadeval;                                              // Use to fade the led's of course.
CRGBPalette16 thisPalette;
CRGBPalette16 thatPalette;
TBlendType    currentBlending;                                // NOBLEND or LINEARBLEND
#define qsubd(x, b)  ((x>b)?b:0)                     // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Unsigned subtraction macro. if result <0, then => 0
uint8_t deltahue = 10;                                        // Hue change between pixels.
uint8_t rainbowDelta = 10;  
//end new methods

//start peronal blend methods
int BlendR1 = 0;
int BlendR2 = 250;
int BlendB1 = 0;
int BlendB2 = 100;
int BlendG1 = 250;
int BlendG2 = 0;
int BlendGroup = 100;
bool BlendFirstColor = true;
bool BlendCycleOn = false;
bool FadeCycleOn = false;
int max_brightness = 100;
int cur_brightness = 100;
bool FadeFirstColor = true;
bool FadeBrightFlip = false;
CRGB endclr;
CRGB midclr;
//end personal blend methods

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
        if(text == "fade_cycle"){
          FadeCycleOn = true;
          BlendCycleOn = false;
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = false;
        }
        if(text == "blend_cycle"){
          FadeCycleOn = false;
          BlendCycleOn = true;
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = false;
        }
        if(text == "two_sin_pal"){
          FadeCycleOn = false;
          BlendCycleOn = false;
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = true;
        }
        if (text == "beat_wave") {
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = true;
          twoSinPalOn = false;
          FadeCycleOn = false;
          BlendCycleOn = false;
          webSocket.sendTXT(num, "beatwave on", lenght);
        }
        if (text == "rainbow_march_send") {
          homemade_method = true;
          rainbowMarchOn = true;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = false;
          FadeCycleOn = false;
          BlendCycleOn = false;
          webSocket.sendTXT(num, "rainbowmarch on", lenght);
        }
        if (text == "rainbow_beat") {
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = true;
          beatWaveOn = false;
          twoSinPalOn = false;
          FadeCycleOn = false;
          BlendCycleOn = false;
          webSocket.sendTXT(num, "rainbowbeat on", lenght);
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
        if (text == "confetti_color") {
          homemade_method = false;
          confetti_color_call();
          webSocket.sendTXT(num, "confetti_color_call", lenght);
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
          homemade_method = false;
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
          Serial.println(FRAMES_PER_SECOND);
          webSocket.sendTXT(num, "Frames set called", lenght);
        }
        if (text.startsWith("grouping=")) {
          String npm_int = text.substring(text.indexOf("=") + 1, text.length());
          int beats = npm_int.toInt();
          BlendGroup = beats;
          webSocket.sendTXT(num, "Grouping set", lenght);
        }
        if (text.startsWith("brightness=")) {
          String npm_int = text.substring(text.indexOf("=") + 1, text.length());
          int beats = npm_int.toInt();
          if (homemade_method == true) {
            FastLED.setBrightness(beats);
            max_brightness = beats;
            cur_brightness = beats;
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
          juggle_call();
          webSocket.sendTXT(num, "juggle_call", lenght);
        }
        if (text == "clear") {
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = false;
          FadeCycleOn = false;
          BlendCycleOn = false;
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
            if(first_word=="solid")
            {
                homemade_method = true;
                rainbowMarchOn = false;
                rainbowBeatOn = false;
                beatWaveOn = false;
                twoSinPalOn = false;
                FadeCycleOn = false;
                BlendCycleOn = false;
                long number = strtol( &second_word[1], NULL, 16);
                // Split them up into r, g, b values
                r = number >> 16;
                g = number >> 8 & 0xFF;
                b = number & 0xFF;
                for (int k = 0; k < NUM_LEDS; k++) {
                  leds[k].setRGB(r, g, b);
                }
                FastLED.show();
            }
            if(first_word=="color1"){
              homemade_method = true;
              rainbowMarchOn = false;
              rainbowBeatOn = false;
              beatWaveOn = false;
              twoSinPalOn = false;
                long number = strtol( &second_word[1], NULL, 16);
                // Split them up into r, g, b values
                r = number >> 16;
                g = number >> 8 & 0xFF;
                b = number & 0xFF;
                BlendR1 = r;
                BlendG1 = g;
                BlendB1 = b;
                for (int k = 0; k < NUM_LEDS; k++) {
                  leds[k].setRGB(r, g, b);
                }
                FastLED.show();
                delay(800);
            }
            if(first_word=="color2"){
              homemade_method = true;
              rainbowMarchOn = false;
              rainbowBeatOn = false;
              beatWaveOn = false;
              twoSinPalOn = false;
                long number = strtol( &second_word[1], NULL, 16);
                // Split them up into r, g, b values
                BlendR2 = number >> 16;
                BlendG2 = number >> 8 & 0xFF;
                BlendB2 = number & 0xFF;
                for (int k = NUM_LEDS/2; k < NUM_LEDS; k++) {
                  leds[k].setRGB(BlendR2, BlendG2, BlendB2);
                }
                FastLED.show();
                delay(800);
            }
        }
        if (text.startsWith("[")) {
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = false;
          FadeCycleOn = false;
          BlendCycleOn = false;
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
          homemade_method = true;
          rainbowMarchOn = false;
          rainbowBeatOn = false;
          beatWaveOn = false;
          twoSinPalOn = false;
          FadeCycleOn = false;
          BlendCycleOn = false;
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
  analogWrite(DATA_PIN,0);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
//  GBR green was green
//  rgb
//bgr
  FastLED.setBrightness(255);
  WiFi.begin(ssid, password);
  //new method setup options start
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  thisPalette = RainbowColors_p;
  thatPalette = RainbowColors_p;
  resetvars();                                                 
   // Initialize the variables
  //new method setup options end
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
  delay(1000);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  delay(1000);
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, bpm, juggle, confettiColor };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void loop() {
  webSocket.loop();

  if (homemade_method == false) {
    gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    yield();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;  // slowly cycle the "base color" through the rainbow
    }
    // EVERY_N_SECONDS( 100 ) { nextPattern(); } // change patterns periodically
  }
  if(rainbowMarchOn){
      rainbow_march();
      FastLED.show();
      yield();
  }
  if(rainbowBeatOn){
    rainbow_beat();
    FastLED.show();
     yield();
  }
  if(beatWaveOn){
    beatwave();
    EVERY_N_MILLISECONDS(1000 / FRAMES_PER_SECOND) {
      uint8_t maxChanges = 24; 
      nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
    }
    EVERY_N_SECONDS(5) {                                        // Change the target palette to a random one every 5 seconds.
      targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
    }
    FastLED.show();
    yield();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
  }
  if(twoSinPalOn){
    ChangeMe();
    EVERY_N_MILLISECONDS(1000 / FRAMES_PER_SECOND) {
      two_sin();                                                // Routine is still delay based, but at least it's now a non-blocking day.
    }
    FastLED.show();
    yield();
    FastLED.delay(1000 / (FRAMES_PER_SECOND*2));
  }
  if(BlendCycleOn){
    blendCycle();
    FastLED.show();
    yield();
    EVERY_N_MILLISECONDS(212 / FRAMES_PER_SECOND);
  }
  if(FadeCycleOn){
    faceCycle();
    for( int i = 0; i < NUM_LEDS; i++) {
      if(i%BlendGroup == 0){
       if(BlendFirstColor){
        BlendFirstColor = false;
       }else{
        BlendFirstColor = true;
       }
      }
      if(FadeFirstColor){
          if(BlendFirstColor){
            leds[i]=CRGB( BlendR1, BlendG1, BlendB1);
          }else{
           leds[i]=CRGB( BlendR2, BlendG2, BlendB2);
          }
      }else{
          if(BlendFirstColor){
            leds[i]=CRGB( BlendR2, BlendG2, BlendB2);
          }else{
           leds[i]=CRGB( BlendR1, BlendG1, BlendB1);
          }
      }
    }
    FastLED.show();
    EVERY_N_MILLISECONDS(212 / FRAMES_PER_SECOND);
  }
  // send the 'leds' array out to the actual LED strip
}

void faceCycle(){
  BlendFirstColor = true;
  if(max_brightness == 0){
    for( int i = 0; i < NUM_LEDS; i++) {
      if(i%BlendGroup == 0){
       if(BlendFirstColor == true){
        BlendFirstColor = false;
       }else{
        BlendFirstColor = true;
       }
      }
      if(FadeFirstColor == true){
          if(BlendFirstColor == true){
            leds[i]=CRGB( BlendR1, BlendG1, BlendB1);
          }else{
           leds[i]=CRGB( BlendR2, BlendG2, BlendB2);
          }
      }else{
          if(BlendFirstColor == true){
            leds[i]=CRGB( BlendR2, BlendG2, BlendB2);
          }else{
           leds[i]=CRGB( BlendR1, BlendG1, BlendB1);
          }
      }
    }
    FadeFirstColor = !FadeFirstColor;
    FadeBrightFlip = false;
  }
  if(max_brightness == cur_brightness){
    FadeBrightFlip = true;
  }
  if(FadeBrightFlip == true){
    max_brightness = max_brightness - 1;
  }else{
    max_brightness = max_brightness + 1;
  }
  FastLED.setBrightness(max_brightness);
}

void blendCycle() {
  uint8_t speed = beatsin8(6,0,255);
  endclr = blend(CRGB( BlendR1, BlendG1, BlendB1), CRGB( BlendR2, BlendG2, BlendB2), speed);
  midclr = blend(CRGB( BlendR2, BlendG2, BlendB2), CRGB( BlendR1, BlendG1, BlendB1), speed);
  BlendFirstColor = true;
  for( int i = 0; i < NUM_LEDS; i++) {
    if(i%BlendGroup == 0){
       if(BlendFirstColor == true){
        BlendFirstColor = false;
       }else{
        BlendFirstColor = true;
       }
    }
    if(BlendFirstColor == true){
      leds[i]=endclr;
    }else{
      leds[i]=midclr;
    }
  }
}

void two_sin() {

    thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase -= beatsin8(thisspeed, 2, 10);
    thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase -= beatsin8(thatspeed, 2, 10);
    thishue += thisrot;                                        // Hue rotation is fun for thiswave.
    thathue += thatrot;                                        // It's also fun for thatwave.
  
  for (int k=0; k<NUM_LEDS-1; k++) {
    int thisbright = qsuba(cubicwave8((k*allfreq)+thisphase), thiscutoff);      // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k*allfreq)+128+thatphase), thatcutoff);  // This wave is 180 degrees out of phase (with the value of 128).

    leds[k] = ColorFromPalette(thisPalette, thishue, thisbright, currentBlending);
    leds[k] += ColorFromPalette(thatPalette, thathue, thatbright, currentBlending);
  }
     nscale8(leds,NUM_LEDS,fadeval);
     
} // two_sin()



// RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p, ForestColors_p, and PartyColors_p.
void ChangeMe() {
  
  uint8_t secondHand = (millis() / 1000) % 60;                // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;                             // Static variable, means it's only defined once. This is our 'debounce' variable.
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: thisrot = 1; thatrot = 1; thisPalette=PartyColors_p; thatPalette=PartyColors_p; break;
      case  5: thisrot = 0; thatdir = 1; thatspeed = -4; thisPalette=ForestColors_p; thatPalette=OceanColors_p; break;
      case 10: thatrot = 0; thisPalette=PartyColors_p; thatPalette=RainbowColors_p; break;
      case 15: allfreq = 16; thisdir = 1; thathue = 128; break;
      case 20: thiscutoff = 96; thatcutoff = 240; break;
      case 25: thiscutoff = 96; thatdir = 0; thatcutoff = 96; thisrot = 1; break;
      case 30: thisspeed= -4; thisdir = 0; thatspeed= -4; break;
      case 35: thiscutoff = 128; thatcutoff = 128; break;
      case 40: thisspeed = 3; break;
      case 45: thisspeed = 3; thatspeed = -3; break;
      case 50: thisspeed = 2; thatcutoff = 96; thiscutoff = 224; thatspeed = 3; break;
      case 55: resetvars(); break;
      case 60: break;
    }
  }
  
} // ChangeMe()



void resetvars() {                       // Reset the variable back to the beginning.

  thishue = 0;                          // You can change the starting hue value for the first wave.
  thathue = 140;                        // You can change the starting hue for other wave.
  thisrot = 1;                          // You can change how quickly the hue rotates for this wave. Currently 0.
  thatrot = 1;                          // You can change how quickly the hue rotates for the other wave. Currently 0.
  allsat = 255;                         // I like 'em fully saturated with colour.
  thisdir = 0;                          // Change the direction of the first wave.
  thatdir = 0;                          // Change the direction of the other wave.
  alldir = 0;                           // You can change direction.
  thisspeed = 4;                        // You can change the speed, and use negative values.
  thatspeed = 4;                        // You can change the speed, and use negative values.
  allfreq = 32;                         // You can change the frequency, thus overall width of bars.
  thisphase = 0;                        // Phase change value gets calculated.
  thatphase = 0;                        // Phase change value gets calculated.
  thiscutoff = 192;                     // You can change the cutoff value to display this wave. Lower value = longer wave.
  thatcutoff = 192;                     // You can change the cutoff value to display that wave. Lower value = longer wave.
  thisdelay = 10;                       // You can change the delay. Also you can change the allspeed variable above.
  fadeval = 192;                        // How quickly we fade.
  
} // resetvars()

void beatwave() {
  
  uint8_t wave1 = beatsin8(9, 0, 255);                        // That's the same as beatsin8(9);
  uint8_t wave2 = beatsin8(8, 0, 255);
  uint8_t wave3 = beatsin8(7, 0, 255);
  uint8_t wave4 = beatsin8(6, 0, 255);

  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, i+wave1+wave2+wave3+wave4, 255, currentBlending); 
  }
  
} // beatwave()

void rainbow_march() {                                        // The fill_rainbow call doesn't support brightness levels
  rainbowHue++;
  fill_rainbow(leds, NUM_LEDS, rainbowHue, rainbowDelta);            // Use FastLED's fill_rainbow routine.
} // rainbow_march()

void rainbow_beat() {
  
  uint8_t beatA = beatsin8(17, 0, 255);                        // Starting hue
  uint8_t beatB = beatsin8(13, 0, 255);
  fill_rainbow(leds, NUM_LEDS, (beatA+beatB)/2, 8);            // Use FastLED's fill_rainbow routine.

} // rainbow_beat()


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void rainbow_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (1 - 1) % ARRAY_SIZE( gPatterns);
  // FastLED's built-in rainbow generator
}

void rainbowWithGlitter_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (2 - 1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
}

void confetti_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (3 - 1) % ARRAY_SIZE( gPatterns);
  // random colored speckles that blink in and fade smoothly
}

void confetti_color_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (7 - 1) % ARRAY_SIZE( gPatterns);
  // random colored speckles that blink in and fade smoothly
}

void sinelon_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (4 - 1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
}

void bpm_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (5 - 1) % ARRAY_SIZE( gPatterns);
  // built-in FastLED rainbow, plus some random sparkly glitter
}

void juggle_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
  gCurrentPatternNumber = (6 - 1) % ARRAY_SIZE( gPatterns);
  // a colored dot sweeping back and forth, with fading trails
}


void next_call()
{
  homemade_method = false;
  rainbowMarchOn = false;
  rainbowBeatOn = false;
  beatWaveOn = false;
  twoSinPalOn = false;
  FadeCycleOn = false;
  BlendCycleOn = false;
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
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void confettiColor(){
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k].setRGB(r, g, b);
  }
  addGlitter(1000 / FRAMES_PER_SECOND);
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
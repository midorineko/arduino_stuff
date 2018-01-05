#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERNAL
#define FASTLED_INTERRUPT_RETRY_COUNT 0 
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define DATA_PIN 15
#define NUM_LEDS 900
#define CALIBRATION_TEMPERATURE TypicalLEDStrip  // Color correction

#define COLOR_ORDER GRB

int colorStepR = 255;
int colorStepG = 0;
int colorStepB = 0;

bool colorStepRUp = false;
bool colorStepGUp = true;
bool colorStepBUp = true;

CRGB leds[NUM_LEDS];

void setup(){
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
}
void loop(){

  checkColor();
  
  FastLED.show();
  delay(1000); 
}

void checkColor(){
  for(int x = 0; x < NUM_LEDS; x++){
      leds[x] = CRGB(colorStepR,colorStepG,colorStepB);
  }
  if(colorStepRUp == true){
    colorStepR++;
    if(colorStepR > 255){
      colorStepRUp = false;
    }
  }else{
    colorStepR--;
    if(colorStepR < 10 ){
      colorStepRUp = true;
    }
  }
  if(colorStepBUp == true){
    colorStepB++;
    if(colorStepB > 255){
      colorStepBUp = false;
    }
  }else{
    colorStepB--;
    if(colorStepB < 10 ){
      colorStepBUp = true;
    }
  }

}
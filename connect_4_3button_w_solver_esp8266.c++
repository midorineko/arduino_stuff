#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"
#include <time.h>
#include "FS.h"

#define NUM_LEDS 42
CRGBArray<NUM_LEDS> leds;
CRGBArray<NUM_LEDS> leds_new;

int curLed = 0;
int lastInt = 0;
char* lastStuff;
bool starting = true;
bool downLoop = true;
bool rightLoop = true;
bool leftLoop = true;
bool diagDownLeftLoop = true;
bool diagDownRightLoop = true;
bool diagUpRightLoop = true;
bool diagUpLeftLoop = true;
bool winner = false;
char loopColor[1] = {'1'};
int point[2] = {0, 0};
int endPoint[2] = {point[0], point[1]};
int finalWinnerHue = 0;
bool darBoard = true;
bool compEnemy = false;
bool compTurn = true;
int dropPosition = 3;
char* movesMade = "";
bool movesCleared = true;

void setup() {
  SPIFFS.begin();
  // SPIFFS.format(); only run once
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  Serial.println("in setup");
  FastLED.addLeds<NEOPIXEL, 15>(leds, NUM_LEDS);
  FastLED.setBrightness( 255 );
  for (int led = 0; led < 42; led++) {
    if (darBoard == false) {
      leds[led] = CHSV(5, 255, 0); FastLED.show();
    } else {
      leds[led] = CHSV(55, 255, 255); FastLED.show();
    }
  }
  Serial.begin(115200);
}

char color[1] = {'r'};
int curHue = 1;
void setLedArray(char* a, int* l) {
  for (int i = 6; i -- > 0;) {
    if (a[i] == 'r') {
      leds[l[i]] = CHSV(155, 255, 255); FastLED.show();
      a[i] = 'r';
    } else if (a[i] == 'b') {
      leds[l[i]] = CHSV(5, 255, 255); FastLED.show();
      a[i] = 'b';
    } else {
      if (color[0] == 'r') {
        leds[l[i]] = CHSV(155, 255, 255); FastLED.show();
      } else {
        leds[l[i]] = CHSV(5, 255, 255); FastLED.show();
      }
      if (darBoard == false) {
        FastLED.delay(30); leds[l[i]] = CHSV(5, 255, 0); FastLED.show();
      } else {
        FastLED.delay(30); leds[l[i]] = CHSV(55, 255, 255); FastLED.show();
      }
    }
  }
  if (color[0] == 'r') {
    color[0] = 'b';
    curHue = 100;
  } else {
    color[0] = 'r';
    curHue = 1;
  }
  checkWin();
  checkComnpEnemy();
}

void checkComnpEnemy() {
  if (compEnemy == true && compTurn == true) {
    compTurn = false;
    switch (rand() % 7) {
      case 0: rowA(); break;
      case 1: rowB(); break;
      case 2: rowC(); break;
      case 3: rowD(); break;
      case 4: rowE(); break;
      case 5: rowF(); break;
      case 6: rowG(); break;
    }
  }
}

void selectRow(char* a, int* l) {
  bool changed = false;
  for (int i = 0; i < 6; i++) {
    if (changed == false) {
      if (a[i] != 'r') {
        if (a[i] != 'b') {
          changed = true;
          curLed = l[i];
          lastStuff = a;
          lastInt = i;
          starting = false;
          a[i] = color[0];
          point[1] = i;
          endPoint[1] = i;
        }
      }
    }
  }
  setLedArray(a, l);
}

void resetLastMove() {
  if (color[0] == 'r') {
    color[0] = 'b';
  } else {
    color[0] = 'r';
  }
  strcat(movesMade,"R");
  lastStuff[lastInt] = '1';
  if (darBoard == false) {
    FastLED.delay(150); leds[curLed] = CHSV(5, 255, 0); FastLED.show();
  } else {
    FastLED.delay(150); leds[curLed] = CHSV(55, 255, 255); FastLED.show();
  }
  delay(200);
}

char az[6] = {'1', '2', '3', '4', '5', '6'};
int al[6] = {5, 4, 3, 2, 1, 0};
void rowA() {
  point[0] = 0;
  endPoint[0] = 0;
  strcat(movesMade,"A");
  selectRow(az, al);
}

char b[6] = {'1', '2', '3', '4', '5', '6'};
int bl[6] = {6, 7, 8, 9, 10, 11};
void rowB() {
  point[0] = 1;
  endPoint[0] = 1;
  strcat(movesMade,"B");
  selectRow(b, bl);
}

char c[6] = {'1', '2', '3', '4', '5', '6'};
int cl[6] = {17, 16, 15, 14, 13, 12};
void rowC() {
  point[0] = 2;
  endPoint[0] = 2;
  strcat(movesMade,"C");
  selectRow(c, cl);
}

char d[6] = {'1', '2', '3', '4', '5', '6'};
int dl[6] = {18, 19, 20, 21, 22, 23};
void rowD() {
  point[0] = 3;
  endPoint[0] = 3;
  strcat(movesMade,"D");
  selectRow(d, dl);
}

char e[6] = {'1', '2', '3', '4', '5', '6'};
int el[6] = {29, 28, 27, 26, 25, 24};
void rowE() {
  point[0] = 4;
  endPoint[0] = 4;
  strcat(movesMade,"E");
  selectRow(e, el);
}

char f[6] = {'1', '2', '3', '4', '5', '6'};
int fl[6] = {30, 31, 32, 33, 34, 35};
void rowF() {
  point[0] = 5;
  endPoint[0] = 5;
  strcat(movesMade,"F");
  selectRow(f, fl);
}

char g[6] = {'1', '2', '3', '4', '5', '6'};
int gl[6] = {41, 40, 39, 38, 37, 36};
void rowG() {
  point[0] = 6;
  endPoint[0] = 6;
  strcat(movesMade,"G");
  selectRow(g, gl);
}

void addWinToFile(char* movesMade){
    File r = SPIFFS.open("/c_4_wins.txt", "a");
    while( !r ) {
        File r = SPIFFS.open("/c_4_wins.txt", "a");
    }
    r.println(movesMade);
    r.println("");
    r.close();
}

void checkWin() {
  int winLeds[4] = {0, 0, 0, 0};
  int* ledArr[7] = {al, bl, cl, dl, el, fl, gl};
  char* arr[7] = {az, b, c, d, e, f, g};
  loopColor[0] = arr[point[0]][point[1]];
  point[0] = endPoint[0];
  point[1] = endPoint[1];

  for (int z = 0; z < 4; z++) {
    if (winner == false) {
      winLeds[z] = ledArr[point[0]][point[1]];
    }
    if (arr[point[0]][point[1]] != loopColor[0] || point[0] < 0 || point[1] < 0 || point[0] > 6 || point[1] > 5) {
      downLoop = false;
    }
    point[1] = point[1] - 1;
  }//down solving is working, next is right then left
  if (downLoop == true) {
    winner = true;
  } else {
    downLoop = true;
  }

  point[0] = endPoint[0];
  point[1] = endPoint[1];
  for (int z = 0; z < 7; z++) {
    if (point[0] < 1 || point[1] < 0 || point[0] > 6|| point[1] > 5) {
    } else {
      if (arr[point[0] - 1][point[1]] == loopColor[0]) {
        point[0] = point[0] - 1;
      }
    }
  }
  for (int z = 0; z < 4; z++) {
    if (point[0] < 0 || point[1] < 0 || point[0] > 6 || point[1] > 5) {
      rightLoop = false;
    } else {
      if (winner == false) {
        winLeds[z] = ledArr[point[0]][point[1]];
      }
      if (arr[point[0]][point[1]] != loopColor[0]) {
        rightLoop = false;
      }
    }
    point[0] = point[0] + 1;
  }//right solving is working, next is right then left
  if (rightLoop == true) {
    winner = true;
  } else {
    rightLoop = true;
  }

  point[0] = endPoint[0];
  point[1] = endPoint[1];
  for (int z = 0; z < 7; z++) {
    if (point[0] < 1 || point[1] < 0 || point[0] > 5 || point[1] > 5) {
    } else {
      if (arr[point[0] + 1][point[1] + 1] == loopColor[0]) {
        point[0] = point[0] + 1;
        point[1] = point[1] + 1;
      }
    }
  }
  for (int z = 0; z < 4; z++) {
    if (point[0] < 0 || point[1] < 0 || point[0] > 6 || point[1] > 5) {
      diagDownLeftLoop = false;
    } else {
      if (winner == false) {
        winLeds[z] = ledArr[point[0]][point[1]];
      }
      if (arr[point[0]][point[1]] != loopColor[0]) {
        diagDownLeftLoop = false;
      }
    }
    point[0] = point[0] - 1;
    point[1] = point[1] - 1;
  }//diagDownLeftLoop solving is working
  if (diagDownLeftLoop == true) {
    winner = true;
  } else {
    diagDownLeftLoop = true;
  }

  point[0] = endPoint[0];
  point[1] = endPoint[1];
  for (int z = 0; z < 7; z++) {
    if (point[0] < 1 || point[1] < 0 || point[0] > 6 || point[1] > 5) {
    } else {
      if (arr[point[0] - 1][point[1] + 1] == loopColor[0]) {
        point[0] = point[0] - 1;
        point[1] = point[1] + 1;
      }
    }
  }
  for (int z = 0; z < 4; z++) {
    if (point[0] < 0 || point[1] < 0 || point[0] > 6 || point[1] > 5) {
      diagDownRightLoop = false;
    } else {
      if (winner == false) {
        winLeds[z] = ledArr[point[0]][point[1]];
      }
      if (arr[point[0]][point[1]] != loopColor[0]) {
        diagDownRightLoop = false;
      }
    }
    point[0] = point[0] + 1;
    point[1] = point[1] - 1;
  }//diagDownRightLoop solving is working, next is right then left
  if (diagDownRightLoop == true) {
    winner = true;
  } else {
    diagDownRightLoop = true;
  }

  if (winner == true) {
    if (loopColor[0] == 'b') {
      finalWinnerHue = 5;
      strcat(movesMade,"red");
    } else {
      finalWinnerHue = 155;
      strcat(movesMade,"blue");
    }
    if (movesCleared == true) {
//      addWinToFile(movesMade);
    }
    for (int i = 0; i < 42; i++) {
      leds_new[i] = leds[i];
      leds[i] = CRGB::Black; FastLED.show();
    };
    FastLED.delay(50);
    for (int i = 0; i < 42; i++) {
      leds[i] = CHSV(finalWinnerHue, 255, 255); FastLED.show();
    };
    FastLED.delay(50);
    for (int i = 0; i < 42; i++) {
      leds[i] = CRGB::Black; FastLED.show();
    };
    FastLED.delay(50);
    for (int i = 0; i < 42; i++) {
      leds[i] = CHSV(finalWinnerHue, 255, 255); FastLED.show();
    };
    winner = false;
    FastLED.delay(50);
    for (int i = 0; i < 42; i++) {
      leds[i] = leds_new[i]; FastLED.show();
    };
  }
}

void resetBoard(char* az, char* b, char* c, char* d, char* e, char* f, char* g) {
  for (int cou = 0; cou < 6; cou++) {
    az[cou] = 'a';
    b[cou] =  'v';
    c[cou] =  's';
    d[cou] =  'e';
    e[cou] =  'u';
    f[cou] =  'p';
    g[cou] =  'q';
  }
}
void startClear() {
  resetBoard(az, b, c, d, e, f, g);
  memset(movesMade, 0, 43);
  movesCleared = true;
  FastLED.clear();
  FastLED.setBrightness( 255 );
  for (int led = 0; led < 42; led++) {
    if (darBoard == false) {
      leds[led] = CHSV(5, 255, 0); FastLED.show();
    } else {
      leds[led] = CHSV(55, 255, 255); FastLED.show();
    }
  }
  FastLED.show();
  delay(200);
}
void naps() {
  FastLED.setBrightness( 0 );
  starting = false;
}
void setColorInAction() {
  if (color[0] == 'b') {
    for (int led = 0; led < 8; led++) {
      leds[led] = CHSV(5, 255, 255); FastLED.show();
    }
  } else {
    for (int led = 0; led < 8; led++) {
      leds[led] = CHSV(155, 255, 255); FastLED.show();
    }
  }
}

void setDropPosition() {
  int dropArr[6] = {};
  if (dropPosition == 0) {
    dropArr[0] = 0;
    dropArr[1] = 1;
    dropArr[2] = 2;
    dropArr[3] = 3;
    dropArr[4] = 4;
    dropArr[5] = 5;
  };
  if (dropPosition == 1) {
    dropArr[0] = 11;
    dropArr[1] = 10;
    dropArr[2] = 9;
    dropArr[3] = 8;
    dropArr[4] = 7;
    dropArr[5] = 6;
  };
  if (dropPosition == 2) {
    dropArr[0] = 12;
    dropArr[1] = 13;
    dropArr[2] = 14;
    dropArr[3] = 15;
    dropArr[4] = 16;
    dropArr[5] = 17;
  };
  if (dropPosition == 3) {
    dropArr[0] = 23;
    dropArr[1] = 22;
    dropArr[2] = 21;
    dropArr[3] = 20;
    dropArr[4] = 19;
    dropArr[5] = 18;
  };
  if (dropPosition == 4) {
    dropArr[0] = 24;
    dropArr[1] = 25;
    dropArr[2] = 26;
    dropArr[3] = 27;
    dropArr[4] = 28;
    dropArr[5] = 29;
  };
  if (dropPosition == 5) {
    dropArr[0] = 35;
    dropArr[1] = 34;
    dropArr[2] = 33;
    dropArr[3] = 32;
    dropArr[4] = 31;
    dropArr[5] = 30;
  };
  if (dropPosition == 6) {
    dropArr[0] = 36;
    dropArr[1] = 37;
    dropArr[2] = 38;
    dropArr[3] = 39;
    dropArr[4] = 40;
    dropArr[5] = 41;
  };
  for (int i = 0; i < 6; i++) {
    leds_new[dropArr[i]] = leds[dropArr[i]];
    leds[dropArr[i]] = CRGB::Black; FastLED.show();
  };
  FastLED.delay(50);
  for (int i = 0; i < 6; i++) {
    leds[dropArr[i]] = leds_new[dropArr[i]]; FastLED.show();
  };
  FastLED.delay(50);
  for (int i = 0; i < 6; i++) {
    leds_new[dropArr[i]] = leds[dropArr[i]];
    leds[dropArr[i]] = CRGB::Black; FastLED.show();
  };
  FastLED.delay(50);
  for (int i = 0; i < 6; i++) {
    leds[dropArr[i]] = leds_new[dropArr[i]]; FastLED.show();
  };
}

void leftButton() {
  dropPosition -= 1;
  if (dropPosition < 0) {
    dropPosition = 6;
  };
  setDropPosition();
}

void rightButton() {
  dropPosition += 1;
  if (dropPosition > 6) {
    dropPosition = 0;
  };
  setDropPosition();
}

void selectDropPosition() {
  if (dropPosition == 0) {
    rowA();
  };
  if (dropPosition == 1) {
    rowB();
  };
  if (dropPosition == 2) {
    rowC();
  };
  if (dropPosition == 3) {
    rowD();
  };
  if (dropPosition == 4) {
    rowE();
  };
  if (dropPosition == 5) {
    rowF();
  };
  if (dropPosition == 6) {
    rowG();
  };
  dropPosition = 3;
  delay(200);
}

int buttonTimer = 0;
int longPressTime = 250;

boolean buttonActive = false;
boolean longPressActive = false;
void loop() {
  //  setColorInAction();
  int but1 = digitalRead(13);
  int but2 = digitalRead(12);
  int but3 = digitalRead(14);

  if (but1 == LOW) {
    leftButton();
    delay(100);
  }

  if (but3 == LOW) {
    rightButton();
    delay(100);
  }

  if (but2 == LOW) {
    selectDropPosition();
    delay(100);
  }

  if (but1 == LOW) {
    if (but2 == LOW) {
      if (but3 == LOW) {
        Serial.println("Reset Board");
        startClear();
        delay(200);
      }
    }
  }

  if (but1 == LOW) {
    if (but3 == LOW) {
      resetLastMove();
      delay(100);
    }
  }
}
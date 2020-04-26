#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <EEPROM.h>

#define LED_COUNT 188
#define BOX_LED_COUNT 30
#define LED_PIN 6
#define BOX_LED_PIN 9

CRGB leds[LED_COUNT];
CRGB boxleds[BOX_LED_COUNT];

// 64 is the fucked up pixel
// don't light pixel 64, you buffoon

#define FIRETIMER 100
#define FIREWINDOWS 100
#define WEED_MAX 420 // he he that's the weed number
#define RAINBOW_MAX 420

byte brightness = 250;
byte saturation = 255;
byte startColor = 0;
unsigned long counter = 0L;
int resetCounter = 0;
int rainbowTimer = RAINBOW_MAX;
int weedTimer = WEED_MAX; 
int prevButtonState = 0;
float weedHue = 0;
int firecounter = 0;
int fireWindowCount = 0;
unsigned long fireWindowStartMillis;
unsigned long buttonPushedMillis;

int allFireCounts[FIREWINDOWS];

byte colors[11][3] = {
  {   0,          0, brightness   }, // white
  {   0, saturation, brightness   }, // red
  {  28, saturation, brightness   }, // orange
  {28*2, saturation, brightness   }, // yellow
  {28*3, saturation, brightness   }, // l green
  {28*4, saturation, brightness   }, // green
  {28*5, saturation, brightness   }, // aqua-ish
  {28*6, saturation, brightness   }, // blue
  {28*7, saturation, brightness   }, // purpley
  {28*8, saturation, brightness   }, // pinkish
  {   0,          0,         55   }, // grey
};

byte WheelPos = 0;

byte segGroups[14][3] = {
  // right (seen from front) digit
  {   6,   7,   8 },    // top, a         //0
  {   9,  10,  11 },    // top right, b   //1
  {  13,  14,  15 },    // bottom right, c //2
  {  16,  17,  18 },    // bottom, d      //3
  {  21,  20,  19 },    // bottom left, e //4
  {   5,   4,   3 },    // top left, f    //5
  {   0,   1,   2 },    // center, g     //6
  // left (seen from front) digit
  {  38,  39,  40 },    // top, a
  {  41,  42,  43 },    // top right, b
  {  25,  26,  27 },    // bottom right, c
  {  28,  29,  30 },    // bottom, d
  {  33,  32,  31 },    // bottom left, e
  {  37,  36,  35 },    // top left, f
  {  44,  45,  46 }    // center, g};
};

byte rainbowColors[9][3] = {
  {20 * 0, saturation, brightness   }, // red
  {20 * 1, saturation, brightness   }, // orange
  {20 * 2, saturation, brightness   }, // yellow
  {20 * 3, saturation, brightness   }, // l green
  {20 * 4, saturation, brightness   }, // green
  {20 * 5, saturation, brightness   }, // aqua-ish
  {20 * 6, saturation, brightness   }, // blue
  {20 * 7, saturation, brightness   }, // purpley
  {20 * 8, saturation, brightness   }, // pinkish
};



byte digits[10][7] = {
  { 1, 1, 1, 1, 1, 1, 0 },  // 0
  { 0, 1, 1, 0, 0, 0, 0 },  // 1
  { 1, 1, 0, 1, 1, 0, 1 },  // 2
  { 1, 1, 1, 1, 0, 0, 1 },  // 3
  { 0, 1, 1, 0, 0, 1, 1 },  // 4
  { 1, 0, 1, 1, 0, 1, 1 },  // 5
  { 1, 0, 1, 1, 1, 1, 1 },  // 6
  { 1, 1, 1, 0, 0, 0, 0 },  // 7
  { 1, 1, 1, 1, 1, 1, 1 },  // 8
  { 1, 1, 1, 1, 0, 1, 1 },  // 9
};

int getFireCount(){
  int totalFire = 0;
  for (int i = 0; i < FIREWINDOWS; i++){
    totalFire += allFireCounts[i];
  }
  return totalFire;
}
void shuffleColors(){
  for (int i=0; i < 10; i++) {
   int n = random(0, 10); 
//   Serial.print("gonna swap the next two colors:");
//   Serial.print(i);
//   Serial.print(n);
   byte temp[3];
   copyColor(colors[n], temp, 3);
   copyColor(colors[i], colors[n], 3);
   copyColor(temp, colors[i], 3);
  }
}

void copyColor(byte* src, byte* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}

void setup() {
  randomSeed(analogRead(0));
  pinMode(13, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  Serial.begin(57600);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.addLeds<WS2812B, BOX_LED_PIN, GRB>(boxleds, BOX_LED_COUNT);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
  FastLED.setDither(0);
  FastLED.setBrightness(brightness);
  FastLED.clear();
  FastLED.show();
  EEPROM.get(0, counter);
//  counter = 88846;

  for (int i = 0; i < FIREWINDOWS; i++){
    allFireCounts[i] = 0;
  }
  fireWindowStartMillis = millis(); 
}

void loop() {
  unsigned long currentMillis = millis(); 
  FastLED.setBrightness(brightness);
  FastLED.clear();
  
  readButton();
  displayCounter();
  displayFire();

  readReset();
  displayReset();

  FastLED.show();
  updateEffects();
  if ( currentMillis - fireWindowStartMillis > FIRETIMER){
      allFireCounts[fireWindowCount] = firecounter;
      firecounter = 0;
      fireWindowCount = (fireWindowCount + 1) % FIREWINDOWS;
      fireWindowStartMillis = currentMillis;
  }
}

void readButton() {
  int counterButton = digitalRead(11);
  if (counterButton != prevButtonState ) {
    if (counterButton == LOW) {
      FastLED.clear();
      counter++;
      firecounter++;
      if (counter % 100 == 69) rainbowTimer = 0;
      if (counter % 1000 == 420) { weedTimer = 0; weedHue = 105; }
      if (counter % 1000 == 0) { shuffleColors(); }
      
      if (counter % 50 == 0) EEPROM.put(0, counter);
    }
    prevButtonState = counterButton;
  }
}

void readReset() {
  int resetButton = digitalRead(0);
  if (resetButton == LOW) {
    resetCounter++;
  }
  else {
    resetCounter = 0;
  }
}

void displayReset() {
  if (resetCounter > LED_COUNT) {
    clearMemory();
    FastLED.clear();
    counter = 0;
    resetCounter = 0;
  }

  for (int i = 0; i < resetCounter; i++)
  {
    if (i == 61 || i == 69) continue;  // lol this is out of date, I forget what it is now... 22?
    leds[i].setHSV(colors[1][0], colors[1][1], colors[1][2]);
  }
}

void displayCounter() {
  char counterString[8];
  sprintf(counterString, "%8ld", counter);
  Serial.println(counterString);
  for (unsigned int i = 0; i < 8; i++) {
    if((counterString[7-i]) == ' '){
      showDigit(0, 10, i);
    }
    else{
      int c = counterString[7 - i] - '0';
//      Serial.println("gonna show that digit now");
      showDigit(c, c, i);
    }
  }
}

void displayFire() {
  int count = getFireCount();
  byte color[3] = {  int(count/2.5), saturation, min(count*1.5, 250) };
//  Serial.println("fire count");
//  Serial.println(counterString);
  for (int i = 0; i < BOX_LED_COUNT; i++){
    boxleds[i].setHSV(color[0], color[1], color[2]);
  }
}

void showDigit(byte digit, byte color, byte segDisplay) {
//  Serial.println("showing a digit" );
//  Serial.println(segDisplay);
//  Serial.println('\n');
  for (byte i = 0; i < 7; i++) { // this is where it iterates over the segments
    if (digits[digit][i] != 0) {
      if(rainbowTimer < RAINBOW_MAX){
        showSegmentRainbow(i, color, segDisplay);
      }
      else if(weedTimer < WEED_MAX){
        byte weedColor[3] = {  int(weedHue), saturation, brightness };
        showSegmentWithColor(i, weedColor, segDisplay);
      }
      else showSegment(i, color, segDisplay);
    }
  }
}

void showDigitWithColor(byte digit, byte color[3], byte segDisplay) {
//  Serial.println("showing a digit" );
//  Serial.println(segDisplay);
//  Serial.println('\n');
  for (byte i = 0; i < 7; i++) { // this is where it iterates over the segments
    if (digits[digit][i] != 0) {
        showSegmentWithColor(i, color, segDisplay);
    }
  }
}

void showSegment(byte segment, byte color, byte segDisplay) {
//  Serial.println("showing a segment now!");
  int segmentOffset = 0;
  int pixelOffset = 0;
  if (segDisplay % 2 == 1) segmentOffset = 7;
  if (segDisplay >= 2) pixelOffset = 47;
  if (segDisplay >= 4) pixelOffset = 94;
  if (segDisplay >= 6) pixelOffset = 141;
  for (byte i = 0; i < 3; i++) {
    if (segGroups[segment+segmentOffset][i] + pixelOffset == 21) continue;
    leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(colors[color][0], colors[color][1], colors[color][2]);
  }
}

void showSegmentWithColor(byte segment, byte color[3], byte segDisplay) {
//  Serial.println("showing a segment now!");
  int segmentOffset = 0;
  int pixelOffset = 0;
  if (segDisplay % 2 == 1) segmentOffset = 7;
  if (segDisplay >= 2) pixelOffset = 47;
  if (segDisplay >= 4) pixelOffset = 94;
  if (segDisplay >= 6) pixelOffset = 141;
  for (byte i = 0; i < 3; i++) {
    leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(color[0], color[1], color[2]);
  }
}

void showSegmentRainbow(byte segment, byte color, byte segDisplay) {
//  Serial.println("showing rainbow segments oh shit");
  if(color==10) {showSegment(segment, color, segDisplay); return; }
  int segmentOffset = 0;
  int pixelOffset = 0;
  if (segDisplay % 2 == 1) segmentOffset = 7;
  if (segDisplay >= 2) pixelOffset = 47;
  if (segDisplay >= 4) pixelOffset = 94;
  if (segDisplay >= 6) pixelOffset = 141;
  for (byte i = 0; i < 3; i++) {
    //horizontal ones are easy
    if (segment == 0) leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(rainbowColors[0][0], rainbowColors[0][1], rainbowColors[0][2]);
    if (segment == 6) leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(rainbowColors[4][0], rainbowColors[4][1], rainbowColors[4][2]);
    if (segment == 3) leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(rainbowColors[8][0], rainbowColors[8][1], rainbowColors[8][2]);

    // vertical ones are fucking stupid
    if (segment == 1 || segment == 5) leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(rainbowColors[i][0], rainbowColors[i][1], rainbowColors[i][2]);
    if (segment == 2 || segment == 4) leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(rainbowColors[i+4][0], rainbowColors[i+4][1], rainbowColors[i+4][2]);
//    leds[(segGroups[segment+segmentOffset][i] + pixelOffset)].setHSV(rainbowColors[segment][0], rainbowColors[segment][1], rainbowColors[segment][2]);
  }
}

void updateEffects(){
  if(rainbowTimer < RAINBOW_MAX) {
    for (int i = 0; i < 9; i++)
      rainbowColors[i][0] = (rainbowColors[i][0]+1 % 255);
    rainbowTimer++;
//    Serial.println(rainbowTimer);
  }
  if(weedTimer < WEED_MAX) {
    weedHue -= 0.25;
    weedHue = max(weedHue, 24);
    weedTimer++;
//    Serial.println(rainbowTimer);
  }
}

void clearMemory()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  FastLED.clear();
}

// borrowed from Adafruit Neopixel library
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// borrowed from Adafruit Neopixel library
 static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
 }

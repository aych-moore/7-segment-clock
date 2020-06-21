#include <FastLED.h>
#include <WiFi.h>
#include "time.h"

#define NUM_LEDS 128
#define DATA_PIN_LEFT 19
#define DATA_PIN_RIGHT 32

const char* ssid       = "2=={::::::::::>";
const char* password   = "7NBMXRTKEP";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 43200;
const int   daylightOffset_sec = 0; //3600

byte hour;
byte minute;

bool separatorsOn = false;

CRGB ledsLeft[NUM_LEDS];
CRGB ledsRight[NUM_LEDS];

/*

         A
        ----
      F \    \ B
         \____\
        E \ G  \ C
           \____\
              D

HOUR
DOT 0,1
B 2,10
A 11,19
F 20,28 
G 29,37
C 38,46
D 47,55
E 56,64
B 65,73
A 74,82
F 83,91
G 92,100
C 101,109
D 110,118
E 119,127

MINUTE
DOT 0,1
E 2,10
D 11,19
C 20,28 
G 29,37
F 38,46
A 47,55
B 56,64
E 65,73
D 74,82
C 83,91
G 92,100
F 101,109
A 110,118
B 119,127

0 ABCDEF
1 BC
2 ABDEG
3 ABCDG
4 BCFG
5 ACDFG
6 CDEFG
7 ABC
8 ABCDEFG
9 ABCFG


 */     
//                                                           INDEX SEGMENT MAPPINGS                       
//                                            INSIDE                   |                 OUTSIDE
//                  HOUR   DOT  B    A     F     G      C     D     E     B     A     F     G      C       D       E             
//                  MINUTE DOT  E    D     C     G      F     A     B     E     D     C     G      F       A       B
//                         0 1 2 3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23  24  25  26  27  28  29
//start and end index     {0,1,2,10,11,19,20,28,29,37,38,46,47,55,56,64,65,73,74,82,83,91,92,100,101,109,110,118,119,127};

//==================================== LED INDEX MAPPINGS FOR SEGMENTS ====================================

//                                   A    B    C    D    E    F    G 
const byte hourTensDigits[10][7] = {{74,  65,  101, 110, 119, 83,  0},  //0
                                    {0,   65,  101, 0,   0,   0,   0},  //1
                                    {74,  65,  0,   110, 119, 0,   92}, //2
                                    {74,  65,  101, 110, 0,   0,   92}, //3
                                    {0,   65,  101, 0,   0,   83,  92}, //4
                                    {74,  0,   101, 110, 0,   83,  92}, //5
                                    {0,   0,   101, 110, 119, 83,  92}, //6
                                    {74,  65,  101, 0,   0,   0,   0},  //7
                                    {74,  65,  101, 110, 119, 83,  92}, //8
                                    {74,  65,  101, 0,   0,   83,  92}};//9
                                    
//                                   A    B    C    D    E    F    G 
const byte hourOnesDigits[10][7] = {{11,  2,   38,  47,  56,  20,  0},  //0
                                    {0,   2,   38,  0,   0,   0,   0},  //1
                                    {11,  2,   0,   47,  56,  0,   29}, //2
                                    {11,  2,   38,  47,  0,   0,   29}, //3
                                    {0,   2,   38,  0,   0,   20,  29}, //4
                                    {11,  0,   38,  47,  0,   20,  29}, //5
                                    {0,   0,   38,  47,  56,  20,  29}, //6
                                    {11,  2,   38,  0,   0,   0,   0},  //7
                                    {11,  2,   38,  47,  56,  20,  29}, //8
                                    {11,  2,   38,  0,   0,   20,  29}};//9

//                                     A    B    C    D    E    F    G 
const byte minuteTensDigits[10][7] = {{47,  56,  20,  11,  2,   38,  0},  //0
                                      {0,   56,  20,  0,   0,   0,   0},  //1
                                      {47,  56,  0,   11,  2,   0,   29}, //2
                                      {47,  56,  20,  11,  0,   0,   29}, //3
                                      {0,   56,  20,  0,   0,   38,  29}, //4
                                      {47,  0,   20,  11,  0,   38,  29}, //5
                                      {0,   0,   20,  11,  2,   38,  29}, //6
                                      {47,  56,  20,  0,   0,   0,   0},  //7
                                      {47,  56,  20,  11,  2,   38,  29}, //8
                                      {47,  56,  20,  0,   0,   38,  29}};//9

//                                     A    B    C    D    E    F    G 
const byte minuteOnesDigits[10][7] = {{110, 119, 83,  74,  65,  101, 0},  //0
                                      {0,   119, 83,  0,   0,   0,   0},  //1
                                      {110, 119, 0,   74,  65,  0,   92}, //2
                                      {110, 119, 83,  74,  0,   0,   92}, //3
                                      {0,   119, 83,  0,   0,   101, 92}, //4
                                      {110, 0,   83,  74,  0,   101, 92}, //5
                                      {0,   0,   83,  74,  65,  101, 92}, //6
                                      {110, 119, 83,  0,   0,   0,   0},  //7
                                      {110, 119, 83,  74,  65,  101, 92}, //8
                                      {110, 119, 83,  0,   0,   101, 92}};//9

                        

void displayDigit(byte digitIndex, byte numberToDisplay){
  switch(digitIndex){
    case 0: //hourTens
      for(byte i = 0; i < 7; i++){
        if(hourTensDigits[numberToDisplay][i] != 0){
          for(byte j = hourTensDigits[numberToDisplay][i]; j < hourTensDigits[numberToDisplay][i]+9; j++){
            ledsLeft[j] = CRGB(254, 40, 3); //254, 72, 3
          }
        }
      }
      break;
    case 1: //hourOnes
      for(byte i = 0; i < 7; i++){
        if(hourOnesDigits[numberToDisplay][i] != 0){
          for(byte j = hourOnesDigits[numberToDisplay][i]; j < hourOnesDigits[numberToDisplay][i]+9; j++){
            ledsLeft[j] = CRGB(254, 40, 3);
          }
        }
      }
      break;
    case 2: //minuteTens
      for(byte i = 0; i < 7; i++){
        if(minuteTensDigits[numberToDisplay][i] != 0){
          for(byte j = minuteTensDigits[numberToDisplay][i]; j < minuteTensDigits[numberToDisplay][i]+9; j++){
            ledsRight[j] = CRGB(254, 40, 3);
          }
        }
      }
      break;
    case 3: //minuteOnes
      for(byte i = 0; i < 7; i++){
        if(minuteOnesDigits[numberToDisplay][i] != 0){
          for(byte j = minuteOnesDigits[numberToDisplay][i]; j < minuteOnesDigits[numberToDisplay][i]+9; j++){
            ledsRight[j] = CRGB(254, 40, 3);
          }
        }
      }
      break;
  }
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;

  //convert to 12 hour
  if(hour > 12){
    hour -= 12;
  }

  
  

  byte hourOnes = (hour%10); //change to hour
  byte hourTens = ((hour/10)%10); //change to hour
  byte minuteOnes = (minute%10);
  byte minuteTens = ((minute/10)%10);

  clearAllDigits();
  
  if(hour >= 10){ //remove leading 0 for hour
    displayDigit(0,hourTens);
  }
  displayDigit(1,hourOnes);
  displayDigit(2,minuteTens);
  displayDigit(3,minuteOnes);
  FastLED.show();

  
}

void toggleSeparators(){
  if(separatorsOn){ //if on then turn off
    for(int i = 0; i < 2; i++){
      ledsLeft[i] = CRGB(0,0,0);
      ledsRight[i] = CRGB(0,0,0);
    }
  }
  else{ //if off then turn on
    for(int i = 0; i < 2; i++){
      ledsLeft[i] = CRGB(254, 40, 3);
      ledsRight[i] = CRGB(254, 40, 3);
    }
  }
  FastLED.show();
}

void clearAllDigits(){
  for(int i = 2; i < NUM_LEDS; i++){
    ledsLeft[i] = CRGB(0,0,0);
    ledsRight[i] = CRGB(0,0,0);
  }
}

void setup() { 
  FastLED.addLeds<NEOPIXEL, DATA_PIN_LEFT>(ledsLeft, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_RIGHT>(ledsRight, NUM_LEDS);

  FastLED.setMaxPowerInVoltsAndMilliamps(5,200); 
  FastLED.clear();

  Serial.begin(115200);
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  toggleSeparators(); //off initially so this will turn them on
}
 
void loop() { 
  /*
  for(int i = 0; i < NUM_LEDS; i++){
    ledsLeft[i] = CHSV(255,255,255); 
    ledsRight[i] = CHSV(175,255,255); 
    FastLED.show();
    delay(1); 
    FastLED.clear();
  }
  */
  
  printLocalTime();
  delay(60000);

  /*
  for(byte i = 0; i < 10; i++){
    clearAllDigits();
    displayDigit(0,i);
    displayDigit(1,i);
    displayDigit(2,i);
    displayDigit(3,i);
    FastLED.show();
    delay(1000);
  }
  */
  
}

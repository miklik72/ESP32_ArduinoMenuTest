#include <Arduino.h>

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 13
#define TFT_CS 5
#define TFT_RST 19

#include <menu.h>
#include <menuIO/adafruitGfxOut.h>
#include <streamFlow.h>
#include <ClickEncoder.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialIO.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#define encA 26
#define encB 27
#define encBtn 14
#define encSteps 4

#define BACKCOLOR TFT_BLACK
#define TEXTCOLOR TFT_WHITE

int chooseField = 1;
int cutsMade = 0;
int numberOfCuts = 5;
int lengthOfCuts = 50;
int feedLength = 304;
int exitMenuOptions = 0;

ClickEncoder clickEncoder(encA, encB, encBtn, encSteps);
ClickEncoderStream encStream(clickEncoder, 1);

Adafruit_ILI9341 gfx = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void feedInOut();
void runCuts();
void IRAM_ATTR onTimer();

#define LEDPIN LED_BUILTIN

#define textScale 3

// Star ArduinoMEnu

result doFeed() {
  delay(500);
  exitMenuOptions = 2;
  return proceed;
}

result doRunCuts() {
  delay(500);
  exitMenuOptions = 1;
  return proceed;
}

result updateEEPROM() {
  // writeEEPROM();
  return quit;
}

#define MAX_DEPTH 3

MENU(subMenuAdjustServo, "Adjust Servo Settings", doNothing, noEvent, noStyle
  ,
  OP("Run!",doFeed, enterEvent), EXIT("<Back"));

CHOOSE(chooseField, freeDirChoose, "Choose Direction", doNothing, noEvent, noStyle, VALUE("Forward",1,doNothing,noEvent), VALUE("Backwards", 0, doNothing, noEvent));

MENU(subMenuFeedInOut, "Feed Tape", doNothing, noEvent, noStyle, FIELD(feedLength, "Length of Feed:", "mm", 0,1000,10,1,doNothing,noEvent,noStyle), SUBMENU(feedDirChoose), OP("Run!", doFeed,enterEvent),EXIT("<Back"));

MENU(mainMenu, "COPPER TAPE CUTTER", doNothing, noEvent,wrapStyle, FIELD(lengthOfCuts, "Cut Size:", "mm", 0,2000,10,1,doNothing,noEvent,noStyle),FIELD(numberOfCuts,"Pieces:","",0,1000,10,1,doNothing,noEvent,noStyle),OP("Cut!",doRunCuts, enterEvent), SUBMENU(subMenuAdjustServo));

#define ILI9341_GRAY RGB565(128,128,128)

// define menu colors --------------------------------------------------------
// #define Black RGB565(0, 0, 0)
// #define Red RGB565(255, 0, 0)
// #define Green RGB565(0, 255, 0)
// #define Blue RGB565(0, 0, 255)
// #define Gray RGB565(128, 128, 128)
// #define LighterRed RGB565(255, 150, 150)
// #define LighterGreen RGB565(150, 255, 150)
// #define LighterBlue RGB565(150, 150, 255)
// #define LighterGray RGB565(211, 211, 211)
// #define DarkerRed RGB565(150, 0, 0)
// #define DarkerGreen RGB565(0, 150, 0)
// #define DarkerBlue RGB565(0, 0, 150)
// #define Cyan RGB565(0, 255, 255)
// #define Magenta RGB565(255, 0, 255)
// #define Yellow RGB565(255, 255, 0)
// #define White RGB565(255, 255, 255)
// #define DarkerOrange RGB565(255, 140, 0)

const colorDef<uint16_t> colors[6] MEMMODE = {
    //{{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
    {{(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLACK}, {(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLUE, (uint16_t)ILI9341_BLUE}},     //bgColor
    {{(uint16_t)ILI9341_GRAY, (uint16_t)ILI9341_GRAY}, {(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_WHITE}},     //fgColor
    {{(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_BLACK}, {(uint16_t)ILI9341_YELLOW, (uint16_t)ILI9341_YELLOW, (uint16_t)ILI9341_RED}},   //valColor
    {{(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_BLACK}, {(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_YELLOW, (uint16_t)ILI9341_YELLOW}}, //unitColor
    {{(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_GRAY}, {(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLUE, (uint16_t)ILI9341_WHITE}},     //cursorColor
    {{(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_YELLOW}, {(uint16_t)ILI9341_BLUE, (uint16_t)ILI9341_RED, (uint16_t)ILI9341_RED}},       //titleColor
};

#define GFX_WIDTH 160
#define GFX_HEIGHT 128
#define fontW 6
#define fontH 9

serialIn serial(Serial);
//MENU_INPUTS(in,&serial);
MENU_INPUTS(in,&encStream);

MENU_OUTPUTS(out,MAX_DEPTH
  ,SERIAL_OUT(Serial)
  ,ADAGFX_OUT(gfx,colors,6*textScale,9*textScale,{0,0,14,8},{14,0,14,8})
  ,NONE//must have 2 items at least
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.println(F("suspended..."));
    o.println(F("press [select]"));
    o.println(F("to continue"));
  }
  return proceed;
}

void setup() {
  pinMode(LEDPIN, OUTPUT);
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Menu 4.x");
  Serial.println("Use keys + - * /");
  Serial.println("to control the menu navigation");
  //nav.idleTask=idle;//point a function to be used when menu is suspended
  //mainMenu[1].disable();

  //SPI.begin();
  gfx.begin();
  //gfx.initR(INITR_BLACKTAB);
  gfx.setRotation(3);
  gfx.setTextSize(textScale);//test scalling
  gfx.setTextWrap(false);
  gfx.fillScreen(ILI9341_BLACK);
  gfx.setTextColor(ILI9341_RED,ILI9341_BLACK);
  gfx.println("Menu 4.x test on GFX");
  delay(1000);
}

bool blink(int timeOn,int timeOff) {
  return millis()%(unsigned long)(timeOn+timeOff)<(unsigned long)timeOn;
}

void loop() {
  nav.poll();
  digitalWrite(LEDPIN, blink(timeOn,timeOff));
}
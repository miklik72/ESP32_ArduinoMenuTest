#include <Arduino.h>

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 13
#define TFT_CS 5
#define TFT_RST 19

Adafruit_ILI9341 gfx = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#include <menu.h>
#include <menuIO/serialOut.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#define LEDPIN LED_BUILTIN
#define MAX_DEPTH 1
#define textScale 3

unsigned int timeOn=10;
unsigned int timeOff=90;

char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
char buf1[]="0x11";

MENU(mainMenu, "Blink menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(timeOn,"On","ms",0,1000,10,1, Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(timeOff,"Off","ms",0,10000,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,EXIT("<Back")
);

#define ILI9341_GRAY RGB565(128,128,128)

const colorDef<uint16_t> colors[6] MEMMODE={
  {{(uint16_t)ILI9341_BLACK,(uint16_t)ILI9341_BLACK}, {(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLUE,  (uint16_t)ILI9341_BLUE}},//bgColor
  {{(uint16_t)ILI9341_GRAY, (uint16_t)ILI9341_GRAY},  {(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_WHITE}},//fgColor
  {{(uint16_t)ILI9341_WHITE,(uint16_t)ILI9341_BLACK}, {(uint16_t)ILI9341_YELLOW,(uint16_t)ILI9341_YELLOW,(uint16_t)ILI9341_RED}},//valColor
  {{(uint16_t)ILI9341_WHITE,(uint16_t)ILI9341_BLACK}, {(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_YELLOW,(uint16_t)ILI9341_YELLOW}},//unitColor
  {{(uint16_t)ILI9341_WHITE,(uint16_t)ILI9341_GRAY},  {(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLUE,  (uint16_t)ILI9341_WHITE}},//cursorColor
  {{(uint16_t)ILI9341_WHITE,(uint16_t)ILI9341_YELLOW},{(uint16_t)ILI9341_BLUE,  (uint16_t)ILI9341_RED,   (uint16_t)ILI9341_RED}},//titleColor
};

serialIn serial(Serial);
MENU_INPUTS(in,&serial);

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
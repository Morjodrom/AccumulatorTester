#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <ArduinoTrace.h>
#include "Arduino.h"
#include <EncButton.h>

#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8
#define TEXT_SIZE 2
#define LINE_MARGIN_PX 4
#define SENSORS_POLLING_PERIOD_MS 1000

enum Alignment {
    Left, Right, Center
};
uint16_t getAlignedX(char * buffer, Alignment alignment);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
EncButton encoder = EncButton(7, 6, 5);

void setup()
{
    encoder.setEncType(EB_STEP4_LOW);
    Serial.begin(9600);

    DUMP(__TIME__);

    // standard charset is CP437, fix for the old library bug
    tft.cp437(true);
    tft.initR(INITR_BLACKTAB);   
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);
    tft.setTextSize(TEXT_SIZE);
}

float voltsNow = 0;
float targetMinVolts = 3.3;
unsigned int currentMilliAmp = 0;
unsigned int milliAmpsHour = 0;
unsigned int secondsSpent = 0;
unsigned int resistanceMilliOhm = 150;

unsigned long lastMillis = 0;
int displayTimeout = 0;
short editMode = 0;

char buffer[10];
void loop()
{
    unsigned long millisUpdate = millis() - lastMillis;
    lastMillis = millis();

    if(encoder.tick()){        
        if(editMode == 0 && encoder.click()){
            editMode = 1;
        }

        if(editMode > 0 && encoder.turn()){
            editMode += encoder.dir();

            if(editMode > 1){
                editMode = 0;
            }
        }

        DUMP(editMode);
    }

    displayTimeout -= millisUpdate;
    if(displayTimeout <= 0){
        displayTimeout = SENSORS_POLLING_PERIOD_MS;

        voltsNow += 0.5;
        currentMilliAmp += 100;
        secondsSpent = floor(millis() / 1000);

        updateDisplay();
    }
}

void updateDisplay()
{
    sprintf(buffer, "%-6s", (String(voltsNow, 2) + 'v').c_str());
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.print(buffer);

    sprintf(buffer, ">%5s", (String(targetMinVolts, 1) + 'v').c_str());
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(getAlignedX(buffer, Alignment::Right), 0);
    tft.print(buffer);

    tft.println();
    tft.setCursor(0, tft.getCursorY() + LINE_MARGIN_PX);

    sprintf(buffer, "%4dmA", currentMilliAmp);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(0, tft.getCursorY());
    tft.print(buffer);

    sprintf(buffer, "%dmAh", milliAmpsHour);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(getAlignedX(buffer, Alignment::Right), tft.getCursorY());
    tft.print(buffer);

    tft.println();
    tft.setCursor(0, tft.getCursorY() + LINE_MARGIN_PX);

    sprintf(buffer, "%4dmO", resistanceMilliOhm);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(0, tft.getCursorY());
    tft.print(buffer);
    tft.setCursor(tft.getCursorX() - getCharWidth(), tft.getCursorY());
    tft.write(0xEA);

    uint16_t minutes = floor(secondsSpent / 60);
    uint16_t seconds = secondsSpent % 60;
    sprintf(buffer, "%2d:%02d", minutes, seconds);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(getAlignedX(buffer, Alignment::Right), tft.getCursorY() + LINE_MARGIN_PX);
    tft.println(buffer);
}
uint16_t getCharWidth(){
    int16_t x = 0, y = 0;
    uint16_t w, h;

    tft.getTextBounds("a", 0, 0, &x, &y, &w, &h);

    return w;
}

uint16_t getAlignedX(char * string, Alignment alignment = Alignment::Left){
    int16_t boundsX, boundsY;
    uint16_t boundsW, boundsH;
    tft.getTextBounds(buffer, 0, 0, &boundsX, &boundsY, &boundsW, &boundsH);


    switch (alignment)
    {
    case Alignment::Right:
        return tft.width() - boundsW;
        break;
    case Alignment::Center:
        return tft.width() / 2 - boundsW / 2;
        break;
    
    default:
        return boundsX;
        break;
    }
}
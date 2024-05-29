#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <ArduinoTrace.h>

#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8
#define TEXT_SIZE 1

enum Alignment {
    Left, Right, Center
};
uint16_t getAlignedX(char * buffer, Alignment alignment);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
unsigned int const charWidthRatio = 8;
unsigned int const charHeightRatio = 16;

void setup()
{
    Serial.begin(9600);

    // standard charset is CP437, fix for the old library bug
    tft.cp437(true);
    tft.initR(INITR_BLACKTAB);   
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(2);
    tft.setTextSize(TEXT_SIZE);
}

float voltsNow = 0;
float targetMinVolts = 3.3;
unsigned int currentMilliAmp = 0;
unsigned int milliAmpsHour = 0;

char buffer[10];
void loop()
{
    voltsNow += 0.5;
    currentMilliAmp += 100;

    sprintf(buffer, "%-6s", (String(voltsNow, 2) + 'v').c_str());
    tft.setTextColor(ST77XX_RED, ST7735_CYAN);
    tft.setCursor(0, 0);
    tft.print(buffer);
   

    sprintf(buffer, "%4dmA", currentMilliAmp);
    tft.setTextColor(ST77XX_RED, ST7735_BLUE);
    tft.setCursor(getAlignedX(buffer, Alignment::Right), 0);
    tft.println(buffer);

    sprintf(buffer, "%5d mAh", milliAmpsHour);
    tft.setTextColor(ST77XX_RED, ST7735_BLUE);
    tft.setCursor(getAlignedX(buffer, Alignment::Center), tft.getCursorY() + 4);
    tft.print(buffer);

    delay(250);
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
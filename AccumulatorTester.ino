#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <ArduinoTrace.h>

#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8

void drawNoise();

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(9600);

    // standard charset is CP437, fix for the old library bug
    tft.cp437(true);
    tft.initR(INITR_BLACKTAB);   
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(2);
/*
    drawNoise(); 
    tft.drawLine(0, 0, 128, 160, ST77XX_GREEN);
    tft.drawFastHLine(100, 100, -50, ST77XX_MAGENTA);
    tft.drawFastVLine(100, 100, -50, ST7735_MAGENTA);
    tft.fillRect(50, 50, 20, 20, 0x0137);
    tft.drawRect(60, 60, 8, 8, ST7735_BLACK);

    int charHeight = 8;
    tft.drawChar(50, 50, 0x03, 0x9051, 0xcccc, 1);
    tft.drawChar(50, 50 + charHeight, 0x04, 0x9051, 0xcccc, 2);
    tft.drawChar(50, 50 + charHeight * 3, 0x05, 0x9051, 0xcccc, 3);

    delay(1000);
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_RED, ST7735_BLACK);
*/


    tft.setTextSize(1);
}

float voltsNow = 0;
unsigned int currentMilliAmp = 800;
char buffer[10];
void loop()
{
    tft.setCursor(0, 0);

    voltsNow += 0.5;
    currentMilliAmp += 100;

    sprintf(buffer, "%-6s", (String(voltsNow, 2) + 'V').c_str());
    tft.setTextColor(ST77XX_RED, ST7735_CYAN);
    tft.print(buffer);
   

    sprintf(buffer, "%4dmA", currentMilliAmp);
    int16_t boundsX, boundsY;
    uint16_t boundsW, boundsH;
    tft.getTextBounds(buffer, 0, 0, &boundsX, &boundsY, &boundsW, &boundsH);
    int upperRightSignX = tft.width() - boundsW;
    int upperRightSignY = 0;
    tft.setCursor(upperRightSignX, upperRightSignY);

    tft.setTextColor(ST77XX_RED, ST7735_BLUE);
    tft.print(buffer);

    delay(250);
}

void drawNoise(){
    int pixelsMax = 1000;
    int offset = 10;
    for(int i = 0; i < pixelsMax; i++){
        tft.drawPixel(
            random(offset, pixelsMax / 10 + offset), 
            random(offset, pixelsMax / 10 + offset), 
            0x632C
        );
    }
}

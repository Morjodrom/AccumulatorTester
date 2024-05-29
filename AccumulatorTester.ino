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
    tft.setTextSize(1);
}

float voltsNow = 0;
unsigned int currentMilliAmp = 800;
char buffer[10];
void loop()
{
    voltsNow += 0.5;
    currentMilliAmp += 100;

    sprintf(buffer, "%-6s", (String(voltsNow, 2) + 'V').c_str());
    tft.setTextColor(ST77XX_RED, ST7735_CYAN);
    tft.setCursor(0, 0);
    tft.print(buffer);
   

    sprintf(buffer, "%4dmA", currentMilliAmp);
    tft.setTextColor(ST77XX_RED, ST7735_BLUE);
    tft.setCursor(getRightAlignedX(buffer), 0);
    tft.print(buffer);

    delay(250);
}

uint16_t getRightAlignedX(char * string){
    int16_t boundsX, boundsY;
    uint16_t boundsW, boundsH;
    tft.getTextBounds(buffer, 0, 0, &boundsX, &boundsY, &boundsW, &boundsH);
    return tft.width() - boundsW;
}


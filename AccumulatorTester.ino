#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8

void drawNoise();

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    // standard charset is CP437, fix for the old library bug
    tft.cp437(true);
    tft.initR(INITR_BLACKTAB);   
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(2);

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



   
}

float voltageNow = 0;
float currentNow = 0;
char buffer[7];
void loop()
{
    tft.setCursor(0, 0);

    voltageNow += 0.5;
    currentNow += 0.5;

    sprintf(buffer, "%-6s", (String(voltageNow) + "V").c_str());
    tft.print(buffer);

    sprintf(buffer, "%6s", (String(currentNow) +  "A").c_str());
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

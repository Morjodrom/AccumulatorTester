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
#define DISPLAY_FRAME_LENGTH 1000

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

const int loads[] = {
    50,
    500,
    1000
};

const uint8_t loadPins[] = {
    2,
    3,
    4
};

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

    for(int i = 0; i < ARRAY_SIZE(loadPins); i++){
        pinMode(loadPins[i], OUTPUT);
        digitalWrite(loadPins[i], LOW);
    }
}

float voltsNow = 0;
float targetMinVolts = 3.3;
unsigned int currentMilliAmp = 0;
unsigned int milliAmpsHour = 0;
unsigned int secondsSpent = 0;
unsigned int resistanceMilliOhm = 150;

unsigned long lastMillis = 0;
int sensorsPollingTimeout = 0;
int displayUpdateTimeout = 0;
unsigned int chosenLoadIndex = 0;
unsigned int activeLoadIndex = chosenLoadIndex;

enum Menu {
    Cancel = 0,
    MinVoltage = 1,
    Load = 2,
    Overflow
};

Menu menuSelected = Menu::Cancel;
Menu editMode = Menu::Cancel;

void loop()
{
    unsigned long millisUpdate = millis() - lastMillis;
    lastMillis = millis();

    if(encoder.tick()){        
        if(encoder.click()){
            handleEncoderClick();
        }

        if(encoder.turn()){            
            if(editMode == Menu::Cancel && menuSelected > Menu::Cancel && menuSelected < Menu::Overflow){
                int menuSelectedChanged = menuSelected + encoder.dir();
                menuSelectedChanged = constrain(menuSelectedChanged, Menu::Cancel, Menu::Overflow);
                menuSelected = static_cast<Menu>(menuSelectedChanged);
            }

            if(editMode == Menu::MinVoltage){
                targetMinVolts += 0.1 * encoder.dir();
            }

            if(editMode == Menu::Load){
                short loadsMaxIndex = sizeof(loads) / sizeof(loads[0]) - 1;
                chosenLoadIndex += encoder.dir();
                chosenLoadIndex = constrain(chosenLoadIndex, 0, loadsMaxIndex);
            }
        }

        DUMP(menuSelected);
    }

    sensorsPollingTimeout -= millisUpdate;
    if(sensorsPollingTimeout <= 0){
        sensorsPollingTimeout = SENSORS_POLLING_PERIOD_MS;

        voltsNow += 0.5;
        currentMilliAmp += 100;
        secondsSpent = floor(millis() / 1000);
    }

    displayUpdateTimeout -= millisUpdate;
    if(displayUpdateTimeout <= 0){
        displayUpdateTimeout = DISPLAY_FRAME_LENGTH;
        updateDisplay();
    }

    for(int i = 0; i < ARRAY_SIZE(loadPins); i++){
        digitalWrite(loadPins[i], LOW);
    }
    digitalWrite(loadPins[activeLoadIndex], HIGH);
}

void updateDisplay()
{
    char buffer[10];

    // voltage sensor
    sprintf(buffer, "%-6s", (String(voltsNow, 2) + 'v').c_str());
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.print(buffer);

    // target volts
    sprintf(buffer, ">%5s", (String(targetMinVolts, 1) + 'v').c_str());

    if(editMode == Menu::MinVoltage){
        tft.setTextColor(ST7735_CYAN, ST7735_RED); 
    }
    else if(menuSelected == Menu::MinVoltage){
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE);       
    }
    else {
        tft.setTextColor(0x07FE, ST7735_BLACK);
    }
    tft.setCursor(getAlignedX(buffer, Alignment::Right), 0);
    tft.print(buffer);

    tft.println();
    tft.setCursor(0, tft.getCursorY() + LINE_MARGIN_PX);

    // current sensor
    sprintf(buffer, "%4dmA", currentMilliAmp);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(0, tft.getCursorY());
    tft.print(buffer);

    // mAh calculation
    sprintf(buffer, "%dmAh", milliAmpsHour);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(getAlignedX(buffer, Alignment::Right), tft.getCursorY());
    tft.print(buffer);

    tft.println();
    tft.setCursor(0, tft.getCursorY() + LINE_MARGIN_PX);

    // resistance sensor
    sprintf(buffer, "%4dm ", resistanceMilliOhm);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(0, tft.getCursorY());
    tft.print(buffer);
    tft.setCursor(tft.getCursorX() - getCharWidth(), tft.getCursorY());
    tft.write(0xEA);

    // load
    sprintf(buffer, "%4dm ", loads[chosenLoadIndex]);
    if(editMode == Menu::Load){
        tft.setTextColor(ST7735_CYAN, ST7735_RED); 
    } 
    else if(menuSelected == Menu::Load){
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE);       
    } 
    else {
        tft.setTextColor(0x07FE, ST7735_BLACK);
    }
    tft.setCursor(getAlignedX(buffer, Alignment::Right), tft.getCursorY());
    tft.print(buffer);
    tft.setCursor(tft.getCursorX() - getCharWidth(), tft.getCursorY());
    tft.write(0xEA);

    tft.println();
    tft.setCursor(0, tft.getCursorY() + LINE_MARGIN_PX);

    // timer
    uint16_t minutes = floor(secondsSpent / 60);
    uint16_t seconds = secondsSpent % 60;
    sprintf(buffer, "%2d:%02d", minutes, seconds);
    tft.setTextColor(0x07FE, ST7735_BLACK);
    tft.setCursor(getAlignedX(buffer, Alignment::Left), tft.getCursorY());
    tft.println(buffer);
}

void handleEncoderClick(){   
    bool isMenuActivation = menuSelected == Menu::Cancel;
    if(isMenuActivation){                
        menuSelected = Menu::MinVoltage;
        return;
    } 

    bool isSaveLoad = editMode == Menu::Load;
    if(isSaveLoad){
        activeLoadIndex = chosenLoadIndex;
        exitMenu();
        return;
    } 
    
    bool isSave = editMode > Menu::Cancel && editMode < Menu::Overflow;
    if(isSave){
        exitMenu();
        return;
    }

    bool isMenuChoice = menuSelected > Menu::Cancel && menuSelected < Menu::Overflow;
    if(isMenuChoice){
        activateMenu();
        return;
    }
}

void exitMenu()
{
    editMode = Menu::Cancel;
    menuSelected = Menu::Cancel;
}

void activateMenu(){
    editMode = menuSelected;
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
    tft.getTextBounds(string, 0, 0, &boundsX, &boundsY, &boundsW, &boundsH);


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
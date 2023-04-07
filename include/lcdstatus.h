#ifndef LCDSTATUS_H
#define LCDSTATUS_H

#include "main.h"

class LcdStatus
{
private:
    State *status;   
    uint8_t drawColorToggle=0; 
    const uint8_t* stdFont = u8g2_font_squirrel_tr;
    bool toggle = true;
    uint8_t ants=0; 
    
    //u8g2_font_ncenB12_tr
    //u8g2_font_astragal_nbp_t
    
public:
    LcdStatus(State* state);


//init LCD - schould be late in setup()
void setupLCD();

//refresh full Screen with all fields
void updateFullScreen();

//refresh only Header (Solarwatt, Powerwatt, Batt)
void updateHeaderStatus();

void drawArrows(uint8_t x, uint8_t y, bool reverse);
void drawX(uint8_t x, uint8_t y);

};

#endif
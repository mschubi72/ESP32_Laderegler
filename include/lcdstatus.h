#ifndef LCDSTATUS_H
#define LCDSTATUS_H

#include "main.h"

class LcdStatus
{
private:
    State *status;   
    uint8_t drawColorBat=0; 
    const uint8_t* stdFont = u8g2_font_squirrel_tr;
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
};

#endif
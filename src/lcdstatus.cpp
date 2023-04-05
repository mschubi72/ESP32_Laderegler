#include <lcdstatus.h>

using namespace std;

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, PIN_OLED_SCL, PIN_OLED_SDA);

LcdStatus::LcdStatus(State *state)
{
  status = state;
}

void LcdStatus::setupLCD()
{
  u8g2.begin();
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.drawStr(6, 12, "Hello Schubi :)");
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(6, 27, "ESP Lader ready...");
  u8g2.drawStr(6, 47, String("IP: " + WiFi.localIP().toString()).c_str());
  u8g2.drawStr(6, 59, __DATE__);
  u8g2.sendBuffer();
  Serial.println("LcdStatus initialized...");
    Serial.printf("\tSCL Pin: %i, SDA Pin: %i \n", PIN_OLED_SCL, PIN_OLED_SDA);
}

void LcdStatus::updateFullScreen()
{
}

// x(0,127);y(0,15)
void LcdStatus::updateHeaderStatus()
{
  u8g2.setClipWindow(0, 0, 128, 16);
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 128, 16);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_unifont_t_weather);
  u8g2.drawGlyph(0, 15, 0x002e); // Sonne  34
  u8g2.setFont(stdFont);
  u8g2.drawStr(16, 15, String(status->solarPower).c_str());

  int w = 16 + 2 + u8g2.getStrWidth(String(status->solarPower).c_str());
  u8g2.setFont(u8g2_font_unifont_t_77);
  u8g2.drawGlyph(w, 15, 0x269b); // Atom
  u8g2.setFont(stdFont);
  w = w + 16 + 1;
  u8g2.drawStr(w, 15, String(status->flatPower).c_str());

  w = w + 1 + u8g2.getStrWidth(String(status->flatPower).c_str());
  u8g2.setFont(u8g2_font_battery19_tn);
  status->batStatus = (status->batStatus + 1) % 6;
  status->batVoltage = 1.0 + status->batStatus;
  u8g2.drawGlyph(w, 19, 0x0030 + status->batStatus); // Bat

  w = w + 8;
  u8g2.setFont(stdFont);
  u8g2.drawStr(w, 15, String(status->batVoltage).c_str());

  if (status->chargePower > 0)
  {
    u8g2.setFont(u8g2_font_battery19_tn);
    drawColorBat = (drawColorBat + 1) % 2;
    u8g2.setDrawColor(drawColorBat);
    u8g2.drawGlyph(128 - 7, 19, 0x0036); // Bat charge
  }
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(6, 59, __DATE__);
  u8g2.sendBuffer();
  // Serial.println("uHS ...");
}

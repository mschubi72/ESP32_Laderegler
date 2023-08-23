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
  u8g2.drawStr(6, 12, "Hello Schubi");
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(6, 27, "ESPLader ready");
  u8g2.drawStr(6, 44, String("IP: " + WiFi.localIP().toString()).c_str());
  u8g2.setFont(stdFont);
  u8g2.drawStr(6, 59, String(String("Ver ") + __DATE__).c_str());
  u8g2.sendBuffer();
  Serial.println("LcdStatus initialized...");
  Serial.printf("\tSCL Pin: %i, SDA Pin: %i \n", PIN_OLED_SCL, PIN_OLED_SDA);
}

void LcdStatus::drawArrows(uint8_t x, uint8_t y, bool reverse)
{
  uint8_t offset = 0;
  if (reverse)
    offset = 1;
  if (toggle)
    offset += 12;

  u8g2.setFont(u8g2_font_siji_t_6x10);
  u8g2.drawGlyph(x, y, 0xe062 + offset);
  u8g2.drawGlyph(x + 10, y, 0xe062 + offset);
  u8g2.drawGlyph(x + 20, y, 0xe062 + offset);
}

void LcdStatus::drawX(uint8_t x, uint8_t y)
{
  // u8g2.setDrawColor(drawColorToggle);
  u8g2.setFont(u8g2_font_siji_t_6x10);
  u8g2.drawGlyph(x + 3, y, 0x002d);
  u8g2.drawGlyph(x + 10, y, 0x0058);
  u8g2.drawGlyph(x + 20 - 3, y, 0x002d);
  // u8g2.setDrawColor(1);
}

void LcdStatus::updateFullScreen()
{
  updateHeaderStatus();

  u8g2.setClipWindow(0, 16, 128, 63);
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 16, 128, 63);
  u8g2.setDrawColor(1);

  u8g2.setFont(u8g2_font_ncenB08_tr);
  //int x = u8g2.getStrWidth(status->formattedTime);
  //x = (128 - x) / 2;
  u8g2.drawStr(0, 63, status->formattedTime);
  u8g2.drawStr(90,52, String(status->batPercent).c_str());

  u8g2.setFont(u8g2_font_unifont_t_weather);
  u8g2.drawGlyph(0, 40, 0x002e); // Sonne  34
                                 // drawArrows(18,38,false);
  if (status->solarPower > 1) // Solarpower 
  {
    drawArrows(18, 38, false);
  }
  else
  {
    drawX(18, 38);
  }

  u8g2.setFont(u8g2_font_unifont_t_77);
  u8g2.drawGlyph(50, 40, 0x269b); // Atom
  if (status->chargePower > 1) // charge battery
  {
    drawArrows(66, 38, false);
    u8g2.setFont(u8g2_font_battery19_tn);
    u8g2.setDrawColor(1);
    u8g2.drawGlyph(96, 40, 0x0036); // Bat charge
  }
  else if (status->feedPowerBat < 0) // feed from battery
  {
    drawArrows(66, 38, true);
    u8g2.setFont(u8g2_font_battery19_tn);
    u8g2.setDrawColor(1);
    u8g2.drawGlyph(96, 40, 0x0030 + status->batStatus); // Bat
  }
  else
  {
    drawX(70, 38);
    u8g2.setFont(u8g2_font_battery19_tn);
    u8g2.setDrawColor(1);
    u8g2.drawGlyph(96, 42, 0x0030 + status->batStatus); // Bat
  }

  // u8g2.setFont(u8g2_font_unifont_t_77);
  // u8g2.drawGlyph(96, 40, 0x26fd); // Tankstelle/Akku

  u8g2.sendBuffer();
}

void LcdStatus::updateHeaderStatus()
{
  toggle = !toggle;
  drawColorToggle = (drawColorToggle + 1) % 2;

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
  if (status->batStatus == 0)
  { // empty
    u8g2.setDrawColor(drawColorToggle);
  }
  else
  {
    u8g2.setDrawColor(1);
  }
  //(status->batStatus + 1) % 6;
  u8g2.drawGlyph(w, 20, 0x0030 + status->batStatus); // Bat

  u8g2.setDrawColor(1);
  w = w + 8;
  u8g2.setFont(stdFont);
  u8g2.drawStr(w, 15, String(status->batVoltage).c_str());

  u8g2.sendBuffer();
}

#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define POWER 5
#define TEMP_SENSOR_PIN 6
#define TEMP_DOWN 7
#define ON_OFF_PIN 8
#define TEMP_UP 9

OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

float Temperature = 0.0;
float MaxTemperature = 33;

int is_program_on = 0;
int is_heating = 0;

long temp_hold_started_at = 0;
long temp_hold_end_time = 0;

int addr = 0;

void setup()
{
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, LOW);

  MaxTemperature=EEPROM.read(addr);
  if (MaxTemperature==0)
  {
    MaxTemperature = 33;
    EEPROM.write(addr, MaxTemperature );
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1.9);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();

  sensors.begin();
}

void loop()
{
  sensors.requestTemperatures();
  readOnOff();
  readTempControlButtons();

  Temperature = sensors.getTempCByIndex(0);

  prepareScreen();

  if (is_program_on)
  {
    hold_heat();
  }
  else
  {
    heat_off();
  }

  display.print("Incubator ");
  if (is_program_on != 1)
  {
    display.println("OFF");
  }
  else
  {
    display.print("ON   ");
    float h = millis() / 3600000;
    if (h <= 9)
      display.print("0");
    display.print(h, 0);
    display.print(":");
    float m = (millis() % 3600000) / 60000;
    if (m <= 9)
      display.print("0");
    display.println(m, 0);
  }

  if (is_heating == 1)
  {
    display.println("Heating");
  }

  display.display();
}

void hold_heat()
{
  if (MaxTemperature - Temperature > 0.5)
    heat_on();
  else
    heat_off();
}

void heat_off()
{
  digitalWrite(POWER, LOW);
  is_heating = 0;
}

void heat_on()
{
  digitalWrite(POWER, HIGH);
  is_heating = 1;
}

void readOnOff()
{
  if (digitalRead(ON_OFF_PIN) == HIGH)
  {
    if (is_program_on)
    {
      is_program_on = 0;
    }
    else
    {
      is_program_on = 1;
    }
  }
}

void readTempControlButtons()
{
  if (digitalRead(TEMP_UP) == HIGH)
    MaxTemperature++;

  if (digitalRead(TEMP_DOWN) == HIGH)
    MaxTemperature--;

  EEPROM.write(addr, MaxTemperature);
}

void prepareScreen()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("T: ");
  display.print(Temperature);
  display.print(", Hold: ");
  display.println(MaxTemperature);
  display.println("---------------------");
}

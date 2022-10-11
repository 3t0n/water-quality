#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <unistd.h>
#include <DFRobot_PH.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define PH_PIN A1
#define ONE_WIRE_BUS 2
#define TRANSISTOR_SWITCH_PIN 3

#define ONE_SECOND (1000UL)
#define ONE_MINUTE (ONE_SECOND * 60UL)
#define ONE_HOUR (ONE_MINUTE * 60UL)

// Setup LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Setup pH meter
DFRobot_PH ph_meter;

// Setup temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp_meter(&oneWire);

float get_ph(float temperature) {
  const int max_samples = 10;
  float voltage = 0.00;
  float result = 0.00;

  // Turn sensors on
  turn_sensors(true);

  // Enable PH calibration from the console
  ph_meter.calibration(voltage, temperature);

  // Make several measurements
  for (int i = 0; i < max_samples; ++i) {
    // Get pin voltage and convert to pH
    voltage = analogRead(PH_PIN) / 1024.0 * 5000;

    // Convert voltage to pH with temperature compensation
    result += ph_meter.readPH(voltage, temperature);

    // Small delay
    delay(100U);
  }

  // Get average number
  result = result / max_samples;

  // Turn sensors off
  turn_sensors(false);

  // Oficial limits of the sensor
  if ((result < 0.00) || (result > 14.00)) {
    result = 7.00;
  }

  return result;
}

float get_temp() {
  const int max_samples = 10;
  float result = 0.00;

  // Turn sensors on
  turn_sensors(true);

  // Make several measurements
  for (int i = 0; i < max_samples; ++i) {
    // Request the temperature
    temp_meter.requestTemperatures();

    // There are could be several sensors on one wire
    result += temp_meter.getTempCByIndex(0);

    // Small delay
    delay(100U);
  }

  // Get average number
  result = result / max_samples;

  // Turn sensors off
  turn_sensors(false);

  // Oficial limits of the sensor
  if ((result < -55.00) || (result > 125.00)) {
    result = 20.00;
  }

  return result;
}

void turn_sensors(bool state) {
  if (state) {
    digitalWrite(TRANSISTOR_SWITCH_PIN, HIGH);
  } else {
    digitalWrite(TRANSISTOR_SWITCH_PIN, LOW);
  }
}

void lcd_print(float temp, float ph, long next) {
  // Print temperature
  lcd.setCursor(0, 1);
  lcd.print("TMP: ");
  lcd.print(temp);
  lcd.print("    ");

  // Print PH
  lcd.setCursor(0, 0);
  lcd.print("PH : ");
  lcd.print(ph);
  lcd.print("    ");

  // Print TDS
  // int tds = 0;
  // lcd.setCursor(11, 0);
  // lcd.print("TDS: ");
  // lcd.print(tds);

  // Print water level
  // bool water = false;
  // lcd.setCursor(11, 1);
  // lcd.print("WTR: ");
  // if (water) {
  //   lcd.print("HIGH");
  // } else {
  //   lcd.print("LOW");
  // }

  // Print status
  lcd.setCursor(0, 3);
  lcd.print("UPDATE IN: ");
  lcd.print(next);
  lcd.print("    ");
}

void serial_print(float temp, float ph) {
  Serial.print("temperature:");
  Serial.println(temp);

  Serial.print("pH: ");
  Serial.println(ph);

  // Serial.print("TDS:");
  // Serial.println(tds);

  // Serial.print("WTR: ");
  // Serial.println(water);
}

void setup() {
  // Init LCD
  lcd.init();
  lcd.backlight();

  // Init PH sensor
  Serial.begin(115200);
  ph_meter.begin();

  // Init temperature sensor
  temp_meter.begin();

  // Setup transistor pin
  pinMode(TRANSISTOR_SWITCH_PIN, OUTPUT);
}

void loop() {
  static unsigned long timepoint = millis();
  static float temperature = get_temp();
  static float ph = get_ph(temperature);

  // Wait 1 hour for the next measuring
  unsigned long timer = millis() - timepoint;
  long next = (ONE_HOUR - timer) / 1000UL;
  if (timer >= ONE_HOUR) {
    timepoint = millis();
    next = 0;

    // Get data
    temperature = get_temp();
    ph = get_ph(temperature);
  }

  // Print measurements
  lcd_print(temperature, ph, next);
  serial_print(temperature, ph);

  // Delay for 1 second
  delay(1000U);
}
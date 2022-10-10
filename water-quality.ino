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

// Setup LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Setup pH meter
DFRobot_PH ph_meter;

// Setup temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp_meter(&oneWire);

void setup() {
  // Init LCD
  lcd.init();
  lcd.backlight();

  // Init PH sensor
  Serial.begin(115200);
  ph_meter.begin();

  // Init temperature sensor
  temp_meter.begin();

  pinMode(TRANSISTOR_SWITCH_PIN, OUTPUT);
}

void loop() {
  turn_sensors(true);

  // Get data
  float temperature = get_temp();
  float ph = get_ph(temperature);

  // Print temperature
  lcd.setCursor(0, 1);
  lcd.print("TMP: ");
  lcd.print(temperature);

  Serial.print("temperature:");
  Serial.println(temperature);

  // Print PH
  lcd.setCursor(0, 0);
  lcd.print("PH : ");
  lcd.print(ph);

  Serial.print("pH: ");
  Serial.println(ph);

  // Print TDS
  int tds = 0;
  lcd.setCursor(11, 0);
  lcd.print("TDS: ");
  lcd.print(tds);

  Serial.print("TDS:");
  Serial.println(tds);

  // Print water level
  bool water = false;
  lcd.setCursor(11, 1);
  lcd.print("WTR: ");
  if (water) {
    lcd.print("HIGH");
  } else {
    lcd.print("LOW");
  }

  Serial.print("WTR: ");
  Serial.println(water);

  // Print status
  lcd.setCursor(0, 3);
  lcd.print("MSG: ");
  lcd.print("STATUS TEXT");

  turn_sensors(false);

  // Delay for 1 hour
  delay(1000U * 60 * 60);
}

float get_ph(float temperature) {
  //static unsigned long timepoint = millis();
  float voltage = 0.00;
  float result = 0.00;

  // if (millis() - timepoint > 1000U) {  // time interval: 1s
  //   timepoint = millis();

  voltage = analogRead(PH_PIN) / 1024.0 * 5000;    // read the voltage
  result = ph_meter.readPH(voltage, temperature);  // convert voltage to pH with temperature compensation
  //}

  // Enable PH calibration from the console
  ph_meter.calibration(voltage, temperature);  // calibration process by Serial CMD

  // Oficial limits of the sensor
  if ((result < 0.00) || (result > 14.00)) {
    result = 7.00;
  }

  return result;
}

float get_temp() {
  float result = 0.00;

  // Request the temperature
  temp_meter.requestTemperatures();

  // There are could be several sensors on one wire
  result = temp_meter.getTempCByIndex(0);

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
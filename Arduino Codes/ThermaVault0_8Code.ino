//Hello There! :D

#include "DHT.h"
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <string.h>

float tempsetting = 0;
float serialValue = -1;
bool serialActive = false;
bool relay2State = false;
unsigned long lastRelayToggle = 0;
const unsigned long onTime = 1000;
const unsigned long offTime = 2000;

#define LEDPIN 2
#define LEDNUM 12
const int POT = A0;
const int DHT_PIN = 12;
const int NTC_PIN1 = A2;
const int RELAY1 = 10;
const int RELAY2 = 9;
const int PEIZO = 13;
const int VOLT = A1;
const float ADC_REF = 5.0;
const int   ADC_MAX = 1023;
const float dividerRatio = 5.0;
const float alpha = 0.4;
float smoothedPercent = -1.0;

struct VPoint { float v; int pct; };
const int NPTS = 9;

#define DHTTYPE DHT11
DHT currtemp(DHT_PIN, DHTTYPE);
Adafruit_NeoPixel pixels(LEDNUM, LEDPIN);

#define NOTE_C4 262
#define NOTE_G3 196
#define NOTE_A3 220
#define NOTE_B3 247

int melody[] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };

VPoint table[NPTS] = {
  {12.90, 100},
  {12.70, 90},
  {12.62, 80},
  {12.42, 70},
  {12.20, 60},
  {12.06, 50},
  {11.90, 30},
  {11.70, 10},
  {11.60, 0}
};

const float R_FIXED = 9650.0;
const float BETA = 3950.0;
const float T0 = 298.15;
const float R0 = 10020.0;
const int samples = 10;

int lastPotVal = 0;
unsigned long lastMoveTime = 0;
bool adjusting = false;

void clearAll() {
  for (int i = 0; i < LEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

void starting_sequence() {
  pixels.begin();
  for (int i = 0; i < LEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 200, 255));
    pixels.show();
    delay(100);
  }
  for (int i = 0; i < LEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(200, 0, 255));
    pixels.show();
    delay(100);
  }
  for (int i = 0; i < LEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 0));
    pixels.show();
    delay(100);
  }
  for (int i = 0; i <= LEDNUM / 2; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    pixels.setPixelColor(LEDNUM - i, pixels.Color(255, 255, 255));
    pixels.show();
    delay(100);
  }
}

float readBatteryVoltage() {
  long sum = 0;
  const int reps = 40;

  for (int i = 0; i < reps; i++) {
    sum += analogRead(VOLT);
    delayMicroseconds(250);
  }

  float raw = sum / (float)reps;

  float vsensor = (raw * ADC_REF) / ADC_MAX;
  float vbatt = vsensor * dividerRatio;
  return vbatt;
}

int voltageToPercent(float v){
  if (v >= table[0].v) return 100;
  if (v <= table[NPTS-1].v) return 0;

  for (int i = 0; i < NPTS-1; ++i) {
    if (v <= table[i].v && v >= table[i+1].v) {
      float v1 = table[i].v;
      float v2 = table[i+1].v;
      int p1 = table[i].pct;
      int p2 = table[i+1].pct;

      float t = (v - v2) / (v1 - v2);
      return round(p2 + t * (p1 - p2));
    }
  }
  return 0;
}

void coolingAlert() {
  tone(PEIZO, 800, 150);
  delay(180);
  tone(PEIZO, 1040, 150);
  delay(180);
  tone(PEIZO, 1320, 200);
  delay(200);
  noTone(PEIZO);
}

void startupMelody() {
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(PEIZO, melody[thisNote], noteDuration);
    delay(noteDuration * 1.30);
    noTone(PEIZO);
  }
}

void StartupSequence() {
  startupMelody();
  delay(400);
}

void coolingAnimation() {
  static int coolPos = 0;
  pixels.clear();
  for (int i = 0; i < LEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 40, 0));
  }
  int p1 = coolPos % LEDNUM;
  int p2 = (coolPos + 1) % LEDNUM;
  int p3 = (coolPos + 2) % LEDNUM;
  pixels.setPixelColor(p1, pixels.Color(200, 0, 255));
  pixels.setPixelColor(p2, pixels.Color(200, 0, 255));
  pixels.setPixelColor(p3, pixels.Color(200, 0, 255));
  pixels.show();
  coolPos++;
}

float readNTC() {
  float voltageSum = 0;
  for (int i = 0; i < samples; i++) {
    voltageSum += analogRead(NTC_PIN1);
    delay(10);
  }
  float raw = voltageSum / samples;
  float Vout = raw * (5.0 / 1023.0);
  float R_thermistor = R_FIXED * (5.0 / Vout - 1.0);
  float tempK = 1.0 / ((1.0 / T0) + (1.0 / BETA) * log(R_thermistor / R0));
  float tempC = tempK - 273.15;
  return tempC;
}

void potProgressBar(int potVal) {
  int ledCount = map(potVal, 0, 1023, 0, LEDNUM);
  ledCount = LEDNUM - ledCount;
  for (int i = 0; i < LEDNUM; i++) {
    if (i < ledCount) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 255));
    } else {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
  }
  pixels.show();
}

void idleBreathing() {
  static int animationPos = 0;
  pixels.clear();
  int p1 = animationPos % LEDNUM;
  int p2 = (animationPos + 1) % LEDNUM;
  int p3 = (animationPos + (LEDNUM / 2)) % LEDNUM;
  int p4 = (p3 + 1) % LEDNUM;
  pixels.setPixelColor(p1, pixels.Color(128, 0, 255));
  pixels.setPixelColor(p2, pixels.Color(0, 255, 255));
  pixels.setPixelColor(p3, pixels.Color(0, 255, 0));
  pixels.setPixelColor(p4, pixels.Color(255, 255, 0));
  pixels.show();
  animationPos++;
  delay(80);
}

void setup() {
  pinMode(POT, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(PEIZO, OUTPUT);
  pinMode(NTC_PIN1, INPUT);
  Serial.begin(9600);
  serialActive = false;
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  currtemp.begin();
  starting_sequence();
  StartupSequence();
  delay(1000);
  clearAll();
}

void loop() {
  float vbatt = readBatteryVoltage();
  int pct = voltageToPercent(vbatt);

  if (smoothedPercent < 0) smoothedPercent = pct; // init first sample
  smoothedPercent = alpha * pct + (1 - alpha) * smoothedPercent;

  float Hum = currtemp.readHumidity();
  float DHTtemp = currtemp.readTemperature();
  float NTCtemp = readNTC();

  if (Serial.available() > 0) {
    String input = Serial.readString();
    //input.trim();
    if (input.equalsIgnoreCase("H")) {
      serialActive = false;
    } else {
      float val = input.toFloat();
      if (input.length() > 0) {
        serialValue = val;
        serialActive = true;
      }
    }
  }
  if (serialActive) {
    tempsetting = serialValue;
  } else {
    float a = analogRead(POT);
    tempsetting = map(a, 0, 1023, -10, 30);
  }

  float CombinedTemp = (NTCtemp * 0.94) + (DHTtemp * 0.06);
  float currentTemp = CombinedTemp;

 if (tempsetting < currentTemp) { //Main Cooling Logic
  digitalWrite(RELAY1, LOW);

  unsigned long now = millis();
  coolingAlert();

  if (relay2State) {
    if (now - lastRelayToggle >= onTime) {
      relay2State = false;
      lastRelayToggle = now;
      digitalWrite(RELAY2, HIGH);
    }
  } else {
    if (now - lastRelayToggle >= offTime) {
      relay2State = true;
      lastRelayToggle = now;
      digitalWrite(RELAY2, LOW);
    }
  }

} else {
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  relay2State = false;
}

  int potVal = analogRead(POT);
  if (abs(potVal - lastPotVal) > 20) {
    adjusting = true;
    lastMoveTime = millis();
    potProgressBar(potVal);
    lastPotVal = potVal;
  } else {
    if (millis() - lastMoveTime > 2000) {
      adjusting = false;
    }
  }

  if (!adjusting) {
    if (tempsetting < currentTemp) {
      coolingAnimation();
    } else {
      idleBreathing();
    }
  }

  String Bluetooth_Serial = String(smoothedPercent) + ";" + String(CombinedTemp) + ";" + String(Hum);
  Serial.println(Bluetooth_Serial);
  delay(40);
}
//Thenku! :D

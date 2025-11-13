#include <LiquidCrystal.h>
#include "DHT.h"

const int POT = A0;
const int TEMPSENS = 10;
#define DHTTYPE DHT11
const int RELAY1 = 12;
const int RELAY2 = 11;
const int rgbG = 8;
const int rgbB = 9;
const int LEDR = 7;
const int PEIZO = 13;

//LCD connections: RS=A1, EN=D6, D4=D2, D5=D3, D6=D4, D7=D5
LiquidCrystal lcd(A1, 6, 2, 3, 4, 5);

DHT currtemp(TEMPSENS, DHTTYPE);

//Startup Melody :D
#define NOTE_C4  262
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247

int melody[] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };

//Functions
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

void setup() {
  pinMode(POT, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(rgbG, OUTPUT);
  pinMode(rgbB, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(PEIZO, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  Serial.begin(9600);

  currtemp.begin();

  //LCD Initialization Phase
  lcd.begin(16, 2);
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ThermaVault v0.2");
  lcd.setCursor(0, 1);
  lcd.print("Hello World! :D");
  delay(2000);

  //LED Startup Animation
  digitalWrite(rgbB, HIGH); delay(1000);
  digitalWrite(rgbB, LOW);  digitalWrite(rgbG, HIGH); delay(1000);
  digitalWrite(rgbG, LOW);  digitalWrite(LEDR, HIGH); delay(1000);
  digitalWrite(LEDR, LOW);  delay(500);
  digitalWrite(rgbB, HIGH); digitalWrite(rgbG, HIGH); digitalWrite(LEDR, HIGH);
  startupMelody();
  delay(500);
  digitalWrite(rgbB, LOW); digitalWrite(rgbG, LOW); digitalWrite(LEDR, LOW);

  lcd.clear();
}

void loop() {
  float anvalue = analogRead(POT);
  float tempsetting = map(anvalue, 0, 1023, 0, 30);
  float Hum = currtemp.readHumidity();
  float Temp = currtemp.readTemperature();

  if (isnan(Hum) || isnan(Temp)) {
    lcd.clear();
    lcd.print("Sensor Failure!");
    Serial.println("Temp Sensor Failure...");
    delay(1000);
    return;
  }

  Serial.print("User Temperature Value: ");
  Serial.println(tempsetting);
  Serial.print("Current Temperature = ");
  Serial.println(Temp);
  Serial.print("Current Humidity = ");
  Serial.println(Hum);

lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Set:");
lcd.print((int)tempsetting);
lcd.print("C");

lcd.setCursor(8, 0);
lcd.print("Cur:");
lcd.print(Temp, 1);
lcd.print((char)223);
lcd.print("C");


  lcd.setCursor(0, 1);

  // Main Cooling Logic
  if (tempsetting < Temp) {
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY1, LOW);
    digitalWrite(rgbB, HIGH);
    digitalWrite(LEDR, HIGH);
    coolingAlert();
    lcd.print("Status: COOLING!");
    Serial.println("Cooling in Progress...");
  } 
  else {
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY1, HIGH);
    digitalWrite(rgbG, HIGH);
    noTone(PEIZO);
    lcd.print("Status: STANDBY");
  }

  delay(4000);
  digitalWrite(rgbB, LOW);
  digitalWrite(rgbG, LOW);
  digitalWrite(LEDR, LOW);
}

//The End! (For now :D)

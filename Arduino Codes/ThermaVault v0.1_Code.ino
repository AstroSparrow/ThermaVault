#include "DHT.h"

const int POT = A0;
const int TEMPSENS = 10;
#define DHTTYPE DHT11
const int RELAY = 12;
const int rgbG = 8;
const int rgbB = 9;
const int LEDR = 7;

DHT currtemp(TEMPSENS, DHTTYPE);

void setup() {
  pinMode(POT, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(rgbG, OUTPUT);
  pinMode(rgbB, OUTPUT);
  pinMode(LEDR, OUTPUT);
  digitalWrite(RELAY, HIGH);
  Serial.begin(9600);

  currtemp.begin();

  digitalWrite(rgbB, HIGH);
  delay(1000);
  digitalWrite(rgbB, LOW);
  digitalWrite(rgbG, HIGH);
  delay(1000);
  digitalWrite(rgbG, LOW);
  digitalWrite(LEDR, HIGH);
  delay(1000);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(rgbB, HIGH);
  digitalWrite(rgbG, HIGH);
  digitalWrite(LEDR, HIGH);
  delay(3000);
  digitalWrite(rgbB, LOW);
  digitalWrite(rgbG, LOW);
  digitalWrite(LEDR, LOW);
}

void loop() {
  float anvalue = analogRead(POT);
  float tempsetting = map(anvalue, 0, 1023, 0, 30);
  //Serial.print("POT Raw Value = ");
  //Serial.println(anvalue");  
  Serial.print("User Temperature Value: ");
  Serial.println(tempsetting);
  float Hum = currtemp.readHumidity();
  float Temp = currtemp.readTemperature();

  if (isnan(Hum) || isnan(Temp)){
    Serial.println("Temp Sensor Failure...");
    return;
  }

  Serial.print("Current Temperature = ");
  Serial.println(Temp);
  Serial.print("Current Humidity = ");
  Serial.println(Hum);

if (tempsetting < Temp) {
  digitalWrite(RELAY, LOW);
  digitalWrite(rgbG, HIGH);
  Serial.println("Cooling in Progress...");
}
  else{
    digitalWrite(RELAY, HIGH);
    digitalWrite(rgbB, HIGH);
    digitalWrite(LEDR, HIGH);
  };
delay(4000);
digitalWrite(rgbB, LOW);
digitalWrite(rgbG, LOW);
digitalWrite(LEDR, LOW);
}

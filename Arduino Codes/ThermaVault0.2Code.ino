//Hello There! :D

//Libraries for the Modules
#include <LiquidCrystal.h>
#include "DHT.h"

float tempsetting = 0;
float serialValue = -1;
bool serialActive = false;

//Pin Definitions
const int POT = A0;
const int DTH11 = 10;
const int NTC_PIN = A2;
const int RELAY1 = 12;
const int RELAY2 = 11;
const int rgbG = 8;
const int rgbB = 9;
const int LEDR = 7;
const int PEIZO = 13;

#define DHTTYPE DHT11
LiquidCrystal lcd(A1, 6, 2, 3, 4, 5);
DHT currtemp(DTH11, DHTTYPE);

//Startup Melody
#define NOTE_C4  262
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247

int melody[] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };

//Thermistor Constants
const float Rfixed = 10000.0;        // 10k resistor
const float nominalTemp = 25.0;      // 25°C reference
const float nominalRes = 10000.0;    // 10k thermistor at 25°C
const float beta = 3950.0;           // typical NTC B-value

//NTC Calibration
//Based on: DHT 23.60, Thermistor 27.60 → offset = -4.00
float ntc_offset = -4.00;

//Functions
void coolingAlert() {
  tone(PEIZO, 800, 150); delay(180);
  tone(PEIZO, 1040, 150); delay(180);
  tone(PEIZO, 1320, 200); delay(200);
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
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("ThermaVault v0.4");
  lcd.setCursor(0,1);
  lcd.print("Hello World! :D");
  delay(1000);

  digitalWrite(LEDR, HIGH);
  delay(1000);
  digitalWrite(LEDR, LOW);
  digitalWrite(rgbG, HIGH);
  delay(1000);
  digitalWrite(rgbG, LOW);
  digitalWrite(rgbB, HIGH);
  delay(1000);
  digitalWrite(rgbB, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  digitalWrite(rgbG, HIGH);
  digitalWrite(rgbB, HIGH);

  startupMelody();

  delay(400);
  digitalWrite(LEDR, LOW);
  digitalWrite(rgbG, LOW);
  digitalWrite(rgbB, LOW);
  lcd.clear();
}

//READ NTC FUNCTION (Steinhart) {ChatGPT Code Start}
float readNTC() {
  int adc = analogRead(NTC_PIN);

  float v = adc * (5.0 / 1023.0);
  float Rntc = (5.0 / v - 1.0) * Rfixed;

  float steinhart;
  steinhart = Rntc / nominalRes;                   
  steinhart = log(steinhart);                      
  steinhart /= beta;                                
  steinhart += 1.0 / (nominalTemp + 273.15);        
  steinhart = 1.0 / steinhart;                      
  steinhart -= 273.15;                              

  return steinhart + ntc_offset;
}
//{ChatGPT Code End}

void setup() {
  pinMode(POT, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(rgbG, OUTPUT);
  pinMode(rgbB, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(PEIZO, OUTPUT);
  pinMode(NTC_PIN, INPUT);
  Serial.begin(9600);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  currtemp.begin();

  StartupSequence();
}

void loop() {

  //Read Sensors
  float Hum = currtemp.readHumidity();
  float DHTtemp = currtemp.readTemperature();
  float NTCtemp = readNTC();

  if (Serial.available() > 0) {

    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.equalsIgnoreCase("P")) {
        serialActive = false;
        Serial.println("Serial Override Taken Back... Control returned to Potentiometer!");
    }
    else {
        float val = input.toFloat();
        if (input.length() > 0) {
            serialValue = val;
            serialActive = true;
            Serial.print("Serial Set Temperature Overriden! New Set Temp = ");
            Serial.print(serialValue);
            Serial.println(" C");
        }
    }
}
if (serialActive) {
    tempsetting = serialValue;
} else {
    float a = analogRead(POT);
    tempsetting = map(a, 0, 1023, -10, 30);
}

  if (isnan(Hum) || isnan(DHTtemp)) {
    lcd.clear();
    lcd.print("DHT Failure!");
    delay(1000);
    return;
  }

  float CombinedTemp = (NTCtemp * 0.9) + (DHTtemp * 0.1);  //Taking Thermistor more seriously than the DHT11

  //LCD Logic
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cur:");
  lcd.print(CombinedTemp, 1);
  lcd.print((char)223);

  lcd.setCursor(8, 0);
  lcd.print("C Set:");
  lcd.print((int)tempsetting);
  lcd.print(" ");

  //Cooling Logic
  float currentTemp = CombinedTemp;

  if (tempsetting < currentTemp) {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(rgbB, HIGH);
    digitalWrite(LEDR, HIGH);
    coolingAlert();
    lcd.setCursor(0, 1);
    lcd.print("Status: Cooling!");
  } else {
    digitalWrite(RELAY1, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Status: FSD Cooldown");
    delay(4000);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(rgbG, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Status: Standby...");
  }

  // Serial Monitor Stuff
  Serial.print("User Set Temp: ");
  Serial.print(tempsetting);
  Serial.print(" C | DHT Temp = ");
  Serial.print(DHTtemp);
  Serial.print(" C | NTC Temp = ");
  Serial.print(NTCtemp);
  Serial.print(" C | Biased Average Temp = ");
  Serial.print(CombinedTemp);
  Serial.print(" C | Humidity = ");
  Serial.println(Hum);

  delay(8000);
  digitalWrite(rgbB, LOW);
  digitalWrite(rgbG, LOW);
  digitalWrite(LEDR, LOW);
}

//Thenku! :D
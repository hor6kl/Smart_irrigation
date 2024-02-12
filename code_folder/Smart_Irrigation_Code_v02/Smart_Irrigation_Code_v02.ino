#include <Sleep_n0m1.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


int relayPin_mot = 7;
int relayPin_hum = 4;
int sensorPowerPin = 8;

int sensor_pin_hum = A0;

int output_value_hum ;
int starttime;

int buttonPin = 12;
int ledPin = 13;

Sleep sleep;
unsigned long sleepduration; /*duration for which the Arduino needs to be in sleep mode*/

// variables will change:
int buttonState = 0;


void setup ()

{

  Serial.begin(9600);
  pinMode(relayPin_mot, OUTPUT);
  pinMode(relayPin_hum, OUTPUT);

  pinMode(sensor_pin_hum, INPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  Serial.println ("Reading From the Sensor ...");


  lcd.init();           // initialize the lcd
  lcd.backlight();      // zapni podsvícení
  lcd.setCursor(4, 0);  // dej kurzor na sloupec 1, řádek 0
  lcd.print("ARDUINO"); // napis na displej "ARDUINO"
  delay(pauza);
  casovaZnacka = millis(); // vytvori casovou znacku

}



void loop() {

  output_value = analogRead(sensor_pin);

  output_value = map (output_value, 550, 10, 0, 100);

  Serial.print ("Moisture : ");
  Serial.print (output_value);
  Serial.print ("%");
  Serial.print('\n');

  digitalWrite (sensorPowerPin, HIGH);


// variables will change:
  if (output_value < 32){

    do {
      starttime = millis();
      delay(1250);

      output_value = analogRead(sensor_pin);
    
      output_value = map (output_value, 550, 10, 0, 100);
       
      digitalWrite (relayPin, LOW);

      Serial.print ("Moisture : ");
      Serial.print (output_value);
      Serial.print (" --> ON");
      Serial.print('\n');

      if (output_value < 65 && starttime > 100000){
        digitalWrite (relayPin, HIGH);
        Serial.print ("Reset the system, moisture does not reach defined value in 10s. ");
        while (true){
          digitalWrite(LED_BUILTIN, HIGH);
          delay(1000);
          digitalWrite(LED_BUILTIN, LOW);
          delay(1000);          
          }
        }

    } while (output_value < 65);

  }

  else

  {

    digitalWrite (relayPin, HIGH);

  }


}

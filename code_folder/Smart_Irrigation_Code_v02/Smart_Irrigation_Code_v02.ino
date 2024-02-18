#include <Sleep_n0m1.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // nastav adresu LDC na 0x27 pro 16 sloupcovy a 2 radkovy displej

// Pin variables
int relayPin_mot = 7;
int relayPin_hum = 4;

int sensor_pin_hum = A0;

int buttonPin = 12;
int ledPin = 13;


int output_value_hum ;

// variables will change:
int buttonState = 0;

// variables for humidity sensor
int water_value = 1000;
int dirt_value = 100;

// variables for time loop
const uint32_t pauza = 10000;  //10 s
unsigned long starttime;
unsigned long starttime_2;
unsigned long currenttime;
unsigned long end_time_2;

// Funtion for display

unsigned long casovaZnacka;

void obrazovka1(int hum, long allSeconds)
{

int runHours= allSeconds/3600;
int secsRemaining=allSeconds%3600;
int runMinutes=secsRemaining/60;
int runSeconds=secsRemaining%60;


  
  lcd.begin(16,2);
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.print("Vlhkost:");  
  lcd.setCursor(10, 0);
  lcd.print(hum);

  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.setCursor(2, 1);
  lcd.print(runHours);
  lcd.setCursor(5, 1);
  lcd.print("M:");  
  lcd.setCursor(7, 1);
  lcd.print(runMinutes);
  lcd.setCursor(10, 1);
  lcd.print("S:");   
  lcd.setCursor(12, 1);
  lcd.print(runSeconds); 
}

void obrazovka2()
{

    lcd.begin(16,2);
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("EMERGENCY");
    lcd.setCursor(4, 1);
    lcd.print("SHUT-DOWN");


}



void setup ()

{

  Serial.begin(9600);
  pinMode(relayPin_mot, OUTPUT);
  pinMode(relayPin_hum, OUTPUT);

  pinMode(sensor_pin_hum, INPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.println ("Reading From the Sensor ...");


  lcd.init();           // initialize the lcd
  lcd.backlight();      // zapni podsvícení
  lcd.setCursor(3, 0);  // dej kurzor na sloupec 1, řádek 0
  lcd.print("IRRIGATION"); // napis na displej "ARDUINO"
  delay(2000);
  starttime = millis(); // vytvori casovou znacku

}



void loop() {
  unsigned long allSeconds=millis()/1000;

  buttonState = digitalRead(buttonPin);


//  Serial.print ("Moisture : ");
//  Serial.print (output_value_hum);
//  Serial.print('\n');

//  Serial.print(currenttime);
  
  // time loop 
  if (millis() - currenttime > 10800000) {

//      digitalWrite (relayPin_mot, HIGH);
      digitalWrite (relayPin_hum, LOW); 

      delay(30000);
      output_value_hum = analogRead(sensor_pin_hum);


      if (output_value_hum > 430){
        
        starttime_2 = millis();
        end_time_2 = millis();
        
        do {
         
          delay(1250);

          output_value_hum = analogRead(sensor_pin_hum);
           
          digitalWrite (relayPin_mot, LOW);
          delay(2000);
          digitalWrite (relayPin_mot, HIGH);          

    
          Serial.print ("Moisture : ");
          Serial.print (output_value_hum);
          Serial.print (" --> ON");
          Serial.print('\n');

    
          if (output_value_hum > 350 && (end_time_2 - starttime_2) > 50000){
            digitalWrite (relayPin_mot, HIGH);
            digitalWrite (relayPin_hum, HIGH);
            
            Serial.print ("Reset the system, moisture does not reached defined value in 50s. ");
            while (true){
              digitalWrite(LED_BUILTIN, HIGH);
              delay(1000);
              digitalWrite(LED_BUILTIN, LOW);
              delay(1000);
              obrazovka2();          
              }
            }

        end_time_2 = millis();
            
        } while (output_value_hum > 350);
      }
    

    currenttime = millis();

      
  }
  else if (buttonState == 0){
    digitalWrite (relayPin_mot, LOW);

    obrazovka1(output_value_hum, currenttime/1000);
    
  }
  
  else {

      digitalWrite (relayPin_mot, HIGH);
      digitalWrite (relayPin_hum, HIGH); 
      
      obrazovka1(output_value_hum, currenttime/1000);
      delay(pauza);
     
  }



}

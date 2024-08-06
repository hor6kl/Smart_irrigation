#include <Sleep_n0m1.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // nastav adresu LDC na 0x27 pro 16 sloupcovy a 2 radkovy displej

// Pin variables definitions
int relayPin_light = 2;
int relayPin_hum = 4;
int relayPin_mot = 7;
int relayPin_fan = 8;
int relayPin_displ = 5;
const byte OC1A_PIN = 9;  // Pin for ventilator
const byte OC1B_PIN = 10; // Pin for ventilator
int sensor_pin_hum = A0;  // Humidity sensor pin
const int buttonPin_displ = 11;       // Button display Pin
const int buttonPin = 12;       // Button Pin

// variables will change:
bool lightOn = false;
bool fanOn = false;
bool humOn = false;
bool displOn = false;
volatile bool buttonPressed = false;
int output_value_hum ;
unsigned long previousLightMillis = 0; // Previous time for light
unsigned long previousFanMillis = 0;   // Previous time for fan
unsigned long previousHumMillis = 0;   // Previous time for humidity sensor
unsigned long lightStartTime = 0;
unsigned long fanStartTime = 0;
unsigned long humStartTime = 0;

// Constant variables for humidity sensor
int water_value = 1000;
int dirt_value = 100;

// Constant variables for ventilator
const word PWM_FREQ_HZ = 25000; //Adjust this value to adjust the frequency (Frequency in HZ!) (Set currently to 25kHZ)
const word TCNT1_TOP = 16000000/(2*PWM_FREQ_HZ);

// Constant variables for time loop
const unsigned long lightInterval = 30000; // 1 hour in milliseconds
const unsigned long fanInterval = 10000;   // 30 minutes in milliseconds
const unsigned long humInterval = 20000;   // 30 minutes in milliseconds
const unsigned long lightDuration = 5000;  // 5 minutes in milliseconds
const unsigned long fanDuration = 5000;  // 5 minutes in milliseconds
const unsigned long humDuration = 15000;  // 5 minutes in milliseconds


// Function for display
unsigned long casovaZnacka;

void obrazovka1(int hum, long allSeconds)
{

int runHours= allSeconds/3600;
int secsRemaining=allSeconds%3600;
int runMinutes=secsRemaining/60;
int runSeconds=secsRemaining%60;

  //lcd.begin(16,2);
  //lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Vlhkost:");  
  lcd.setCursor(8, 0);
  lcd.print("    ");
  lcd.setCursor(8, 0);
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
  lcd.print("  "); 
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
  // Start serial communication
  Serial.begin(9600);

  // Set pin modes
  pinMode(OC1A_PIN, OUTPUT);
  pinMode(relayPin_mot, OUTPUT);
  pinMode(relayPin_light, OUTPUT);
  pinMode(relayPin_fan, OUTPUT);
  pinMode(relayPin_hum, OUTPUT);
  pinMode(relayPin_displ, OUTPUT);  
  pinMode(sensor_pin_hum, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // initialize the pushbutton pin as an input
  pinMode(buttonPin_displ, INPUT_PULLUP); // initialize the pushbutton pin as an input

  // Initialize the relay as LOW
  digitalWrite(relayPin_mot, HIGH);
  digitalWrite(relayPin_light, HIGH);
  digitalWrite(relayPin_fan, HIGH);
  digitalWrite(relayPin_hum, HIGH);
  digitalWrite(relayPin_displ, HIGH);

  // Initialize LCD
  lcd.init();           // initialize the lcd
  lcd.backlight();      // zapni podsvícení
  lcd.setCursor(3, 0);  // dej kurzor na sloupec 1, řádek 0
  lcd.print("IRRIGATION"); // napis na displej "IRRIGATION"
  delay(2000);
  lcd.begin(16,2);
  lcd.clear();

  // Ventilator
  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // Set Timer1 configuration
  // COM1A(1:0) = 0b10   (Output A clear rising/set falling)
  // COM1B(1:0) = 0b00   (Output B normal operation)
  // WGM(13:10) = 0b1010 (Phase correct PWM)
  // ICNC1      = 0b0    (Input capture noise canceler disabled)
  // ICES1      = 0b0    (Input capture edge select disabled)
  // CS(12:10)  = 0b001  (Input clock select = clock/1)

  //TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  //TCCR1B = _BV(WGM13) | _BV(CS10);
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << CS10);
  ICR1 = TCNT1_TOP;

  OCR1A = 0;
  OCR1B = 0;



}



void loop() {

  //lcd.noDisplay();

  unsigned long currentMillis = millis();

  // reading button values
  int buttonState = digitalRead(buttonPin);   // 0 ON
  int buttonState_displ = digitalRead(buttonPin_displ);

  Serial.print("Cycle");

  // Check if lcd should be ON or OFF
  if (buttonState_displ == 0 && !displOn){
    digitalWrite(relayPin_displ, HIGH);
    lcd.begin(16,2);
    lcd.clear();
    obrazovka1(output_value_hum, currentMillis/1000);
    displOn = true;
  }
  else if (buttonState_displ == 0 && displOn) {
    obrazovka1(output_value_hum, currentMillis/1000);
  }
  else {
    digitalWrite(relayPin_displ, LOW);
    displOn = false;
  }


  
  
  // Check if it's time to turn on the light
  if (currentMillis - previousLightMillis >= lightInterval) {
    previousLightMillis = currentMillis;
    lightStartTime = currentMillis; // Record the time when the light is turned on
    digitalWrite(relayPin_light, LOW);
    lightOn = true;
    if (buttonState_displ == 0){
      lcd.setCursor(15, 1);
      lcd.print("L");
    }
  }

  // Check if the light should turn off 
  if (lightOn && (currentMillis - lightStartTime >= lightDuration)) {
    digitalWrite(relayPin_light, HIGH);
    lightOn = false;
    if (buttonState_displ == 0){
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
  }

  
  // Check if it's time to turn on the fan
  if (currentMillis - previousFanMillis >= fanInterval) {
    previousFanMillis = currentMillis;
    fanStartTime = currentMillis; // Record the time when the light is turned on
    digitalWrite(relayPin_fan, LOW);
    setPwmDuty(50);
    fanOn = true;
    if (buttonState_displ == 0){
      lcd.setCursor(15, 0);
      lcd.print("F");
    }
  }

  // Check if the fan should turn off 
  if (fanOn && (currentMillis - fanStartTime >= fanDuration)) {
    digitalWrite(relayPin_fan, HIGH);
    fanOn = false;
    setPwmDuty(0);
    if (buttonState_displ == 0){
      lcd.setCursor(15, 0);
      lcd.print(" ");
    }
  }

  // Check if it's time to turn on the humidity sensor
  if (currentMillis - previousHumMillis >= humInterval) {
    previousHumMillis = currentMillis;
    humStartTime = currentMillis; // Record the time when the humidity sensor is turned on
    digitalWrite(relayPin_hum, LOW);
    humOn = true;
    if (buttonState_displ == 0){
      lcd.setCursor(14, 1);
      lcd.print("H");
    }
  }

  // Check if the humidity sensor should turn off 
  if (humOn && (currentMillis - humStartTime >= humDuration)) {
    digitalWrite(relayPin_hum, HIGH);
    humOn = false;
    output_value_hum = analogRead(sensor_pin_hum);
    Serial.print("Humidity is: ");
    Serial.print(output_value_hum);
    Serial.print("\n");
    if (buttonState_displ == 0){
      lcd.setCursor(14, 1);
      lcd.print(" ");
    }
  }
  

  // Update display if button is pressed
  if (buttonState == 0) {
    buttonPressed = false; // Reset the flag
    digitalWrite(relayPin_mot, LOW);
    if (buttonState_displ == 0){
      lcd.setCursor(14, 0);
      lcd.print("W");
    }
  }
  else {
    digitalWrite(relayPin_mot, HIGH);
    if (buttonState_displ == 0){
      lcd.setCursor(14, 0);
      lcd.print(" ");
    }
  }
  
  
  delay(1000); // Delay to reduce CPU usage
}


// Function which converts PWM values for ventilator
void setPwmDuty(byte duty) {
  OCR1A = (word) (duty*TCNT1_TOP)/100;
}

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Time Struct
struct Waktu {
  int jam;
  int menit;
  int detik;
};

// Button and Buzzer Pin, Initialize LCD 
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int buttonPin2 = 2;
const int buttonPin3 = 3;
const int buttonPin4 = 4;
const int buzzerPin = 7;

// button Read 3 & 4 Variable
bool buttonRead3 = false;
bool buttonRead4 = false;

// Button Variable
int previousButtonState = LOW;

// External Interrupt Button 2 Variable
volatile int seconds = 0; 
volatile bool flag = false; 
volatile bool buttonPressed = false; 
volatile double buttonPressStartTime = 0;
volatile double buttonPressDuration;

// Mode Variable
volatile int mode = 0;

// Timer Variable
volatile bool setTimer = 0;
volatile int timerDown = 0;
int timerCount;

// Alarm Variable 
volatile bool curTimeAlarm = 1;
volatile bool setAlarm = 0;
volatile int alarmUp = 0;
int alarmCount;

// Stopwatch Variable
volatile bool setStopwatch = 0;
volatile int stopwatchUp = 0;

// Digital Watch Time
Waktu waktuAwal;
Waktu waktuBaru;

// Timer Time
Waktu waktuAwalTimer = {0, 0, 0};
Waktu targetTimer;
Waktu counterTimer;

// Alarm Time
Waktu waktuAwalAlarm;
Waktu targetAlarm;
Waktu counterAlarm;

// Stopwatch Time
Waktu waktuAwalStopwatch = {0, 0, 0};
Waktu waktuBaruStopWatch;

// Timer for Arduino
ISR(TIMER1_COMPA_vect) {
  seconds++;
  flag = true;
}

// External Button interrupt Function for Button 2
void buttonInterrupt() {
  if (digitalRead(buttonPin2) == LOW) {
    buttonPressStartTime = millis(); 
    buttonPressed = true; 
  }
  else { 
    if (buttonPressed) {
      buttonPressDuration = (millis() - buttonPressStartTime) / 1000;
      watchMode();

      Serial.print("Current Mode : ");
      Serial.println(mode);
      Serial.print("Button pressed for: ");
      Serial.print(buttonPressDuration); 
      Serial.println(" seconds");

      buttonPressed = false; 
    }
  }
}

// Button Press counter for Timer Edit with Button 3 and 4
void timerEdit(int buttonPin) {
  int buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && previousButtonState == LOW) {
    if(buttonPin == buttonPin3){
      timerCount = timerCount + 60;
    }
    else if(buttonPin == buttonPin4 && timerCount >= 60){
      timerCount = timerCount - 60;
    }
  }
  previousButtonState = buttonState;
}

// Button Press counter for alarm Edit with Button 3 and 4
void alarmEdit(int buttonPin) {
  int buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && previousButtonState == LOW) {
    if(buttonPin == buttonPin3){
      alarmCount = alarmCount + 60;
    }
    else if(buttonPin == buttonPin4 && alarmCount >= 60){
      alarmCount = alarmCount - 60;
    }
  }
  previousButtonState = buttonState;
}

// Add Time Function
Waktu addTime(Waktu waktuAwal, int detikTambahan) {
  int jamTambahan = detikTambahan / 3600;
  int sisaDetik = detikTambahan % 3600;
  int menitTambahan = sisaDetik / 60;
  int detikBaru = sisaDetik % 60;

  Waktu waktuBaru = {waktuAwal.jam + jamTambahan, waktuAwal.menit + menitTambahan, waktuAwal.detik + detikBaru};

  waktuBaru.menit += waktuBaru.detik / 60;
  waktuBaru.detik %= 60;

  waktuBaru.jam += waktuBaru.menit / 60;
  waktuBaru.menit %= 60;

  waktuBaru.jam %= 24;

  return waktuBaru;
}

// Substract Time Function
Waktu subsTime(Waktu waktuAwal, int detikKurang) {
  int jamKurang = detikKurang / 3600;
  int sisaDetik = detikKurang % 3600;
  int menitKurang = sisaDetik / 60;
  int detikBaru = sisaDetik % 60;

  Waktu waktuBaru = {waktuAwal.jam + jamKurang, waktuAwal.menit + menitKurang, waktuAwal.detik + detikBaru};

  if (waktuBaru.detik < 0) {
    waktuBaru.detik += 60;
    waktuBaru.menit--;
  }
  if (waktuBaru.menit < 0) {
    waktuBaru.menit += 60;
    waktuBaru.jam--;
  }
  if (waktuBaru.jam < 0) {
    waktuBaru.jam += 24;
  }

  return waktuBaru;
}

// LCD Display Fucntion
void displayLCD(Waktu time){
  if(time.jam > 9){
    lcd.print(time.jam);
  }
  else{
    lcd.print("0");
    lcd.print(time.jam);
  }
  
  lcd.print(":");

  if(time.menit > 9){
    lcd.print(time.menit);
  }
  else{
    lcd.print("0");
    lcd.print(time.menit);
  }

  lcd.print(":");

  if(time.detik > 9){
    lcd.print(time.detik);
  }
  else{
    lcd.print("0");
    lcd.print(time.detik);
  }
}

// Change Watch Mode with Button 2
void watchMode() {
  if ((buttonPressDuration < 3) && (buttonPressDuration > 0.5) && (mode < 3) && (!setTimer) && (!setAlarm) && (!setStopwatch)){
    mode++;
  }
  else if ((buttonPressDuration < 3) && (buttonPressDuration > 0.5) && (mode == 3) && (!setTimer) && (!setAlarm) && (!setStopwatch)){
    mode = 0;
  }
}

// Set Timer with Button 2
void timerMode() {
  if ((buttonPressDuration >= 3) && (setTimer == false) && (mode == 1)){
    setTimer = true;
  }
  else if ((buttonPressDuration >= 3) && (setTimer == true) && (mode == 1)){
    setTimer = false;
  }
  buttonPressDuration = 0;
}

// Set Alarm with Button 2
void alarmMode() {
  if ((buttonPressDuration >= 3) && (setAlarm == false) && (mode == 2)){
    setAlarm = true;
  }
  else if ((buttonPressDuration >= 3) && (setAlarm == true) && (mode == 2)){
    setAlarm = false;
  }
  buttonPressDuration = 0;
}

// Set Stopwatch with Button 2
void stopwatchMode() {
  if ((buttonPressDuration >= 3) && (setStopwatch == false) && (mode == 3)){
    setStopwatch = true;
  }
  else if ((buttonPressDuration >= 3) && (setStopwatch == true) && (mode == 3)){
    setStopwatch = false;
  }
  buttonPressDuration = 0;
}

// Change Watch Mode Function
void changeMode(){
  switch(mode){
    case 0: // Watch Mode
      lcd.setCursor(0, 0);
      lcd.print("Digital Watch");
      lcd.setCursor(0, 1);

      waktuAwal = {23, 58, 50};
      waktuBaru = addTime(waktuAwal, seconds);

      displayLCD(waktuBaru);
      break;
    case 1: // Timer Mode
      lcd.setCursor(0, 0);
      lcd.print("Timer");
      lcd.setCursor(0, 1);

      if(setTimer){
          timerDown--;
          counterTimer = subsTime(targetTimer, timerDown);
          displayLCD(counterTimer);
          if(counterTimer.jam == waktuAwalTimer.jam && counterTimer.menit == waktuAwalTimer.menit && counterTimer.detik == waktuAwalTimer.detik){
            setTimer = false;
            while(digitalRead(buttonPin2)){
              digitalWrite(buzzerPin, HIGH);
              digitalWrite(buzzerPin, LOW);
            }
            timerDown = 0;
          }
      }
      else{
        timerDown = 0;

        timerEdit(buttonPin3);
        timerEdit(buttonPin4);

        targetTimer = addTime(waktuAwalTimer, timerCount);

        displayLCD(targetTimer);
      }

      timerMode();
      break;
    case 2: // Alarm Mode
      lcd.setCursor(0, 0);
      lcd.print("Alarm");
      lcd.setCursor(0, 1);
      
      if(curTimeAlarm){
        waktuAwalAlarm = addTime(waktuAwal, seconds);
      }
      
      if(setAlarm){
        alarmUp++;
        counterAlarm = addTime(waktuAwal, seconds);
        displayLCD(counterAlarm);

        if(counterAlarm.jam == targetAlarm.jam && counterAlarm.menit== targetAlarm.menit && counterAlarm.detik == targetAlarm.detik){
          setAlarm = false;
          while(digitalRead(buttonPin2)){
            digitalWrite(buzzerPin, HIGH);
            digitalWrite(buzzerPin, LOW);
          }
          alarmUp = 0;
          curTimeAlarm = 1;
        }
      }
      else{
        curTimeAlarm = 0;

        alarmEdit(buttonPin3);
        alarmEdit(buttonPin4);

        targetAlarm = addTime(waktuAwalAlarm, alarmCount);

        displayLCD(targetAlarm);
      }

      alarmMode();
      break;
    case 3: // Stopwatch Mode
      curTimeAlarm = 1;

      lcd.setCursor(0, 0);
      lcd.print("Stop Watch");
      lcd.setCursor(0, 1);

      if(setStopwatch){
        stopwatchUp++;
        waktuBaruStopWatch = addTime(waktuAwalStopwatch, stopwatchUp);
        displayLCD(waktuBaruStopWatch);
      }
      else{
        stopwatchUp = 0;
        displayLCD(waktuAwalStopwatch);
      }

      stopwatchMode();
      break;
    default:
      lcd.setCursor(0, 0);
      lcd.print("Digital Watch");

      lcd.setCursor(0, 1);
      waktuAwal = {20, 21, 50};
      waktuBaru = addTime(waktuAwal, seconds);

      displayLCD(waktuBaru);
      break;
  }
}

// Setup
void setup() {
  // Setup Serial Monitor
  Serial.begin(9600);
  
  // Setup LCD
  lcd.begin();
  lcd.backlight();

  // Atur pin tombol sebagai input & interrupt eksternal untuk tombol
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(buttonPin4, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(buttonPin2), buttonInterrupt, CHANGE); 

  // Set timer1 untuk interrupt setiap 1 detik, Mode CTC, dan prescaler 1024
  TCCR1A = 0; 
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = 15624;
  TIMSK1 |= (1 << OCIE1A); 
  sei(); 
}

// Main loop
void loop() {
  if (flag) {
    lcd.clear();
    changeMode();
    flag = false; // Reset flag
  }
}
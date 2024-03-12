#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

struct Waktu {
  int jam;
  int menit;
  int detik;
};

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int buttonPin2 = 2;
const int buttonPin3 = 3;
const int buttonPin4 = 4;
const int buzzerPin = 7;

bool buttonRead3 = false;
bool buttonRead4 = false;

volatile int seconds = 0; 
volatile bool flag = false; 
volatile bool buttonPressed = false; 
volatile unsigned long buttonPressStartTime = 0;
volatile unsigned long buttonPressDuration;
volatile int mode = 0;

volatile bool setTimer = 0;
volatile int timerDown = 0;

int buttonPresses;
int previousButtonState = LOW;

Waktu waktuAwal;
Waktu waktuAwalTimer = {0, 0, 0};
Waktu waktuBaru;
Waktu waktuBaruTimer;
Waktu waktuBaruTimer2;

void buttonInterrupt() {
  if (digitalRead(buttonPin2) == HIGH) {
    buttonPressStartTime = millis(); 
    buttonPressed = true; 
  }
  else { 
    if (buttonPressed) {
      buttonPressDuration = millis() - buttonPressStartTime;
      buttonPressDuration = buttonPressDuration / 1000;
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

void countButtonPresses(int buttonPin) {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && previousButtonState == LOW) {
    if(buttonPin == buttonPin3){
      buttonPresses = buttonPresses + 60;
    }
    else if(buttonPin == buttonPin4){
      buttonPresses = buttonPresses - 60;
    }
  }

  previousButtonState = buttonState;
}

Waktu tambahDetik(Waktu waktuAwal, int detikTambahan) {
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

Waktu kurangiDetik(Waktu waktuAwal, int detikKurang) {
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

ISR(TIMER1_COMPA_vect) {
  seconds++;
  flag = true;
}

void watchMode() {
  if ((buttonPressDuration == 1) && (mode < 2)){
    mode++;
  }
  else if ((buttonPressDuration == 1) && (mode == 2)){
    mode = 0;
  }
}

void timerMode() {
  if ((buttonPressDuration >= 2) && (setTimer == false) && (mode == 1)){
    setTimer = true;
  }
  else if ((buttonPressDuration >= 2) && (setTimer == true) && (mode == 1)){
    setTimer = false;
  }
  buttonPressDuration = 0;
}

void changeMode(){
  switch(mode){
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Digital Watch");

      lcd.setCursor(0, 1);
      waktuAwal = {23, 58, 50};
      waktuBaru = tambahDetik(waktuAwal, seconds);

      lcd.print(waktuBaru.jam);
      lcd.print(":");
      lcd.print(waktuBaru.menit);
      lcd.print(":");
      lcd.print(waktuBaru.detik);
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Timer");
      lcd.setCursor(0, 1);

      if(setTimer){
          timerDown--;
          waktuBaruTimer2 = kurangiDetik(waktuBaruTimer, timerDown);
          if(waktuBaruTimer2.jam == waktuAwalTimer.jam && waktuBaruTimer2.menit== waktuAwalTimer.menit && waktuBaruTimer2.detik == waktuAwalTimer.detik){
            setTimer = false;
            while(!digitalRead(buttonPin2)){
              digitalWrite(7, HIGH);
              digitalWrite(7, LOW);
            }
            timerDown = 0;
          }

          lcd.print(waktuBaruTimer2.jam);
          lcd.print(":");
          lcd.print(waktuBaruTimer2.menit);
          lcd.print(":");
          lcd.print(waktuBaruTimer2.detik);
      }
      else{
        countButtonPresses(buttonPin3);
        countButtonPresses(buttonPin4);
        waktuBaruTimer = tambahDetik(waktuAwalTimer, buttonPresses);
        lcd.print(waktuBaruTimer.jam);
        lcd.print(":");
        lcd.print(waktuBaruTimer.menit);
        lcd.print(":");
        lcd.print(waktuBaruTimer.detik);
      }
      break;
    default:
      lcd.setCursor(0, 0);
      lcd.print("Digital Watch");

      lcd.setCursor(0, 1);
      waktuAwal = {20, 21, 50};
      waktuBaru = tambahDetik(waktuAwal, seconds);

      lcd.print(waktuBaru.jam);
      lcd.print(":");
      lcd.print(waktuBaru.menit);
      lcd.print(":");
      lcd.print(waktuBaru.detik);
      break;
  }
}

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

void loop() {
  if (flag) {
    lcd.clear();
    changeMode();
    timerMode();
    flag = false; // Reset flag
  }
}

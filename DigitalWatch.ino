#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Inisialiasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Variabel untuk menyimpan jumlah detik
volatile int seconds = 0; 
// Flag untuk menandakan waktu telah berubah
volatile bool flag = false; 
// Flag untuk menandakan tombol ditekan
volatile bool buttonPressed = false; 
// Waktu mulai tombol ditekan
volatile unsigned long buttonPressStartTime = 0;
// Durasi tombol ditekan
volatile unsigned long buttonPressDuration;
// Pin Button
const int buttonPin = 2;
// Variabel untuk menyimpan mode button
volatile int mode = 0;

struct Waktu {
  int jam;
  int menit;
  int detik;
};

// Fungsi untuk menambahkan detik ke waktu
Waktu tambahDetik(Waktu waktuAwal, int detikTambahan) {
  // Mengubah tambahan detik menjadi jam, menit, dan detik baru
  int jamTambahan = detikTambahan / 3600;
  int sisaDetik = detikTambahan % 3600;
  int menitTambahan = sisaDetik / 60;
  int detikBaru = sisaDetik % 60;

  // Menambahkan jam, menit, dan detik tambahan ke waktu awal
  Waktu waktuBaru = {waktuAwal.jam + jamTambahan, waktuAwal.menit + menitTambahan, waktuAwal.detik + detikBaru};

  // Handle jika detik baru melebihi 60
  waktuBaru.menit += waktuBaru.detik / 60;
  waktuBaru.detik %= 60;

  // Handle jika menit baru melebihi 60
  waktuBaru.jam += waktuBaru.menit / 60;
  waktuBaru.menit %= 60;

  // Handle jika jam baru melebihi 24 jam
  waktuBaru.jam %= 24;

  return waktuBaru;
}

// Fungsi untuk menghandle interrupt timer
ISR(TIMER1_COMPA_vect) {
  seconds++;
  flag = true;
}

// Fungsi untuk mengubah Mode dari Jam
void watchMode() {
  if ((buttonPressDuration > 3) && (mode < 3)){
    mode++;
  }
  else if ((buttonPressDuration > 3) && (mode == 3)){
    mode = 0;
  }
}

void changeMode(){
  switch(mode){
    case 0:
      Waktu waktuAwal = {20, 21, 50};
      Waktu waktuBaru = tambahDetik(waktuAwal, seconds);

      lcd.clear();
      lcd.print(waktuBaru.jam);
      lcd.print(":");
      lcd.print(waktuBaru.menit);
      lcd.print(":");
      lcd.print(waktuBaru.detik);
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    default:
      break;
  }
}

// Fungsi untuk menghandle interrupt tombol
int buttonInterrupt() {
  if (digitalRead(buttonPin) == HIGH) {
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

void setup() {
  // Setup Serial Monitor
  Serial.begin(9600);
  
  // Setup LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Atur pin tombol sebagai input & interrupt eksternal untuk tombol
  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, CHANGE); 

  // Set timer1 untuk interrupt setiap 1 detik, Mode CTC, dan prescaler 1024
  TCCR1A = 0; 
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = 15624;
  TIMSK1 |= (1 << OCIE1A); 
  sei(); 
}

void loop() {
    if (flag) {
        Waktu waktuAwal = {20, 21, 50};
        Waktu waktuBaru = tambahDetik(waktuAwal, seconds);

        lcd.clear();
        lcd.print(waktuBaru.jam);
        lcd.print(":");
        lcd.print(waktuBaru.menit);
        lcd.print(":");
        lcd.print(waktuBaru.detik);
        
        flag = false; // Reset flag
    }
}

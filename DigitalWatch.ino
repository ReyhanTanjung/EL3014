#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
volatile int seconds = 0; // Variabel untuk menyimpan jumlah detik
volatile bool flag = false; // Flag untuk menandakan waktu telah berubah

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

// Fungsi untuk menghandle interrupt
ISR(TIMER1_COMPA_vect) {
  seconds++; // Increment jumlah detik setiap kali interrupt terjadi
  flag = true; // Set flag menjadi true untuk menandakan waktu telah berubah
}

void setup() {
  Serial.begin(9600);
  
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  

  // Set timer1 untuk interrupt setiap 1 detik
  TCCR1A = 0; // Atur mode normal
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // Mode CTC, prescaler 1024
  OCR1A = 15624; // Hitung ulang untuk mendapatkan interrupt setiap 1 detik
  TIMSK1 |= (1 << OCIE1A); // Aktifkan interrupt OCR1A
  
  sei(); // Aktifkan global interrupt
}

void loop() {
  if (flag) {
    Waktu waktuAwal = {12, 30, 45};
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

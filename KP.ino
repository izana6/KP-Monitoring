#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(D2, DHT22);

const char* ssid = "estikuntari";
const char* password = "kuntariesti123";

const unsigned long myChannelNumber = 2690365;
const char* myWriteAPIKey = "11UCVD0FZLY8JZ3C";
WiFiClient client;

const int sensorCO = A0;          // MiCS-5524 sensor pin
const int sensorHC = A0;          // MiCS-5524 sensor pin
const int sensorMQ135 = A0;       // MQ135 sensor pin
const int relayPin = D5;
const int relayPinRed = D5;       // Pin lampu merah
const int relayPinYellow = D6;    // Pin lampu kuning
const int relayPinGreen = D7;     // Pin lampu hijau

// Ambang batas
const float CO_THRESHOLD = 50.0;  // Ambang batas CO (ppm)
const float HC_THRESHOLD = 500.0; // Ambang batas HC (ppm)
const float CO2_THRESHOLD = 1000.0; // Ambang batas CO2 (ppm)

// Rentang untuk lampu kuning
const float WARNING_FACTOR = 0.8; // 80% dari ambang batas

const float RL = 10.0; // Load resistance dalam kilo ohm
float Ro_CO = 9.8; // Nilai Ro di udara bersih untuk MiCS5524 (kalibrasi CO)
float Ro_HC = 10.0; // Nilai Ro di udara bersih untuk MiCS5524 (kalibrasi HC)
float Ro_CO2 = 10.0; // Nilai Ro di udara bersih untuk MQ-135 (kalibrasi CO2)

// Fungsi untuk menghitung nilai Rs dari sensor
float getSensorResistance(int pin) {
    int sensorValue = analogRead(pin);
    float sensorVoltage = (sensorValue / 1023.0) * 5.0;  // Tegangan sensor (0-5V)
    float Rs_value = (5.0 - sensorVoltage) / sensorVoltage * RL; // Menghitung Rs
    return Rs_value;
}

// Fungsi untuk menghitung konsentrasi CO (ppm) dari rasio Rs/Ro
float calculateCO(float ratio) {
    float ppm = pow(10, ((log10(ratio) - 0.45) / (-0.8))); 
    return ppm;
}
    
// Fungsi untuk menghitung konsentrasi HC (ppm) dari rasio Rs/Ro
float calculateHC(float ratio) {
    float ppm = pow(10, ((log10(ratio) - 0.4) / (-0.5)));
    return ppm;
}
  
// Fungsi untuk menghitung konsentrasi CO2 (ppm) dari rasio Rs/Ro
float calculateCO2(float ratio) {
    float ppm = 116.6020682 * pow(ratio, -2.769034857);
    return ppm;
}

void setup() {
    Serial.begin(115200);

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            lcd.clear();
            lcd.print("Connecting...");
            Serial.println("Connecting to WiFi...");
        }
    }

    lcd.init();
    lcd.backlight();

    // Inisialisasi RTC DS3231
    if (!rtc.begin()) {
      Serial.println("RTC DS3231 tidak terdeteksi!");
      while (1);
    }
    

    pinMode(relayPin, OUTPUT);
    pinMode(relayPinRed, OUTPUT);
    pinMode(relayPinYellow, OUTPUT);
    pinMode(relayPinGreen, OUTPUT);
    digitalWrite(relayPin, LOW);
    dht.begin();

    ThingSpeak.begin(client);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setup Complete");
    delay(2000);
}

void loop() {
    DateTime now = rtc.now();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Baca Rs dari sensor CO (MiCS5524) dan HC
    float Rs_CO = getSensorResistance(sensorCO);
    float Rs_HC = getSensorResistance(sensorHC);
  
    // Baca Rs dari sensor CO2 (MQ-135)
    float Rs_CO2 = getSensorResistance(sensorMQ135);

    // Kalibrasi menggunakan Rs/Ro
    float ratio_CO = Rs_CO / Ro_CO;   // Rasio Rs/Ro untuk CO
    float ratio_HC = Rs_HC / Ro_HC;   // Rasio Rs/Ro untuk HC
    float ratio_CO2 = Rs_CO2 / Ro_CO2; // Rasio Rs/Ro untuk CO2

    // Estimasi konsentrasi gas (ppm)
    float co_ppm = calculateCO(ratio_CO);
    float hc_ppm = calculateHC(ratio_HC);  // Menghitung konsentrasi HC
    float co2_ppm = calculateCO2(ratio_CO2);

    // Kontrol lampu lalu lintas berdasarkan kadar gas
    controlTrafficLight(co_ppm, hc_ppm, co2_ppm);

    // Display data on Serial Monitor
    Serial.print("CO: "); 
    Serial.print(co_ppm); 
    Serial.print(" ppm | ");
    Serial.print("HC: "); 
    Serial.print(hc_ppm); 
    Serial.print(" ppm | ");
    Serial.print("CO2: "); 
    Serial.print(co2_ppm); 
    Serial.print(" ppm | ");
    Serial.print("Temp: "); 
    Serial.print(temperature); 
    Serial.print(" C | ");
    Serial.print("Humidity: "); 
    Serial.print(humidity); 
    Serial.println(" %");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CO: "); 
    lcd.print(co_ppm); 
    lcd.print("ppm");
    lcd.setCursor(0, 1);
    lcd.print("HC: "); 
    lcd.print(hc_ppm); 
    lcd.print("ppm");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CO2: "); 
    lcd.print(co2_ppm); 
    lcd.print("ppm");
    lcd.setCursor(0, 1);
    lcd.print("Temp: "); 
    lcd.print(temperature); 
    lcd.print("C");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hum: ");
    lcd.print(humidity);
    lcd.println("%");
    delay(2000);

    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, co_ppm);
    ThingSpeak.setField(4, hc_ppm);
    ThingSpeak.setField(5, co2_ppm);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
        lcd.clear();
        lcd.print("Data sent!");
        Serial.println("Data sent to ThingSpeak.");
    } else {
        lcd.clear();
        lcd.print("Send failed");
        Serial.println("Failed to send data. Error code: " + String(x));
    }
    delay(5000);

    // Check relay control time
    if (now.hour() == 6 && now.minute() == 0) {
        digitalWrite(relayPin, HIGH);
        delay(1000);
    } else if (now.hour() == 18 && now.minute() == 0) {
        digitalWrite(relayPin, LOW);
    }
    delay(10000);
}

// Fungsi untuk mengontrol lampu lalu lintas
void controlTrafficLight(float co, float hc, float co2) {
    bool redLight = false;
    bool yellowLight = false;
    bool greenLight = false;

    // Periksa kondisi untuk lampu merah
    if (co > CO_THRESHOLD || hc > HC_THRESHOLD || co2 > CO2_THRESHOLD) {
        redLight = true;  // Nyalakan lampu merah
    } 
    // Periksa kondisi untuk lampu kuning
    else if ((co >= WARNING_FACTOR * CO_THRESHOLD) || (hc >= WARNING_FACTOR * HC_THRESHOLD) || (co2 >= WARNING_FACTOR * CO2_THRESHOLD)) {
        yellowLight = true; // Nyalakan lampu kuning
    } 
    // Jika aman, nyalakan lampu hijau
    else {
        greenLight = true; // Nyalakan lampu hijau
    }

    // Aktifkan relay berdasarkan kondisi
    digitalWrite(relayPinRed, redLight ? HIGH : LOW);
    digitalWrite(relayPinYellow, yellowLight ? HIGH : LOW);
    digitalWrite(relayPinGreen, greenLight ? HIGH : LOW);

    // Tampilkan status lampu di LCD
    if (redLight) {
        lcd.setCursor(0, 0);
        lcd.print("Lampu Merah");
    } else if (yellowLight) {
        lcd.setCursor(0, 0);
        lcd.print("Lampu Kuning");
    } else {
        lcd.setCursor(0, 0);
        lcd.print("Lampu Hijau");
    }
}
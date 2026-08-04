/*
  IoT-Based Heart Rate and Temperature Monitoring System
  ---------------------------------------------------------
  Reads heart rate (analog pulse sensor) and body temperature (DS18B20),
  displays live status on an OLED, logs data to ThingSpeak, and sends
  an emergency SMS via Twilio when readings fall outside a safe range.

  Hardware:
    - ESP8266 NodeMCU
    - Analog pulse/heartbeat sensor on A0
    - DS18B20 temperature sensor (OneWire)
    - SSD1306 OLED display (I2C)
    - Buzzer + status LED

  NOTE: Replace all placeholder values below (WiFi, ThingSpeak, Twilio)
  with your own credentials in a private config — never commit real
  credentials to a public repository.
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>

Adafruit_SSD1306 display(128, 64, &Wire);
WiFiClient client;

// ---------------- Pin Definitions ----------------
const int sensorPin = A0;   // Pulse sensor analog input
#define STATUS_LED   8       // D8 - status/heartbeat LED
#define BUZZER_PIN   12      // D6 - buzzer
#define ONE_WIRE_BUS 4        // D2 - DS18B20 data pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// ---------------- Heart Rate Variables ----------------
int sensorValue;
int lastSensorValue = 0;
int count = 0;
unsigned long starttime = 0;
int heartrate = 0;
boolean peakDetected = false;

// ---------------- Temperature Variable ----------------
float temperatureC = 0.0;

// ---------------- WiFi Credentials (placeholder) ----------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------------- ThingSpeak Credentials (placeholder) ----------------
unsigned long myChannelNumber = 0000000;              // Your channel ID
const char myWriteAPIKey[]    = "YOUR_THINGSPEAK_WRITE_API_KEY";

// ---------------- Twilio Credentials (placeholder) ----------------
const char* TWILIO_SID          = "YOUR_TWILIO_SID";
const char* TWILIO_AUTH_TOKEN   = "YOUR_TWILIO_AUTH_TOKEN";
const char* TWILIO_NUMBER       = "YOUR_TWILIO_PHONE_NUMBER";
const char* RECEIVER_NUMBER     = "YOUR_RECEIVER_PHONE_NUMBER";

void sendSMSAlert(int bpm); // forward declaration

void setup(void) {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(115200);
  display.clearDisplay();

  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(STATUS_LED, HIGH);
    delay(300);
    digitalWrite(STATUS_LED, LOW);
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);
  tempSensor.begin();
}

void loop() {
  starttime = millis();
  count = 0;
  lastSensorValue = 0;
  peakDetected = false;

  // Reading pulse sensor for 20 seconds using a DYNAMIC (self-adjusting) threshold
  while (millis() < starttime + 20000) {
    sensorValue = analogRead(sensorPin);

    // Detect heartbeat using a dynamic threshold based on the previous sample
    if ((sensorValue > lastSensorValue + 50) && !peakDetected) {
      count++;
      digitalWrite(STATUS_LED, HIGH);
      delay(10);
      digitalWrite(STATUS_LED, LOW);
      peakDetected = true;
    } else if (sensorValue < lastSensorValue - 50) {
      peakDetected = false; // reset once signal drops back down
    }

    lastSensorValue = sensorValue;
    delay(50);
  }

  Serial.print("Pulse count: ");
  Serial.println(count);

  heartrate = count * 3; // 20-second window -> multiply by 3 for BPM
  Serial.print("BPM = ");
  Serial.println(heartrate);

  // ---------------- Read Temperature ----------------
  tempSensor.requestTemperatures();
  temperatureC = tempSensor.getTempCByIndex(0);
  Serial.print("Temperature = ");
  Serial.print(temperatureC);
  Serial.println(" C");

  // ---------------- Display live BPM on OLED ----------------
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("Heart Rate");
  display.setCursor(0, 28);
  display.print("BPM: ");
  display.print(heartrate);
  display.setCursor(0, 48);
  display.print("Temp: ");
  display.print(temperatureC);
  display.display();

  // ---------------- Upload heart-rate and temperature data to ThingSpeak cloud ----------------
  ThingSpeak.setField(1, heartrate);
  ThingSpeak.setField(2, temperatureC);
  int status = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (status == 200) {
    Serial.println("Data sent successfully to ThingSpeak!");
  } else {
    Serial.print("Error sending data. HTTP Code: ");
    Serial.println(status);
  }

  // ---------------- Send SMS if BPM is outside the safe range ----------------
  if (heartrate < 50 || heartrate > 120) {
    sendSMSAlert(heartrate);
  }

  delay(20000); // Wait before next reading (always runs, alert or not)
}

// ---------------- Twilio SMS Alert ----------------
void sendSMSAlert(int bpm) {
  Serial.println("ALERT: Abnormal Heart Rate Detected!");

  WiFiClientSecure secureClient;
  secureClient.setInsecure(); // For testing only; use certificate validation in production

  HTTPClient https;
  String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(TWILIO_SID) + "/Messages.json";

  String postData = "To=" + String(RECEIVER_NUMBER) +
                     "&From=" + String(TWILIO_NUMBER) +
                     "&Body=Emergency! Abnormal Heart Rate Detected: " + String(bpm) + " BPM";

  https.begin(secureClient, url);
  https.setAuthorization(TWILIO_SID, TWILIO_AUTH_TOKEN);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = https.POST(postData);

  if (httpResponseCode > 0) {
    Serial.print("SMS Sent! Response: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error Sending SMS: ");
    Serial.println(httpResponseCode);
  }

  https.end();
}

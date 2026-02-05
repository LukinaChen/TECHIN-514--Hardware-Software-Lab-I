#include <Arduino.h>
#include <WiFi.h>

// --- Configuration ---
#define WIFI_SSID     "Your_SSID"
#define WIFI_PASSWORD "Your_Password"
#define TRIG_PIN      5
#define ECHO_PIN      18
#define THRESHOLD_CM  50.0
#define SLEEP_TIME_S  30 

// --- Function to read Ultrasonic Sensor ---
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

// --- Mock function for Firebase upload ---
void uploadData(float dist) {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Success] Uploading distance to Firebase...");
    // Add your Firebase logic here
    delay(500); // Simulate network latency
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Wake up...");
  
  // 1. Measure Distance
  float distance = getDistance();
  Serial.printf("Measured Distance: %.2f cm\n", distance);

  // 2. Decision Logic
  if (distance < THRESHOLD_CM) {
    Serial.println("Object detected! Triggering WiFi Upload.");
    uploadData(distance);
  } else {
    Serial.println("No movement. Skipping WiFi to save power.");
  }

  // 3. Prepare for Deep Sleep
  Serial.println("Entering Deep Sleep for 30 seconds...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
  // This section is never reached in deep sleep mode
}
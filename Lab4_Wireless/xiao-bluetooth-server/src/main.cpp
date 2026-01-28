#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// =======================
// HC-SR04 pins (ESP32-C3)
// D3 -> GPIO5, D4 -> GPIO6
// =======================
#define TRIG_PIN 5
#define ECHO_PIN 6

// =======================
// BLE UUIDs
// =======================
#define SERVICE_UUID        "ec5d3da5-5fc1-4f03-9164-c586a51a4f02"
#define CHARACTERISTIC_UUID "e09301b1-b541-408b-b93a-92ae95f5d767"

// =======================
// BLE objects
// =======================
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// =======================
// DSP: Moving Average
// =======================
const int WINDOW_SIZE = 5;
float distanceBuffer[WINDOW_SIZE];
int bufferIndex = 0;
bool bufferFilled = false;

// =======================
// BLE callbacks
// =======================
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {
    deviceConnected = true;
    Serial.println("Client connected");
  }

  void onDisconnect(BLEServer*) {
    deviceConnected = false;
    Serial.println("Client disconnected");
  }
};

// =======================
// Read raw distance
// =======================
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance = duration * 0.034 / 2.0;
  return distance;
}

// =======================
// Moving average filter
// =======================
float movingAverage(float newValue) {
  distanceBuffer[bufferIndex] = newValue;
  bufferIndex++;

  if (bufferIndex >= WINDOW_SIZE) {
    bufferIndex = 0;
    bufferFilled = true;
  }

  float sum = 0;
  int count = bufferFilled ? WINDOW_SIZE : bufferIndex;

  for (int i = 0; i < count; i++) {
    sum += distanceBuffer[i];
  }

  return sum / count;
}

// =======================
// Setup
// =======================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE Server + HC-SR04");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  BLEDevice::init("Lukina_BLE_Server");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("Init");

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("Advertising started");
}

// =======================
// Loop
// =======================
void loop() {
  float rawDistance = readDistanceCM();
  float filteredDistance = movingAverage(rawDistance);

  Serial.print("Raw: ");
  Serial.print(rawDistance);
  Serial.print(" cm | Filtered: ");
  Serial.print(filteredDistance);
  Serial.println(" cm");

  if (deviceConnected && filteredDistance < 30.0) {
    char msg[32];
    snprintf(msg, sizeof(msg), "%.2f", filteredDistance);
    pCharacteristic->setValue(msg);
    pCharacteristic->notify();
    Serial.println("BLE sent (filtered < 30 cm)");
  }

  delay(500);
}

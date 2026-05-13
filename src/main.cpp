#include <Arduino.h>
#include <NimBLEDevice.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-ab12-ab12-ab12-abcdef123456"

NimBLECharacteristic* pCharacteristic;

const int TOUCH_PINS[] = {33, 13, 12, 15, 2};
const int NUM_INSTRUMENTS = 5;
const int MODE_PIN = 32;
const int TOUCH_THRESHOLD = 50;

const char* INSTRUMENT_NAMES[] = {"flute", "tambourine", "lyre", "clarinet", "horn"};

// Mode 0 = single, Mode 1 = loop
int currentMode = 0;
bool wasActive[NUM_INSTRUMENTS] = {false};
bool modeWasActive = false;
unsigned long modePressStart = 0;
bool modeLongPress = false;
unsigned long lastModeToggle = 0;
const int MODE_DEBOUNCE_MS = 400;
const int LONG_PRESS_MS = 800;

void setup() {
  Serial.begin(115200);
  delay(500);

  NimBLEDevice::init("InstrumentPanel");
  NimBLEServer* pServer = NimBLEDevice::createServer();
  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::NOTIFY
  );

  pService->start();
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE advertising as 'InstrumentPanel'");
}

void send(const char* msg) {
  Serial.println(msg);
  pCharacteristic->setValue((uint8_t*)msg, strlen(msg));
  pCharacteristic->notify();
  delay(20);
}

void loop() {
  unsigned long now = millis();

  int modeVal = touchRead(MODE_PIN);
  bool modeTouched = modeVal < TOUCH_THRESHOLD;

  if (modeTouched && !modeWasActive) {
      modePressStart = now;
      modeLongPress = false;
  }
  if (modeTouched && modeWasActive && !modeLongPress) {
      if (now - modePressStart >= LONG_PRESS_MS) {
          modeLongPress = true;
          currentMode = 2;
          send("MODE:RECORD");
      }
  }
  if (!modeTouched && modeWasActive && !modeLongPress) {
      if (now - modePressStart < LONG_PRESS_MS && now - lastModeToggle > MODE_DEBOUNCE_MS) {
          currentMode = (currentMode == 2) ? 0 : (currentMode + 1) % 2;
          lastModeToggle = now;
          send(("MODE:" + String(currentMode)).c_str());
      }
  }
  modeWasActive = modeTouched;

  for (int i = 0; i < NUM_INSTRUMENTS; i++) {
    int val = touchRead(TOUCH_PINS[i]);
    bool touched = val < TOUCH_THRESHOLD;

    if (touched && !wasActive[i]) {
      send((String(currentMode) + "," + INSTRUMENT_NAMES[i]).c_str());
    }

    wasActive[i] = touched;
  }

  delay(50);
}



// Test code for touch pin sensors
// #include <Arduino.h>

// void setup() {
//   Serial.begin(115200);
//   delay(1000);
//   Serial.println("READY");
// }

// void loop() {
//   Serial.print("33:"); Serial.print(touchRead(33));
//   Serial.print("  13:"); Serial.print(touchRead(13));
//   Serial.print("  12:"); Serial.print(touchRead(12));
//   Serial.print("  15:"); Serial.print(touchRead(15));
//   Serial.print("  2:"); Serial.print(touchRead(2));
//   Serial.print("  32:"); Serial.println(touchRead(32));
//   delay(200);
// }
#include <Arduino.h>
#include <NimBLEDevice.h>

// ===============================
// M5Atom Lite IR Blaster
// ===============================

#define IR_PIN 25
#define DEVICE_NAME "M5-IR-Blaster"

// BLE UUIDs
#define SERVICE_UUID  "7b7e0001-1234-4567-89ab-123456789000"
#define COMMAND_UUID  "7b7e0002-1234-4567-89ab-123456789000"
#define STATUS_UUID   "7b7e0003-1234-4567-89ab-123456789000"

NimBLECharacteristic *statusCharacteristic;

// ---------------------------------
// IR carrier
// ---------------------------------

void sendMark(uint32_t duration) {
  uint32_t start = micros();

  while ((micros() - start) < duration) {
    digitalWrite(IR_PIN, HIGH);
    delayMicroseconds(13);
    digitalWrite(IR_PIN, LOW);
    delayMicroseconds(13);
  }

  digitalWrite(IR_PIN, LOW);
}

void sendSpace(uint32_t duration) {
  digitalWrite(IR_PIN, LOW);
  delayMicroseconds(duration);
}

// ---------------------------------
// NEC protocol
// ---------------------------------

void sendNECByte(uint8_t value) {
  for (int bit = 0; bit < 8; bit++) {
    sendMark(560);

    if (value & 0x01) {
      sendSpace(1690);  // NEC "1"
    } else {
      sendSpace(560);   // NEC "0"
    }

    value >>= 1;
  }
}

void sendNEC(uint8_t address, uint8_t command) {
  Serial.printf(
    "NEC frame: %02X %02X %02X %02X\n",
    address,
    (uint8_t)~address,
    command,
    (uint8_t)~command
  );

  // NEC header
  sendMark(9000);
  sendSpace(4500);

  // Address
  sendNECByte(address);

  // Inverted address
  sendNECByte(~address);

  // Command
  sendNECByte(command);

  // Inverted command
  sendNECByte(~command);

  // Final mark
  sendMark(560);

  digitalWrite(IR_PIN, LOW);
}

// ---------------------------------
// IR test
// ---------------------------------

void testIR() {
  Serial.println("IR TEST");

  for (int i = 0; i < 5; i++) {
    sendMark(10000);
    digitalWrite(IR_PIN, LOW);
    delay(100);
  }

  digitalWrite(IR_PIN, LOW);

  Serial.println("IR TEST COMPLETE");
}

// ---------------------------------
// Send status back over BLE
// ---------------------------------

void sendStatus(const String &message) {
  if (statusCharacteristic) {
    statusCharacteristic->setValue(message.c_str());
    statusCharacteristic->notify();
  }

  Serial.println(message);
}

// ---------------------------------
// BLE command callback
// ---------------------------------

class CommandCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(
    NimBLECharacteristic *characteristic,
    NimBLEConnInfo &connInfo
  ) override {

    String command = characteristic->getValue().c_str();

    command.trim();
    command.toUpperCase();

    Serial.print("BLE Command: ");
    Serial.println(command);

    // -----------------------------
    // IR test
    // -----------------------------
    if (command == "TEST") {
      testIR();
      sendStatus("TEST OK");
      return;
    }

    // -----------------------------
    // NEC command
    //
    // Format:
    // NEC:04:02
    // -----------------------------

    if (command.startsWith("NEC:")) {

      int firstColon = command.indexOf(':');
      int secondColon = command.indexOf(':', firstColon + 1);

      if (secondColon > 0) {

        String addressString =
          command.substring(firstColon + 1, secondColon);

        String commandString =
          command.substring(secondColon + 1);

        uint8_t address =
          (uint8_t)strtoul(addressString.c_str(), nullptr, 16);

        uint8_t irCommand =
          (uint8_t)strtoul(commandString.c_str(), nullptr, 16);

        Serial.printf(
          "Sending NEC address=0x%02X command=0x%02X\n",
          address,
          irCommand
        );

        sendNEC(address, irCommand);

        String status =
          "SENT NEC:" +
          String(address, HEX) +
          ":" +
          String(irCommand, HEX);

        status.toUpperCase();

        sendStatus(status);
        return;
      }
    }

    sendStatus("UNKNOWN COMMAND");
  }
};

// ---------------------------------
// BLE server callbacks
// ---------------------------------

class ServerCallbacks : public NimBLEServerCallbacks {

  void onConnect(
    NimBLEServer *server,
    NimBLEConnInfo &connInfo
  ) override {

    Serial.println("BLE client connected");

    sendStatus("CONNECTED");
  }

  void onDisconnect(
    NimBLEServer *server,
    NimBLEConnInfo &connInfo,
    int reason
  ) override {

    Serial.printf(
      "BLE client disconnected, reason=%d\n",
      reason
    );

    // Start advertising again
    NimBLEDevice::getAdvertising()->start();

    Serial.println("BLE advertising restarted");
  }
};

// ---------------------------------
// Setup
// ---------------------------------

void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" M5Atom Lite Haier IR Blaster");
  Serial.println("==============================");

  // IR GPIO
  pinMode(IR_PIN, OUTPUT);
  digitalWrite(IR_PIN, LOW);

  // BLE
  NimBLEDevice::init(DEVICE_NAME);

  NimBLEServer *server =
    NimBLEDevice::createServer();

  server->setCallbacks(
    new ServerCallbacks(),
    false
  );

  NimBLEService *service =
    server->createService(SERVICE_UUID);

  NimBLECharacteristic *commandCharacteristic =
    service->createCharacteristic(
      COMMAND_UUID,
      NIMBLE_PROPERTY::WRITE |
      NIMBLE_PROPERTY::WRITE_NR
    );

  commandCharacteristic->setCallbacks(
    new CommandCallbacks()
  );

  statusCharacteristic =
    service->createCharacteristic(
      STATUS_UUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::NOTIFY
    );

  statusCharacteristic->setValue("READY");

  service->start();

  NimBLEAdvertising *advertising =
    NimBLEDevice::getAdvertising();

  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setName(DEVICE_NAME);
  advertising->start();

  Serial.println("BLE advertising started");
  Serial.print("Device: ");
  Serial.println(DEVICE_NAME);
  Serial.print("IR GPIO: ");
  Serial.println(IR_PIN);
  Serial.println("Ready.");
}

// ---------------------------------
// Main loop
// ---------------------------------

void loop() {
  delay(10);
}

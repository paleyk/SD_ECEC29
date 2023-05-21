#include <ArduinoBLE.h>
#include <Arduino_LSM6DSOX.h>

const int numDataPoints = 300;  // Number of data points to collect
const float threshold = 3.0;    // Accelerometer threshold
const int chunkSize = 20;       // Size of each BLE chunk

struct IMUData {
  unsigned long timestamp;
  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;
};

IMUData data[numDataPoints];

// Create a BLE service
BLEService imuService("180D");

// Create a BLE characteristic
BLECharacteristic imuChar("2A37", BLERead | BLENotify, chunkSize);  // BLE chunkSize data

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  BLE.begin();

  // set advertised local name and service UUID:
  BLE.setLocalName("IMUData");
  BLE.setAdvertisedService(imuService);

  // add the characteristic to the service
  imuService.addCharacteristic(imuChar);

  // add service
  BLE.addService(imuService);
}

void loop() {
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    float xAcc, yAcc, zAcc;
    IMU.readAcceleration(xAcc, yAcc, zAcc);

    float sumAcc = abs(xAcc) + abs(yAcc) + abs(zAcc);
    
    if (sumAcc >= threshold) {
      recordData();
      sendBLEData();
    }
  }
}

void recordData() {
  for (int i = 0; i < numDataPoints; i++) {
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      float xAcc, yAcc, zAcc, xGyro, yGyro, zGyro;
      IMU.readAcceleration(xAcc, yAcc, zAcc);
      IMU.readGyroscope(xGyro, yGyro, zGyro);

      data[i].timestamp = millis();
      data[i].accX = xAcc;
      data[i].accY = yAcc;
      data[i].accZ = zAcc;
      data[i].gyroX = xGyro;
      data[i].gyroY = yGyro;
      data[i].gyroZ = zGyro;

      delay(10); // Slow down the data collection rate
    }
  }
  
  printData();
}

void printData() {
  for (int i = 0; i < numDataPoints; i++) {
    Serial.print("Timestamp: ");
    Serial.print(data[i].timestamp);
    Serial.print(", AccX: ");
    Serial.print(data[i].accX);
    Serial.print(", AccY: ");
    Serial.print(data[i].accY);
    Serial.print(", AccZ: ");
    Serial.print(data[i].accZ);
    Serial.print(", GyroX: ");
    Serial.print(data[i].gyroX);
    Serial.print(", GyroY: ");
    Serial.print(data[i].gyroY);
    Serial.print(", GyroZ: ");
    Serial.println(data[i].gyroZ);
  }
}

void sendBLEData() {
  byte* bytePtr = (byte*)data;  // Get a byte pointer to the data array
  
  for (int i = 0; i < sizeof(IMUData) * numDataPoints; i += chunkSize) {
    int currentChunkSize = min(chunkSize, sizeof(IMUData) * numDataPoints - i);
    imuChar.setValue(bytePtr + i, currentChunkSize);
    delay(20);  // Allow BLE to send data
  }
}
#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>

#define CE_PIN 7
#define CSN_PIN 8

// Create RF24 and MPU6050 objects
RF24 radio(CE_PIN, CSN_PIN);
Adafruit_MPU6050 mpu;

// Create servo objects for the servos
Servo servo1;
Servo servo2;

// Structure to hold all the sensor data
struct SensorData {
  float angleX;
  float angleY;
  float gyroX;
  float gyroY;
  float gyroZ;
  float accX;
  float accY;
  float accZ;
  float temp;
  float thrust;
  float altitude;
  float pressure;
} sensorData;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize the I2C communication
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Set accelerometer and gyroscope ranges
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize RF24
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setPayloadSize(sizeof(sensorData));
  radio.openWritingPipe(0xF0F0F0F0E1LL);  // Set the address for writing
  radio.stopListening();  // Set the radio to TX mode

  // Attach the servos to their respective pins
  servo1.attach(6);
  servo2.attach(5);

  Serial.println("Setup complete");
}

void loop() {
  // Variables to hold the sensor event data
  sensors_event_t a, g, temp;

  // Get new sensor events
  mpu.getEvent(&a, &g, &temp);

  // Convert the accelerometer values to angles
  sensorData.angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  sensorData.angleY = atan2(a.acceleration.x, a.acceleration.z) * 180 / PI;

  // Map the angles to servo positions (0 to 180 degrees)
  int servoPosX = map(sensorData.angleX, -90, 90, 0, 180);
  int servoPosY = map(sensorData.angleY, -90, 90, 0, 180);

  // Write the positions to the servos
  servo1.write(servoPosX);
  servo2.write(servoPosY);

  // Populate the rest of the sensor data
  sensorData.gyroX = g.gyro.x;
  sensorData.gyroY = g.gyro.y;
  sensorData.gyroZ = g.gyro.z;
  sensorData.accX = a.acceleration.x;
  sensorData.accY = a.acceleration.y;
  sensorData.accZ = a.acceleration.z;
  sensorData.temp = temp.temperature;
  sensorData.thrust = 10;  // Dummy value for thrust, replace with actual calculation
  sensorData.altitude = 100;  // Dummy value for altitude, replace with actual calculation
  sensorData.pressure = 1013;  // Dummy value for pressure, replace with actual sensor reading

  // Transmit sensor data using RF24
  bool report = radio.write(&sensorData, sizeof(sensorData));
  if (report) {
    Serial.println("Transmission successful!");
  } else {
    Serial.println("Transmission failed or timed out");
  }

  // Delay to allow the servos to move
  delay(15);
}

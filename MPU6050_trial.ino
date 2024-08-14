#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>

// Create servo objects for the servos
Servo servo1;
Servo servo2;

// Create an Adafruit_MPU6050 object
Adafruit_MPU6050 mpu;

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

  // Set accelerometer range
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Set gyroscope range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Set filter bandwidth
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Attach the servos to their respective pins
  servo1.attach(7);
  servo2.attach(4);

  Serial.println("Setup complete");
}

void loop() {
  // Variables to hold the sensor event data
  sensors_event_t a, g, temp;

  // Get new sensor events
  mpu.getEvent(&a, &g, &temp);

  // Convert the accelerometer values to angles
  float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  float angleY = atan2(a.acceleration.x, a.acceleration.z) * 180 / PI;

  // Print the angles to the serial monitor for debugging
  Serial.print("Angle X: ");
  Serial.print(angleX);
  Serial.print(" Angle Y: ");
  Serial.println(angleY);

  // Map the angle to a servo position (0 to 180 degrees)
  int servoPosX = map(angleX, -90, 90, 0, 180);
  int servoPosY = map(angleY, -90, 90, 0, 180);

  // Write the positions to the servos
  servo1.write(servoPosX);
  servo2.write(servoPosY);

  // Delay to allow the servos to move
  delay(15);
}
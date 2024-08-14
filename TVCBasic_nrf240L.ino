#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <RF24.h>

// Create an RF24 object
RF24 radio(7, 8); // CE, CSN pins

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

  // Initialize RF24
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(0xF0F0F0F0E1LL); // Set the address for writing
  radio.setDataRate(RF24_1MBPS);

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

  // Prepare the data to send
  float data[2] = {angleX, angleY};

  // Send the data
  radio.write(&data, sizeof(data));

  // Print the angles to the serial monitor for debugging
  Serial.print("Angle X: ");
  Serial.print(angleX);
  Serial.print(" Angle Y: ");
  Serial.println(angleY);

  // Delay before the next loop
  delay(100);
}

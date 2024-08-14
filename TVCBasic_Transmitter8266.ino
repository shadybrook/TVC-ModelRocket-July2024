#include <SPI.h>
#include <RF24.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);  // CE, CSN pins
Adafruit_MPU6050 mpu;

float angles[2] = {0.0, 0.0}; // Array to hold angle X and Y

void setup() {
  Serial.begin(115200);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setPayloadSize(sizeof(angles));
  radio.openWritingPipe(0xF0F0F0F0E1LL);  // Set the address for writing
  radio.stopListening();  // Set the radio to TX mode
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  angles[0] = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  angles[1] = atan2(a.acceleration.x, a.acceleration.z) * 180 / PI;

  bool report = radio.write(&angles, sizeof(angles));

  if (report) {
    Serial.println("Transmission successful!");
  } else {
    Serial.println("Transmission failed or timed out");
  }

  delay(1000);  // Adjust the delay as needed
}

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <RF24.h>

// Create servo objects
Servo servo1;
Servo servo2;

// Create an Adafruit_MPU6050 object
Adafruit_MPU6050 mpu;

// SD card module CS pin
const int chipSelect = 10;

// nRF24L01 module CE and CSN pins
RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";

// PID controller variables
float Kp = 2.0;
float Ki = 0.5;
float Kd = 1.0;

float setpointX = 0.0; // Desired angle for X-axis
float setpointY = 0.0; // Desired angle for Y-axis

float previousErrorX = 0.0;
float integralX = 0.0;

float previousErrorY = 0.0;
float integralY = 0.0;

struct FlightData {
  unsigned long time;
  float angleX;
  float angleY;
  float controlX;
  float controlY;
  int servoPosX;
  int servoPosY;
};

File dataFile;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize the MPU6050
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

  // Attach the servos to pins
  servo1.attach(7);
  servo2.attach(6);

  // Initial servo positions
  servo1.write(90);
  servo2.write(90);

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Card initialized.");

  // Open the file for writing
  dataFile = SD.open("flightdata.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time,AngleX,AngleY,ControlX,ControlY,ServoPosX,ServoPosY");
    dataFile.close();
  } else {
    Serial.println("Error opening flightdata.txt");
  }

  // Initialize nRF24L01
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening();

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

  // PID control for X-axis
  float errorX = setpointX - angleX;
  integralX += errorX;
  float derivativeX = errorX - previousErrorX;
  float controlX = Kp * errorX + Ki * integralX + Kd * derivativeX;
  previousErrorX = errorX;

  // PID control for Y-axis
  float errorY = setpointY - angleY;
  integralY += errorY;
  float derivativeY = errorY - previousErrorY;
  float controlY = Kp * errorY + Ki * integralY + Kd * derivativeY;
  previousErrorY = errorY;

  // Map the control output to a servo position (0 to 180 degrees)
  int servoPosX = map(controlX, -90, 90, 0, 180);
  int servoPosY = map(controlY, -90, 90, 0, 180);

  // Constrain the values to be within the range 0 to 180
  servoPosX = constrain(servoPosX, 0, 180);
  servoPosY = constrain(servoPosY, 0, 180);

  // Write the positions to the servos
  servo1.write(servoPosX);
  servo2.write(servoPosY);

  // Log data to SD card
  dataFile = SD.open("flightdata.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(angleX);
    dataFile.print(",");
    dataFile.print(angleY);
    dataFile.print(",");
    dataFile.print(controlX);
    dataFile.print(",");
    dataFile.print(controlY);
    dataFile.print(",");
    dataFile.print(servoPosX);
    dataFile.print(",");
    dataFile.println(servoPosY);
    dataFile.close();
  } else {
    Serial.println("Error opening flightdata.txt");
  }

  // Prepare and send the flight data via nRF24L01
  FlightData flightData = { millis(), angleX, angleY, controlX, controlY, servoPosX, servoPosY };
  radio.write(&flightData, sizeof(FlightData));

  // Delay to allow the servos to move
  delay(15);
}

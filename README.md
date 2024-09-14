Here's a detailed README file for your GitHub project:

---

# TVC Model Rocket Project - July 2024

Welcome to the TVC (Thrust Vector Control) Model Rocket Project! This repository contains the design, development, and implementation of a thrust vector-controlled model rocket, including all necessary code, hardware schematics, and data analysis tools.

![IMG_5201](https://github.com/user-attachments/assets/f79279ba-92d7-421b-9616-d9e13f6ccbce)
![IMG_5140](https://github.com/user-attachments/assets/513478c2-106b-469b-97e0-c7516c5422ec)
## Overview

This project aims to develop a model rocket with thrust vector control capabilities, allowing precise maneuverability and stability during flight. The rocket is equipped with sensors, actuators, and a control system designed to maintain its intended flight path and perform controlled maneuvers.

## Features

- **Thrust Vector Control**: Utilize servo motors to adjust the nozzle for flight stabilization and trajectory control.
- **Data Logging**: Collect flight data using sensors and log it to an SD card for post-flight analysis.
- **Live Telemetry**: Transmit real-time data from the rocket to a ground station for monitoring.
- **Arduino-Based Control**: Use an Arduino Uno for processing sensor inputs and controlling the servos.
- **Modular Design**: Easy-to-modify code and hardware components for customization and testing.

## Hardware Components

- **Arduino Uno**: Main microcontroller used for processing inputs and controlling outputs.
- **MPU6050**: IMU sensor for capturing acceleration and gyroscopic data.
- **Servo Motors**: Used for adjusting the rocket’s thrust vector.
- **nRF24L01**: Wireless module for live data transmission.
- **SD Card Module**: For logging flight data locally on the rocket.
- **Battery**: 11.1V 2000mAh 3C 3S1P Li-Ion Battery Pack for powering the control circuit.

## Software Components

- **Arduino Code**: Contains the main control algorithms and sensor integration.
- **Python Scripts**: For analyzing flight data and visualizing thrust curves, roll, and pitch control.
- **NodeMCU ESP8266**: Receives telemetry data and displays it on a connected device.

## Getting Started

### Prerequisites

- Arduino IDE
- Python 3.x
- Required Python libraries: `matplotlib`, `numpy`, `pandas`
- Basic understanding of Arduino programming and circuit design

## Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/shadybrook/TVC-ModelRocket-July2024.git
   cd TVC-ModelRocket-July2024
   ```

2. **Set up the Arduino environment**:
   - Open the Arduino IDE and install necessary libraries:
     - MPU6050 by Electronic Cats
     - Servo library (built-in)
     - RF24 by TMRh20

3. **Upload the Arduino code**:
   - Connect your Arduino Uno to your computer.
   - Open the `TVC_Rocket.ino` file in the Arduino IDE.
   - Select the correct board and port.
   - Click on **Upload**.

4. **Install Python dependencies**:
   ```bash
   pip install matplotlib numpy pandas
   ```

## Usage

1. **Prepare the Rocket**:
   - Assemble the hardware as per the schematics provided in the `/hardware` folder.
   - Ensure all connections are secure and the battery is fully charged.

2. **Launch the Rocket**:
   - Place the rocket on the launch pad.
   - Power on the system and initiate the launch sequence.

3. **Monitor Telemetry**:
   - Use the NodeMCU ESP8266 setup to receive and visualize telemetry data on your laptop.

4. **Post-Flight Analysis**:
   - Retrieve the data from the SD card.
   - Use the provided Python scripts in the `/data_analysis` folder to analyze and visualize the flight performance.

## Data Analysis

- **Thrust Curve**: Analyze the thrust curve data to optimize motor performance.
- **Roll and Pitch Control**: Visualize roll and pitch control effectiveness during flight.
- **Center of Gravity and Pressure**: Evaluate the rocket’s stability based on CG and CP calculations.

## Contributing

Contributions are welcome! If you have ideas for improvement or want to add new features, feel free to open an issue or submit a pull request.

1. Fork the repository.
2. Create a new branch (`git checkout -b feature-branch`).
3. Make your changes.
4. Commit your changes (`git commit -m 'Add some feature'`).
5. Push to the branch (`git push origin feature-branch`).
6. Open a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Special thanks to the open-source community for providing valuable tools and libraries.
- Inspiration for this project was drawn from various model rocketry enthusiasts and educational resources.

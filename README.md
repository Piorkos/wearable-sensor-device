<p align="center">
  <img src="/img/readme_header_3.jpg" />
</p>

# Wearable device for data collection from sensors - firmware
This repository contains the firmware for a custom-built device that collects and stores data from various sensors, including a gyroscope, accelerometer, magnetometer, and GPS. The device stores data in Flash memory and can transfer it to a computer via UART for further analysis. The device features a user interface with a display and two buttons for simple interaction.


#### Modular Hardware Design
The firmware is designed to be hardware-agnostic. Each hardware component's functionality is encapsulated within its own dedicated class, making it easy to replace or upgrade hardware components. To adapt the firmware to new hardware, simply implement or modify the driver class for the specific part.

---

### Features:
- **Data Collection:** The device collects sensor data from:
  - Gyroscope
  - Accelerometer
  - Magnetometer
  - GPS
- **Data Storage:** Sensor data is stored in an onboard 16MB Flash memory.
- **Data Transfer:** Data can be sent to a computer via UART.
- **User Interface:** A Sharp memory in pixel display provides real-time information, and two buttons allow users to:
  - Start/stop data collection
  - Send data to a computer
  - Erase stored data




### Hardware
Used hardware:
- microcontroller: RP2040, dual core 32-bit Arm® Cortex®-M0+ @ 133MHz
- accelerometer and gyroscope: LSM6DSOXTR
- GPS: Pimoroni PA1010D
- e-compass: Pimoroni LSM303D
- memory: AT25SF128A-MHB-T, 16MB Flash IC
- display: OLED based on SSD1306 driver

The firmware is designed to ensure that each hardware component is easily swappable. By isolating hardware-specific functionality into dedicated classes (e.g., for sensors, memory, and display), replacing a hardware part requires minimal changes—just modify or create a driver for the new part. This modular structure allows the device to support a variety of different components, enabling easy customization and expansion.


#### Planned improvements
In first version I used an OLED display (see image below), however later I replaced it by Sharp memory in pixel display. I plan to improve the firmware to support both kinds of displays.
<p align="center">
  <img src="/img/readme_header_2.png" />
</p>

---
Feel free to modify the code and adapt it to your specific use case or hardware setup. Contributions are welcome!



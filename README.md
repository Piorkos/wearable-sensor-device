<p align="center">
  <img src="/img/readme_header.png" />
</p>

# Wearable device for data collection from sensors
This device is a versatile tool designed for collecting and storing data from a gyroscope, accelerometer, e-compass, and GPS.

<ins>**Note:** The objective was to obtain a working device as soon as possible, hence the code design is far from perfection.</ins>

---

### Features:
1. Capture data from gyroscope, accelerometer, magnetometer, GPS and then store it in the Flash memory. 
2. Read data from the memory and send it via serial port to the computer.
3. Erase data from the memory.

UI is composed from display and 2 buttons.

<p align="center">
  <img width="600" src="/img/readme_erase_data.jpg" />
</p>


### Hardware
The firmware was designed for following hardware:
- microcontroller: RP2040, dual core 32-bit Arm® Cortex®-M0+ @ 133MHz
- accelerometer and gyroscope: LSM6DSOXTR
- GPS: Pimoroni PA1010D
- e-compass: Pimoroni LSM303D
- memory: AT25SF128A-MHB-T, 16MB Flash IC
- display: OLED based on SSD1306 driver




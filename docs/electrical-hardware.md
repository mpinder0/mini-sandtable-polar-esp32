# Electrical Hardware Information

![architechture diagram](./Electrical%20Architecture.png)

## Electrical BOM

Component Description | Quantity | Example Link
---- | ---- | ----
ESP32 dev board | 1 | [Link](https://github.com/TronixLab/DOIT_ESP32_DevKit-v1_30P/tree/main)
A4988 stepper driver | 2 | [Link](https://www.pololu.com/product/1182)
NEMA17 stepper motor | 2 | [Link](https://www.omc-stepperonline.com/e-series-nema-17-bipolar-1-8deg-17ncm-24-07oz-in-1a-42x42x23mm-4-wires-17he08-1004s)
A3144 UNIPOLAR Hall Effect | 1 | [Link](https://components101.com/sensors/a3144-hall-effect-sensor)
12V power supply - 2A | 1 | 

## ESP32 Wiring
[ESP32 DevKit Pinout](https://raw.githubusercontent.com/TronixLab/DOIT_ESP32_DevKit-v1_30P/refs/heads/main/Docs/DOIT_ESP32-DevkitV1_PINOUT.png)

- 3.3V -> Stepper Drivers, VDD 
- GND -> Stepper Drivers, GND
- GPIO15 - OUT - Stepper Drivers, Enable
- GPIO02 - OUT - Stepper Driver 1 _Theta_ Step
- GPIO04 - OUT - Stepper Driver 1 _Theta_ Direction
- GPIO16 - OUT - Stepper Driver 2 _Rho_ Step
- GPIO17 - OUT - Stepper Driver 2 _Rho_ Direction
- GPIO05 - IN - Hall Effect Sensor (Reference)

## A4988 Stepper Driver Helpful Information
[Control Stepper Motor with A4988 Driver Module & Arduino](https://lastminuteengineers.com/a4988-stepper-motor-driver-arduino-tutorial/)

 - For Half-stepping, hold MS1 pin `High`. MS2 and 3 `Low`.
    - Note: MS pins are `Low` by default.
 - RST pin should be held `High` as we're not using that feature.

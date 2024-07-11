# Arduino Scale and Menu System

This project is an Arduino-based scale and menu system that uses various components such as a rotary encoder, load cell, RTC, and TFT display. The system allows users to navigate through menus, select parts, and measure weights with a load cell.

## Components Used

- **BPI-Leaf-S3**
- **Rotary Encoder**
- **Load Cell with HX711 Amplifier**
- **RTC Module (DS3231)**
- **TFT Display ILI9486 Parallel 8-bit (TFT_eSPI)**
- **Buzzer (Tone library)**
- **SPIFFS for File System**
- **ArduinoJson for JSON Handling**

## Features

- **Menu Navigation**: Navigate through different menus using a rotary encoder.
- **Weight Measurement**: Measure weights using a load cell and display the readings on a TFT screen.
- **RTC Integration**: Display current date and time using an RTC module.
- **Part List Management**: Manage a list of parts with their standard weights stored in SPIFFS.
- **Scale Calibration**: Calibrate the scale for accurate weight measurements.

## Installation

1. **Clone the Repository**:
```sh
git clone https://github.com/mekanii/selector.git
cd arduino-scale-menu-system
```

2. **Install Required Libraries**:
    - [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
    - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
    - [RTClib](https://github.com/adafruit/RTClib)
    - [Encoder](https://www.pjrc.com/teensy/td_libs_Encoder.html)
    - [HX711](https://github.com/bogde/HX711)
    - [Tone](https://github.com/bhagman/Tone)
    - [FS](https://github.com/esp8266/Arduino/tree/master/libraries/FS)
    - [SPIFFS](https://github.com/pellepl/spiffs)

3. **Define Constants**:
    - Open `src/main.cpp` in your preferred code editor.
    - Locate the section at the top of the file where constants are defined. It should look like this:
    ```cpp
    #define FORMAT_SPIFFS_IF_FAILED true
    #define CREATE_NEW_PARTLIST true

    #define RW_SCALE_FACTOR false
    #define INIT_CALIBRATION true
    #define TEST_SCALE_CALIBRATION false

    #define HYSTERESIS 5.0f
    #define HYSTERESIS_STABLE_CHECK 1.0f
    #define STABLE_READING_REQUIRED 30
    ```
    - Modify the constants as needed for your specific setup:
        - `FORMAT_SPIFFS_IF_FAILED`: Set to `true` to format SPIFFS if it fails to mount.
        - `CREATE_NEW_PARTLIST`: Set to `true` to create a new part list.
        - `RW_SCALE_FACTOR`: Set to `true` if you want to read/write the scale factor.
        - `INIT_CALIBRATION`: Set to `true` to initialize the calibration process.
        - `TEST_SCALE_CALIBRATION`: Set to `true` to test the scale calibration.
        - `HYSTERESIS`: Set the value for hysteresis in weight measurement.
        - `HYSTERESIS_STABLE_CHECK`: Set the value for stable check hysteresis.
        - `STABLE_READING_REQUIRED`: Set the number of stable readings required for a stable weight measurement.

    - Save the changes to `src/main.cpp`.

4. **Upload the Code**:
    - Open `src/main.cpp` in the Arduino IDE.
    - Select the appropriate board and port.
    - Unplug the TFT LCD.
    - Unplug the header with the longest pin on the bottom of the board.
    - Upload the code to your Arduino board.
    - Plug the header back in.
    - Plug the TFT LCD back in.

## Usage

1. **Power on the System**: Connect the Arduino to a power source.
2. **Navigate Menus**: Use the rotary encoder to navigate through the menus.
3. **Select Parts**: Select parts from the list and measure their weights.
4. **Calibrate Scale**: Follow the instructions to calibrate the scale if necessary (next).
5. **Measure Weight**: Place the weight after the OK or NG status disappears on the LCD.

## File Structure

- `src/main.cpp`: Main code file containing the implementation of the menu system and scale functionality.
- `monofontoReg20.h`, `monofontoReg28.h`, `monofontoReg96.h`: Font files for the TFT display.
- `partList.json`: JSON file storing the list of parts and their standard weights.
- `scaleFactor.txt`: File storing the scale factor for calibration.

## Acknowledgements

- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [RTClib](https://github.com/adafruit/RTClib)
- [Encoder](https://www.pjrc.com/teensy/td_libs_Encoder.html)
- [HX711](https://github.com/bogde/HX711)
- [Tone](https://github.com/bhagman/Tone)
- [FS](https://github.com/esp8266/Arduino/tree/master/libraries/FS)
- [SPIFFS](https://github.com/pellepl/spiffs)

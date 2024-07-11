# Arduino Scale and Menu System

This project is an Arduino-based scale and menu system that uses various components such as a rotary encoder, load cell, RTC, and TFT display. The system allows users to navigate through menus, select parts, and measure weights with a load cell.

## Components Used

- **Arduino Board**
- **Rotary Encoder**
- **Load Cell with HX711 Amplifier**
- **RTC Module (DS3231)**
- **TFT Display (TFT_eSPI)**
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
sh
git clone https://github.com/yourusername/arduino-scale-menu-system.git
cd arduino-scale-menu-system

2. **Install Required Libraries**:
    - [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
    - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
    - [RTClib](https://github.com/adafruit/RTClib)
    - [Encoder](https://www.pjrc.com/teensy/td_libs_Encoder.html)
    - [HX711](https://github.com/bogde/HX711)
    - [Tone](https://github.com/bhagman/Tone)
    - [FS](https://github.com/esp8266/Arduino/tree/master/libraries/FS)
    - [SPIFFS](https://github.com/pellepl/spiffs)

3. **Upload the Code**:
    - Open `src/main.cpp` in the Arduino IDE.
    - Select the appropriate board and port.
    - Upload the code to your Arduino board.

## Usage

1. **Power on the System**: Connect the Arduino to a power source.
2. **Navigate Menus**: Use the rotary encoder to navigate through the menus.
3. **Select Parts**: Select parts from the list and measure their weights.
4. **Calibrate Scale**: Follow the instructions to calibrate the scale if necessary.

## File Structure

- `src/main.cpp`: Main code file containing the implementation of the menu system and scale functionality.
- `monofontoReg20.h`, `monofontoReg28.h`, `monofontoReg96.h`: Font files for the TFT display.
- `partList.json`: JSON file storing the list of parts and their standard weights.
- `scaleFactor.txt`: File storing the scale factor for calibration.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any improvements or bug fixes.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [RTClib](https://github.com/adafruit/RTClib)
- [Encoder](https://www.pjrc.com/teensy/td_libs_Encoder.html)
- [HX711](https://github.com/bogde/HX711)
- [Tone](https://github.com/bhagman/Tone)
- [FS](https://github.com/esp8266/Arduino/tree/master/libraries/FS)
- [SPIFFS](https://github.com/pellepl/spiffs)
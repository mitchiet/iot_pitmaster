# Important Notes from Mitchell, and ESP_IDF

## ESP_IDF Getting Started

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)

## Setup ESP-IDF with VS Code

https://www.youtube.com/watch?v=Lc6ausiKvQM

## Driver needed to connect to ESP32 via micro usb

https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

## c_cpp_properties.json

Change the compiler path to the xtensa-esp32-elf-gcc.exe compiler on your machine.

## Other things that are important to download

- Python 3.5+
- git
- GCC
- CMake
- VS Code

## Necessary VS Code extensions

- ms-vscode.cpptools
- ms-vscode.cmake-tools
- espressif.esp-idf-extension
- twxs.cmake
- formulahendry.code-runner --helpful but not necessary

## Running the program

After pressing the flame icon for "Build, Flash, and Monitor" and making sure that the port is set to COM3, the terminal will say "Connecting......". At that point, hold down the boot button and only let go after stuff stops being printed to the console. Then, tap the enable button and the program will run.

## Debugging

Monitor the ESP32 in MobaXTerm on a COM port at a 115200 baud rate. Type the signal name, and a number associated with it. You can view the debug_print_loop function in "test/test.cpp" to see what signals can be set.

There are some print debugging and test debugging options in "test/debug.hpp". These are set via macro.
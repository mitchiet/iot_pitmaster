# Important Notes from Mitchell

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
- CMake --maybe unneccessary to download
- VS Code

## Necessary VS Code extensions

- ms-vscode.cpptools
- ms-vscode.cmake-tools
- espressif.esp-idf-extension
- twxs.cmake
- formulahendry.code-runner --helpful but not necessary

## Running the program

After pressing the flame icon for "Build, Flash, and Monitor" and making sure that the port is set to COM3, the terminal will say "Connecting......". At that point, hold down the boot button and only let go after stuff stops being printed to the console. Then, tap the enable button and the program will run.



# Hello World (Pre-built) Example

Starts a FreeRTOS task to print "Hello World".

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## How to use example

Follow detailed instructions provided specifically for this example. 

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)


## Example folder contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are build using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── example_test.py            Python script used for automated example testing
├── main
│   ├── CMakeLists.txt
│   ├── component.mk           Component make file
│   └── hello_world_main.c
├── Makefile                   Makefile used by legacy GNU Make
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.

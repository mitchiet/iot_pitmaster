/**
 * @file debug.hpp
 * @brief Debug Macros
 * 
 */

#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__


// PRINTING DEBUG

// Prints length everytime the esp32 writes to the Android App
#define DEBUG_WRITE_BT (0)

// Print length and value in hex everytime the ESP32 reads from Bluetooth
#define DEBUG_READ_BT (0)

// Print whenever GAP events happen.
#define DEBUG_GAP_BT (0)

// Print temperature everytime an ADC read occurs
#define DEBUG_THERMOCOUPLE (0)

// Print line for a4988 stepper driver functions
#define DEBUG_A4988 (0)

// Print line for setting the pwm duty cycle
#define DEBUG_PWM (0)




// FUNCTIONALITY DEBUG

// Always send 315 Celsius (etc.) over Bluetooth
#define DEBUG_SEND_HARDCODED_TEMP (0)


#endif /* __DEBUG_HPP__ */
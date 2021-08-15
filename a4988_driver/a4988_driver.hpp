/**
 * @file a4988_driver.hpp                  
 * @brief Stepper Motor Driver
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __A4988_DRIVER_HPP__
#define __A4988_DRIVER_HPP__

#include <iostream>

#include "driver/gpio.h"

// GPIO for stepper driver
constexpr gpio_num_t GPIO_NOT_EN = GPIO_NUM_18;
constexpr gpio_num_t GPIO_MS1 = GPIO_NUM_5;
constexpr gpio_num_t GPIO_MS2 = GPIO_NUM_17;
constexpr gpio_num_t GPIO_MS3 = GPIO_NUM_16;
constexpr gpio_num_t GPIO_NOT_RST = GPIO_NUM_4;
constexpr gpio_num_t GPIO_NOT_SLP = GPIO_NUM_0;
constexpr gpio_num_t GPIO_STEP = GPIO_NUM_2;
constexpr gpio_num_t GPIO_DIR = GPIO_NUM_15;

/// \todo get rid of static
/// \todo make GPIO members
/// \todo change name to stepper motor
/// \todo this pointers

class a4988_driver {

    private:

        // thread that runs
        static void* stepper_run(void* p);

        // ~enable signal
        static int not_en;
        // ms1 signal
        static int ms1;
        // ms2 signal
        static int ms2;
        // ms3 signal
        static int ms3;
        // ~reset signal
        static int not_rst;
        // ~sleep signal
        static int not_slp;
        // step signal, on and off pulses
        static int step;
        // direction signal
        static int dir;


    public:
        inline a4988_driver() {
        }

        // thread launcher
        static bool launch_a4988_driver_thread();

        // sets ~enable
        inline static void set_not_en(int level) {
            not_en = level;
            gpio_set_level(GPIO_NOT_EN, not_en);
            std::cout << "Set ~Enable signal to " << level << "\n";
        }

        // sets ms1
        inline static void set_ms1(int level) {
            ms1 = level;
            gpio_set_level(GPIO_MS1, ms1);
            std::cout << "Set MS1 signal to " << level << "\n";
        }

        // sets ms2
        inline static void set_ms2(int level) {
            ms2 = level;
            gpio_set_level(GPIO_MS2, ms2);
            std::cout << "Set MS2 signal to " << level << "\n";
        }

        // sets ms3
        inline static void set_ms3(int level) {
            ms3 = level;
            gpio_set_level(GPIO_MS3, ms3);
            std::cout << "Set MS3 signal to " << level << "\n";
        }

        // sets ~reset
        inline static void set_not_rst(int level) {
            not_rst = level;
            gpio_set_level(GPIO_NOT_RST, not_rst);
            std::cout << "Set ~Reset signal to " << level << "\n";
        }

        // sets ~sleep
        inline static void set_not_slp(int level) {
            not_slp = level;
            gpio_set_level(GPIO_NOT_SLP, not_slp);
            std::cout << "Set ~Sleep to " << level << "\n";
        }

        // sets step (pulse)
        inline static void set_step(const int level) {
            step = level;
            gpio_set_level(GPIO_STEP, step);
            //std::cout << "Set step to " << level << "\n"; // called too often
        }

        // sets direction (1 is clockwise, 0 is counterclockwise)
        inline static void set_dir(int level) {
            dir = level;
            gpio_set_level(GPIO_DIR, dir);
            std::cout << "Set Direction signal to " << level << "\n";
            if (level == 1)
                std::cout << "Direction set to Clockwise\n";
            else if(level == 0)
                std::cout << "Direction set to Counterclockwise\n";
        }

};

#endif /* __A4988_DRIVER_HPP__ */
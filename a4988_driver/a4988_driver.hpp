/**
 * @file a4988_driver.hpp                  
 * @brief Stepper Motor Driver
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __A4988_DRIVER_HPP__
#define __A4988_DRIVER_HPP__

#include <atomic>
#include <iostream>
#include <thread>

#include "driver/gpio.h"

inline bool is_valid_signal(const gpio_num_t gpio, const int level) {
    if (gpio == GPIO_NUM_NC) {
        std::cout << "Error: This signal is not assigned to a valid GPIO.\n";
        return false;
    }
    if (!(level == 1 || level == 0)) {
        std::cout << "Error: This signal can only be set to 1 or 0.\n";
        return false;
    }
    return true;
}

class a4988_driver {

    private:

        // ~enable signal
        gpio_num_t m_gpio_not_en;
        // ms1 signal
        gpio_num_t m_gpio_ms1;
        // ms2 signal
        gpio_num_t m_gpio_ms2;
        // ms3 signal
        gpio_num_t m_gpio_ms3;
        // ~reset signal
        gpio_num_t m_gpio_not_rst;
        // ~sleep signal
        gpio_num_t m_gpio_not_slp;
        // step signal, on and off pulses
        gpio_num_t m_gpio_step;
        // direction signal
        gpio_num_t m_gpio_dir;

        // atomic bool so it is thread safe
        std::atomic<bool> m_enabled {false};


    public:

        inline a4988_driver(const gpio_num_t not_en, const gpio_num_t ms1,
        const gpio_num_t ms2, const gpio_num_t ms3,
        const gpio_num_t not_rst, const gpio_num_t not_slp,
        const gpio_num_t step, const gpio_num_t dir) {

            this->m_gpio_not_en = not_en;
            this->m_gpio_ms1 = ms1;
            this->m_gpio_ms2 = ms2;
            this->m_gpio_ms3 = ms3;
            this->m_gpio_not_rst = not_rst;
            this->m_gpio_not_slp = not_slp;
            this->m_gpio_step = step;
            this->m_gpio_dir = dir;

        }

        // main pwm logic function
        void a4988_run();

        // thread function
        std::thread a4988_run_thread();

        // sets ~enable
        inline void set_not_en(int level) {
            if (is_valid_signal(this->m_gpio_not_en, level)) {
                gpio_set_level(this->m_gpio_not_en, level);
                std::cout << "Set ~Enable signal to " << level << "\n";
                if (level == 0)
                    m_enabled = true;
                else
                    m_enabled = false;
            }
        }

        // sets ms1
        inline void set_ms1(int level) {
            if (is_valid_signal(this->m_gpio_ms1, level)) {
                gpio_set_level(this->m_gpio_ms1, level);
                std::cout << "Set MS1 signal to " << level << "\n";
            }
        }

        // sets ms2
        inline void set_ms2(int level) {
            if (is_valid_signal(this->m_gpio_ms2, level)) {
                gpio_set_level(this->m_gpio_ms2, level);
                std::cout << "Set MS2 signal to " << level << "\n";
            }
        }

        // sets ms3
        inline void set_ms3(int level) {
            if (is_valid_signal(this->m_gpio_ms3, level)) {
                gpio_set_level(this->m_gpio_ms3, level);
                std::cout << "Set MS3 signal to " << level << "\n";
            }
        }

        // sets ~reset
        inline void set_not_rst(int level) {
            if (is_valid_signal(this->m_gpio_not_rst, level)) {
                gpio_set_level(this->m_gpio_not_rst, level);
                std::cout << "Set ~Reset signal to " << level << "\n";
            }
        }

        // sets ~sleep
        inline void set_not_slp(int level) {
            if (is_valid_signal(this->m_gpio_not_slp, level)) {
                gpio_set_level(this->m_gpio_not_slp, level);
                std::cout << "Set ~Sleep to " << level << "\n";
            }
        }

        // sets step (pulse)
        inline void set_step(const int level) {
            if (is_valid_signal(this->m_gpio_step, level)) {
                gpio_set_level(this->m_gpio_step, level);
                //std::cout << "Set step to " << level << "\n"; // called too often
            }
        }

        // sets direction (1 is clockwise, 0 is counterclockwise)
        inline void set_dir(int level) {
            if (is_valid_signal(this->m_gpio_dir, level)) {
                gpio_set_level(this->m_gpio_dir, level);
                std::cout << "Set Direction signal to " << level << "\n";
                if (level == 1)
                    std::cout << "Direction set to Clockwise\n";
                else if(level == 0)
                    std::cout << "Direction set to Counterclockwise\n";
            }
        }

};

#endif /* __A4988_DRIVER_HPP__ */
/**
 * @file a4988_driver.hpp                  
 * @brief Stepper Motor Driver
 * 
 */
#ifndef __A4988_DRIVER_HPP__
#define __A4988_DRIVER_HPP__

#include <atomic>
#include <iostream>
#include <string>

#include "driver/gpio.h"

#include "debug.hpp"

inline bool is_valid_signal(const gpio_num_t gpio, const int level) {
    if (gpio == GPIO_NUM_NC) {
        std::cout << "Error: This signal is not assigned to a valid GPIO.\n\n";
        return false;
    }
    if (!(level == 1 || level == 0)) {
        std::cout << "Error: This signal can only be set to 1 or 0.\n\n";
        return false;
    }
    return true;
}

class a4988_driver {

    private:

        // motor name, used for debuggiong purposes
        std::string m_name {};

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

        // Atomic bool so it is thread safe
        std::atomic<bool> m_enabled {false};

        // Atomic bool so it is thread safe
        std::atomic<bool> m_stop_motor {false};

    public:

        inline a4988_driver(const std::string name, const gpio_num_t not_en, const gpio_num_t ms1,
        const gpio_num_t ms2, const gpio_num_t ms3,
        const gpio_num_t not_rst, const gpio_num_t not_slp,
        const gpio_num_t step, const gpio_num_t dir) {

            this->m_name = name;
            this->m_gpio_not_en = not_en;
            this->m_gpio_ms1 = ms1;
            this->m_gpio_ms2 = ms2;
            this->m_gpio_ms3 = ms3;
            this->m_gpio_not_rst = not_rst;
            this->m_gpio_not_slp = not_slp;
            this->m_gpio_step = step;
            this->m_gpio_dir = dir;

            this->set_default_gpio_levels();
        }

        // Sets all the gpio to default states
        void set_default_gpio_levels();

        // Run the motor continuously
        void run_motor_continuous();

        // Run the motor for a certain amount of steps
        void run_motor_steps(int num_steps);

        // Stop the motor from continuously running
        inline void stop_motor() {
            this->m_stop_motor = true;
            if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Stopping continuous motor run.\n\n";
        }

        // Sets ~enable
        inline void set_not_en(int level) {
            if (is_valid_signal(this->m_gpio_not_en, level)) {
                gpio_set_level(this->m_gpio_not_en, level);

                if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Set ~Enable signal to " << level << ".\n\n";

                if (level == 0)
                    m_enabled = true;
                else
                    m_enabled = false;
            }
        }

        // Sets ms1
        inline void set_ms1(int level) {
            if (is_valid_signal(this->m_gpio_ms1, level)) {
                gpio_set_level(this->m_gpio_ms1, level);
                if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Set MS1 signal to " << level << ".\n\n";
            }
        }

        // Sets ms2
        inline void set_ms2(int level) {
            if (is_valid_signal(this->m_gpio_ms2, level)) {
                gpio_set_level(this->m_gpio_ms2, level);
                if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Set MS2 signal to " << level << ".\n\n";
            }
        }

        // Sets ms3
        inline void set_ms3(int level) {
            if (is_valid_signal(this->m_gpio_ms3, level)) {
                gpio_set_level(this->m_gpio_ms3, level);
                if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Set MS3 signal to " << level << ".\n\n";
            }
        }

        // Sets ~reset
        inline void set_not_rst(int level) {
            if (is_valid_signal(this->m_gpio_not_rst, level)) {
                gpio_set_level(this->m_gpio_not_rst, level);
                if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Set ~Reset signal to " << level << ".\n\n";
            }
        }

        // Sets ~sleep
        inline void set_not_slp(int level) {
            if (is_valid_signal(this->m_gpio_not_slp, level)) {
                gpio_set_level(this->m_gpio_not_slp, level);
                if constexpr (DEBUG_A4988)
                    std::cout << this->m_name << ": Set ~Sleep to " << level << ".\n\n";
            }
        }

        // Sets step (pulse)
        inline void set_step(const int level) {
            if (is_valid_signal(this->m_gpio_step, level)) {
                gpio_set_level(this->m_gpio_step, level);
                // Called too often to be worth printing
                //if constexpr (DEBUG_A4988) {
                    //std::cout << this->m_name << ": Set Step signal to " << level << "\n";
                //}
            }
        }

        // Sets direction (1 is clockwise, 0 is counterclockwise)
        inline void set_dir(int level) {
            if (is_valid_signal(this->m_gpio_dir, level)) {
                gpio_set_level(this->m_gpio_dir, level);
                if constexpr (DEBUG_A4988) {
                    if (level == 1)
                        std::cout << this->m_name << ": Set Direction signal to 1 (clockwise).\n\n";
                    else if(level == 0)
                        std::cout << this->m_name << ": Set Direction signal to 0 (counterclockwise).\n\n";
                }
            }
        }

        inline bool is_enabled() {
            return m_enabled;
        }
};

#endif /* __A4988_DRIVER_HPP__ */
/**
 * @file pwm.cpp
 * @brief Pulse Width Modulation
 * 
 */
#include "pwm.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include "driver/gpio.h"

void pwm::pwm_run() {
    if (this->m_gpio != GPIO_NUM_NC) {
        // pwm GPIO setup
        gpio_reset_pin(this->m_gpio);
        gpio_set_direction(this->m_gpio, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio, 0);

        while (true) {
            // on part of cycle
            if (this->m_duty_cycle > 0) {
                gpio_set_level(this->m_gpio, 1);
                std::this_thread::sleep_for(std::chrono::milliseconds(this->m_duty_cycle));
            }

            // off part of cycle
            if (this->m_duty_cycle < 100) {
                gpio_set_level(this->m_gpio, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(100 - this->m_duty_cycle));
            }
        }
    }
    else {
        std::cout << "Error: This signal is not assigned to a valid GPIO.\n\n";
    }
}
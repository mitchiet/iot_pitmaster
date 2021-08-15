/**
 * @file pwm.cpp
 * @brief Pulse Width Modulation
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "pwm.hpp"

#include <iostream>
#include <thread>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void pwm::pwm_run() {
    // pwm GPIO setup
    gpio_reset_pin(this->m_gpio);
    gpio_set_direction(this->m_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio, 0);

    while (1) {
        // on part of cycle
        if (this->m_duty_cycle > 0) {
            gpio_set_level(this->m_gpio, 1);
            vTaskDelay(this->m_duty_cycle / portTICK_PERIOD_MS);
        }

        // off part of cycle
        if (this->m_duty_cycle < 100) {
            gpio_set_level(this->m_gpio, 0);
            vTaskDelay((100 - this->m_duty_cycle) / portTICK_PERIOD_MS);
        }
    }
}

// pwm thread function
std::thread pwm::pwm_run_thread() {
    return std::thread( [this] {
        this->pwm_run();
    });
}
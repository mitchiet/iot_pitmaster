/**
 * @file a4988_driver.cpp
 * @brief Stepper Motor Driver
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "a4988_driver.hpp"

#include <iostream>
#include <thread>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// a4988 run function
void a4988_driver::a4988_run() {

    // ~enable
    gpio_reset_pin(this->m_gpio_not_en);
    gpio_set_direction(this->m_gpio_not_en, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_not_en, 1); // default

    // ms1
    gpio_reset_pin(this->m_gpio_ms1);
    gpio_set_direction(this->m_gpio_ms1, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_ms1, 0); // default

    // ms2
    gpio_reset_pin(this->m_gpio_ms2);
    gpio_set_direction(this->m_gpio_ms2, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_ms2, 0); // default

    // ms3
    gpio_reset_pin(this->m_gpio_ms3);
    gpio_set_direction(this->m_gpio_ms3, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_ms3, 0); // default

    // ~reset
    gpio_reset_pin(this->m_gpio_not_rst);
    gpio_set_direction(this->m_gpio_not_rst, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_not_rst, 1); // default

    // ~sleep
    gpio_reset_pin(this->m_gpio_not_slp);
    gpio_set_direction(this->m_gpio_not_slp, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_not_slp, 1); // default

    // step
    gpio_reset_pin(this->m_gpio_step);
    gpio_set_direction(this->m_gpio_step, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_step, 0); // default

    // dir
    gpio_reset_pin(this->m_gpio_dir);
    gpio_set_direction(this->m_gpio_dir, GPIO_MODE_OUTPUT);
    gpio_set_level(this->m_gpio_dir, 0); // default

    while (1) {
        if (this->m_enabled) {
            this->set_step(1);
            vTaskDelay(1 / portTICK_PERIOD_MS); // running at 1000 HZ, set pulse for 1 ms

            this->set_step(0);
            vTaskDelay(1 / portTICK_PERIOD_MS); // running at 1000 HZ, set pulse for 1 ms
        }
        else {
            this->set_step(0);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
}

// a4988 thread function
std::thread a4988_driver::a4988_run_thread() {
    return std::thread( [this] {
        this->a4988_run();
    });
}
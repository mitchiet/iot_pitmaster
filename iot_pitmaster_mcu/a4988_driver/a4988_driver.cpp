/**
 * @file a4988_driver.cpp
 * @brief Stepper Motor Driver
 *
 */
#include "a4988_driver.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include "driver/gpio.h"

#include "debug.hpp"

using namespace std::chrono_literals;

// Sets all the gpio to default states
// Default to enable off, and quarter step size
void a4988_driver::set_default_gpio_levels() {

    // ~enable
    if (this->m_gpio_not_en != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_not_en);
        gpio_set_direction(this->m_gpio_not_en, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_not_en, 1); // default
    }

    // ms1
    if (this->m_gpio_ms1 != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_ms1);
        gpio_set_direction(this->m_gpio_ms1, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_ms1, 0); // default
    }

    // ms2
    if (this->m_gpio_ms2 != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_ms2);
        gpio_set_direction(this->m_gpio_ms2, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_ms2, 1); // default
    }

    // ms3
    if (this->m_gpio_ms3 != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_ms3);
        gpio_set_direction(this->m_gpio_ms3, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_ms3, 0); // default
    }

    // ~reset
    if (this->m_gpio_not_rst != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_not_rst);
        gpio_set_direction(this->m_gpio_not_rst, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_not_rst, 1); // default
    }

    // ~sleep
    if (this->m_gpio_not_slp != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_not_slp);
        gpio_set_direction(this->m_gpio_not_slp, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_not_slp, 1); // default
    }

    // step
    if (this->m_gpio_step != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_step);
        gpio_set_direction(this->m_gpio_step, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_step, 0); // default
    }

    // dir
    if (this->m_gpio_dir != GPIO_NUM_NC) {
        gpio_reset_pin(this->m_gpio_dir);
        gpio_set_direction(this->m_gpio_dir, GPIO_MODE_OUTPUT);
        gpio_set_level(this->m_gpio_dir, 0); // default
    }
}

// Run the motor continuously
void a4988_driver::run_motor_continuous() {
    if constexpr (DEBUG_A4988)
        std::cout << this->m_name << ": Starting continuous motor run.\n\n";
    while (!this->m_stop_motor) {
        // Send a pulse to the a4988 to step
        if (this->m_enabled) {
            this->set_step(1);
            std::this_thread::sleep_for(1ms); // running at 1000 HZ, set pulse for 1 ms

            this->set_step(0);
            std::this_thread::sleep_for(1ms); // running at 1000 HZ, set pulse for 1 ms
        }
        // Case when the motor is not enabled, save energy
        else {
            std::this_thread::sleep_for(2ms);
        }
    }
    this->m_stop_motor = false;
}

// Run the motor for a certain amount of steps
void a4988_driver::run_motor_steps(int num_steps) {
    if constexpr (DEBUG_A4988)
        std::cout << this->m_name << ": Stepping " << num_steps << " times.\n\n";

    for (int i = 0; i < num_steps; i++) {

        // Send a step pulse unless the motor is stopped in the middle of the process
        if (this->m_stop_motor)
            break;

        this->set_step(1);
        std::this_thread::sleep_for(1ms); // running at 1000 HZ, set pulse for 1 ms

        this->set_step(0);
        std::this_thread::sleep_for(1ms); // running at 1000 HZ, set pulse for 1 ms
    }
    this->m_stop_motor = false;
}
/**
 * @file blowfan.cpp
 * @author Mitchell Taylor
 *                          
 * @brief 
 * @version 0.1
 * @date 2021-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "blowfan.hpp"

#include <iostream>
#include <pthread.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// default duty cycle to 0%
int blowfan::m_duty_cycle = 0;

// blowfan thread function
void* blowfan::blowfan_run(void* duty_cycle) {
    // blowfan GPIO setup
    gpio_reset_pin(GPIO_FAN_PWM);
    gpio_set_direction(GPIO_FAN_PWM, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_FAN_PWM, 0);

    // cast the duty cycle input to an int pointer
    int* on_percent_ptr = reinterpret_cast<int*>(duty_cycle);

    while (1) {
        if (*on_percent_ptr > 0) {
            // on part of cycle
            gpio_set_level(GPIO_FAN_PWM, 1);
            vTaskDelay(*on_percent_ptr / portTICK_PERIOD_MS);

            // off part of cycle
            if (*on_percent_ptr < 100) {
                gpio_set_level(GPIO_FAN_PWM, 0);
                vTaskDelay((100 - *on_percent_ptr) / portTICK_PERIOD_MS);
            }
        }
    }
}

// starts the thread that controls the blowfan voltage
bool blowfan::launch_fan_thread() {
    pthread_t blowfan_thread;
    int ret = pthread_create(&blowfan_thread, NULL, blowfan_run, (void*) &blowfan::m_duty_cycle);
    if (ret) {
        std::cout << "Error: The blowfan thread did not start.\n";
        return false;
    }

    return true;
}
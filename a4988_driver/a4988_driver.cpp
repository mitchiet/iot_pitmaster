/**
 * @file a4988_driver.cpp
 * @brief Stepper Motor Driver
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "a4988_driver.hpp"

#include <iostream>
#include <pthread.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// DEFAULTS
// ~enable signal
int a4988_driver::not_en = 1;
// ms1 signal
int a4988_driver::ms1 = 0;
// ms2 signal
int a4988_driver::ms2 = 0;
// ms3 signal
int a4988_driver::ms3 = 0;
// ~reset signal
int a4988_driver::not_rst = 1;
// ~sleep signal
int a4988_driver::not_slp = 1;
// step signal, on and off pulses
int a4988_driver::step = 0;
// direction signal
int a4988_driver::dir = 0;

// hopper motor thread function
void* a4988_driver::stepper_run(void* p) {
    // ~enable
    gpio_reset_pin(GPIO_NOT_EN);
    gpio_set_direction(GPIO_NOT_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_EN, a4988_driver::not_en);

    // ms1
    gpio_reset_pin(GPIO_MS1);
    gpio_set_direction(GPIO_MS1, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS1, a4988_driver::ms1);

    // ms2
    gpio_reset_pin(GPIO_MS2);
    gpio_set_direction(GPIO_MS2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS2, a4988_driver::ms2);

    // ms3
    gpio_reset_pin(GPIO_MS3);
    gpio_set_direction(GPIO_MS3, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS3, a4988_driver::ms3);

    // ~reset
    gpio_reset_pin(GPIO_NOT_RST);
    gpio_set_direction(GPIO_NOT_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_RST, a4988_driver::not_rst);

    // ~sleep
    gpio_reset_pin(GPIO_NOT_SLP);
    gpio_set_direction(GPIO_NOT_SLP, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_SLP, a4988_driver::not_slp);

    // step
    gpio_reset_pin(GPIO_STEP);
    gpio_set_direction(GPIO_STEP, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_STEP, a4988_driver::step);

    // dir
    gpio_reset_pin(GPIO_DIR);
    gpio_set_direction(GPIO_DIR, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DIR, a4988_driver::dir);

    while (1) {
        if (!a4988_driver::not_en) {
            a4988_driver::set_step(1);
            vTaskDelay(1 / portTICK_PERIOD_MS); // running at 1000 HZ, set pulse for 1 ms

            a4988_driver::set_step(0);
            vTaskDelay(1 / portTICK_PERIOD_MS); // running at 1000 HZ, set pulse for 1 ms
        }
        else {
            a4988_driver::set_step(0);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
}

// starts the thread that controls the stepper motor
bool a4988_driver::launch_a4988_driver_thread() {
    pthread_t stepper_thread;
    int ret = pthread_create(&stepper_thread, NULL, stepper_run, NULL);
    if (ret) {
        std::cout << "Error: The hopper motor thread did not start.\n";
        return false;
    }

    return true;
}
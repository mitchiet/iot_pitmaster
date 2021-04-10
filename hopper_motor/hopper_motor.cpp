/**
 * @file hopper_motor.cpp
 * @author Mitchell Taylor
 *                          
 * @brief 
 * @version 0.1
 * @date 2021-04-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "hopper_motor.hpp"

#include <iostream>
#include <pthread.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// DEFAULTS
// ~enable signal
int hopper_motor::not_en = 0;
// ms1 signal
int hopper_motor::ms1 = 0;
// ms2 signal
int hopper_motor::ms2 = 0;
// ms3 signal
int hopper_motor::ms3 = 0;
// ~reset signal
int hopper_motor::not_rst = 1;
// ~sleep signal
int hopper_motor::not_slp = 1;
// step signal, on and off pulses
int hopper_motor::step = 0;
// direction signal
int hopper_motor::dir = 0;

// hopper motor thread function
void* hopper_motor::stepper_run(void* p) {
    // ~enable
    gpio_reset_pin(GPIO_NOT_EN);
    gpio_set_direction(GPIO_NOT_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_EN, hopper_motor::not_en);

    // ms1
    gpio_reset_pin(GPIO_MS1);
    gpio_set_direction(GPIO_MS1, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS1, hopper_motor::ms1);

    // ms2
    gpio_reset_pin(GPIO_MS2);
    gpio_set_direction(GPIO_MS2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS2, hopper_motor::ms2);

    // ms3
    gpio_reset_pin(GPIO_MS3);
    gpio_set_direction(GPIO_MS3, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS3, hopper_motor::ms3);

    // ~reset
    gpio_reset_pin(GPIO_NOT_RST);
    gpio_set_direction(GPIO_NOT_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_RST, hopper_motor::not_rst);

    // ~sleep
    gpio_reset_pin(GPIO_NOT_SLP);
    gpio_set_direction(GPIO_NOT_SLP, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_SLP, hopper_motor::not_slp);

    // step
    gpio_reset_pin(GPIO_STEP);
    gpio_set_direction(GPIO_STEP, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_STEP, hopper_motor::step);

    // dir
    gpio_reset_pin(GPIO_DIR);
    gpio_set_direction(GPIO_DIR, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DIR, hopper_motor::dir);

    while (1) {
        if (!hopper_motor::not_en) {
            hopper_motor::set_step(1);
            vTaskDelay(1 / portTICK_PERIOD_MS); // running at 1000 HZ, set pulse for 1 ms

            hopper_motor::set_step(0);
            vTaskDelay(1 / portTICK_PERIOD_MS); // running at 1000 HZ, set pulse for 1 ms
        }
    }
}

// starts the thread that controls the stepper motor
bool hopper_motor::launch_hopper_motor_thread() {
    pthread_t stepper_thread;
    int ret = pthread_create(&stepper_thread, NULL, stepper_run, NULL);
    if (ret) {
        std::cout << "Error: The hopper motor thread did not start.\n";
        return false;
    }

    return true;
}
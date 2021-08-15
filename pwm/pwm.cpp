#include "pwm.hpp"

#include <iostream>
#include <pthread.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// default member variables
/// \todo delete with deletion of static
int pwm::m_duty_cycle = 0;
gpio_num_t pwm::m_gpio = GPIO_NUM_21;

// pwm thread function
void* pwm::pwm_run(void* p) {
    // pwm GPIO setup
    gpio_reset_pin(m_gpio);
    gpio_set_direction(m_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(m_gpio, 0);

    while (1) {
        // on part of cycle
        if (m_duty_cycle > 0) {
            gpio_set_level(m_gpio, 1);
            vTaskDelay(m_duty_cycle / portTICK_PERIOD_MS);
        }

        // off part of cycle
        if (m_duty_cycle < 100) {
            gpio_set_level(m_gpio, 0);
            vTaskDelay((100 - m_duty_cycle) / portTICK_PERIOD_MS);
        }
    }
}

// starts the thread that controls the pwm voltage
bool pwm::launch_pwm_thread() {
    pthread_t pwm_thread;
    int ret = pthread_create(&pwm_thread, NULL, pwm_run, NULL);
    if (ret) {
        std::cout << "Error: The pwm thread did not start.\n";
        return false;
    }

    return true;
}
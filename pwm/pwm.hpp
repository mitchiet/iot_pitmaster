/**
 * @file pwm.hpp
 * @brief Pulse Width Modulation
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __PWM_HPP__
#define __PWM_HPP__

#include <atomic>
#include <iostream>
#include <thread>

#include "driver/gpio.h"

class pwm {

    private:
        // the duty cycle of the pwm (0-100)%
        std::atomic<int> m_duty_cycle {0};
        // the gpio pin being used
        gpio_num_t m_gpio;

    public:
        inline pwm(const gpio_num_t gpio, const int duty_cycle) {
            this->m_gpio = gpio;
            this->m_duty_cycle = duty_cycle;
        }

        inline void set_duty_cycle(const int duty_cycle) {
            if (duty_cycle >= 0 || duty_cycle <= 100) {
                this->m_duty_cycle = duty_cycle;
                std::cout << "Set pwm duty cycle to " << duty_cycle << "%\n";
            }
            else {
                std::cout << "Duty cycle can only be set 0-100.\n";
            }
        }

        // main pwm logic function
        void pwm_run();

        // thread function
        std::thread pwm_run_thread();
};

#endif /* __PWM_HPP__ */
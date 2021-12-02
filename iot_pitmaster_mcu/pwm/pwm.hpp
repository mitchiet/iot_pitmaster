/**
 * @file pwm.hpp
 * @brief Pulse Width Modulation
 * 
 */

#ifndef __PWM_HPP__
#define __PWM_HPP__

#include <atomic>
#include <iostream>

#include "driver/gpio.h"

#include "debug.hpp"

class pwm {

    private:
        // The duty cycle of the pwm (0-100)%
        std::atomic<int8_t> m_duty_cycle {0};
        // The gpio pin being used
        gpio_num_t m_gpio;

    public:
        inline pwm(const gpio_num_t gpio, const int8_t duty_cycle) {
            this->m_gpio = gpio;
            this->m_duty_cycle = duty_cycle;
        }

        inline void set_duty_cycle(const int8_t duty_cycle) {
            if (duty_cycle >= 0 && duty_cycle <= 100) {
                this->m_duty_cycle = duty_cycle;
                if constexpr (DEBUG_PWM)
                    std::cout << "Set pwm duty cycle to " << static_cast<int>(duty_cycle) << "%.\n\n";
            }
            else {
                std::cout << "Duty cycle can only be set 0-100.\n\n";
            }
        }

        inline int8_t get_duty_cycle() {
            return this->m_duty_cycle;
        }

        // Main pwm logic function
        void pwm_run();
};

#endif /* __PWM_HPP__ */
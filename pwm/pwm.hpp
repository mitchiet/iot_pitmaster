/**
 * @file pwm.hpp
 * @author Mitchell Taylor
 *                          
 * @brief 
 * @version 0.1
 * @date 2021-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __PWM_HPP__
#define __PWM_HPP__

#include <iostream>
#include "driver/gpio.h"

/// \todo get rid of static

class pwm {

    private:
        // the duty cycle of the pwm (0-100)%
        static int m_duty_cycle;
        // the gpio pin being used
        static gpio_num_t m_gpio;

        // thread that runs
        static void* pwm_run(void* duty_cycle);

    public:
        inline pwm(const gpio_num_t gpio, const int duty_cycle) {
            m_gpio = gpio;
            m_duty_cycle = duty_cycle;
        }

        inline void set_duty_cycle(const int duty_cycle) {
            m_duty_cycle = duty_cycle;
            std::cout << "Set pwm duty cycle to " << duty_cycle << "%\n";
        }

        // thread launcher
        static bool launch_pwm_thread();

};

#endif /* __PWM_HPP__ */
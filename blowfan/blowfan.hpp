/**
 * @file blowfan.hpp
 * @author Mitchell Taylor
 *                          
 * @brief 
 * @version 0.1
 * @date 2021-04-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __BLOWFAN_HPP__
#define __BLOWFAN_HPP__

#include <iostream>
#include "driver/gpio.h"

// GPIO for blowfan
const gpio_num_t GPIO_FAN_PWM = GPIO_NUM_21;

class blowfan {

    private:
        // the duty cycle of the fan (0-100)%
        static int m_duty_cycle;

        // thread that runs
        static void* blowfan_run(void* duty_cycle);

    public:
        inline blowfan(const int duty_cycle) {
            m_duty_cycle = duty_cycle;
        }
        inline blowfan() {
            m_duty_cycle = 0;
        }
        inline ~blowfan(){
        }

        inline void set_duty_cycle(const int duty_cycle) {
            m_duty_cycle = duty_cycle;
            std::cout << "Set Blowfan duty cycle to " << duty_cycle << "%\n";
        }

        // thread launcher
        static bool launch_fan_thread();

};

#endif /* __BLOWFAN_HPP__ */
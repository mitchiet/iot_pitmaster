/**
 * @file hopper_motor.hpp
 * @author Mitchell Taylor
 *                          
 * @brief 
 * @version 0.1
 * @date 2021-04-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef __HOPPER_MOTOR_HPP__
#define __HOPPER_MOTOR_HPP__

#include "driver/gpio.h"

// GPIO for stepper driver
const gpio_num_t GPIO_NOT_EN = GPIO_NUM_19;
const gpio_num_t GPIO_MS1 = GPIO_NUM_18;
const gpio_num_t GPIO_MS2 = GPIO_NUM_5;
const gpio_num_t GPIO_MS3 = GPIO_NUM_17;
const gpio_num_t GPIO_NOT_RST = GPIO_NUM_16;
const gpio_num_t GPIO_NOT_SLP = GPIO_NUM_4;
const gpio_num_t GPIO_STEP = GPIO_NUM_0;
const gpio_num_t GPIO_DIR = GPIO_NUM_2;

class hopper_motor {

    private:

        // thread that runs
        static void* stepper_run(void* p);

        // ~enable signal
        static int not_en;
        // ms1 signal
        static int ms1;
        // ms2 signal
        static int ms2;
        // ms3 signal
        static int ms3;
        // ~reset signal
        static int not_rst;
        // ~sleep signal
        static int not_slp;
        // step signal, on and off pulses
        static int step;
        // direction signal
        static int dir;


    public:
        inline hopper_motor() {
        }

        inline ~hopper_motor(){
        }

        // thread launcher
        static bool launch_hopper_motor_thread();

        // sets ~enable
        inline static void set_not_en(int level) {
            not_en = level;
            gpio_set_level(GPIO_NOT_EN, not_en);
        }

        // sets ms1
        inline static void set_ms1(int level) {
            ms1 = level;
            gpio_set_level(GPIO_MS1, ms1);
        }

        // sets ms2
        inline static void set_ms2(int level) {
            ms2 = level;
            gpio_set_level(GPIO_MS2, ms2);
        }

        // sets ms3
        inline static void set_ms3(int level) {
            ms3 = level;
            gpio_set_level(GPIO_MS3, ms3);
        }

        // sets ~reset
        inline static void set_not_rst(int level) {
            not_rst = level;
            gpio_set_level(GPIO_NOT_RST, not_rst);
        }

        // sets ~sleep
        inline static void set_not_slp(int level) {
            not_slp = level;
            gpio_set_level(GPIO_NOT_SLP, not_slp);
        }

        // sets step (pulse)
        inline static void set_step(const int level) {
            step = level;
            gpio_set_level(GPIO_STEP, step);
        }

        // sets direction (1 is clockwise, 0 is counterclockwise)
        inline static void set_dir(int level) {
            dir = level;
            gpio_set_level(GPIO_DIR, dir);
        }

};

#endif /* __HOPPER_MOTOR_HPP__ */
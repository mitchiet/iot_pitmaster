/**
 * @file main.cpp
 * @brief Main Function
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include "sdkconfig.h"

#include "a4988_driver.hpp"
#include "ble.hpp"
#include "pwm.hpp"
#include "test.cpp"

#define PRINT_DEBUG_MODE (1)

// Blowfan PWM GPIO
constexpr gpio_num_t gpio_blowfan = GPIO_NUM_21;

// Hopper Motor Driver GPIO
constexpr gpio_num_t gpio_hopper_not_en = GPIO_NUM_18;
constexpr gpio_num_t gpio_hopper_ms1 = GPIO_NUM_5;
constexpr gpio_num_t gpio_hopper_ms2 = GPIO_NUM_17;
constexpr gpio_num_t gpio_hopper_ms3 = GPIO_NUM_16;
constexpr gpio_num_t gpio_hopper_not_rst = GPIO_NUM_4;
constexpr gpio_num_t gpio_hopper_not_slp = GPIO_NUM_0;
constexpr gpio_num_t gpio_hopper_step = GPIO_NUM_2;
constexpr gpio_num_t gpio_hopper_dir = GPIO_NUM_15;

// Damper Motor Driver GPIO
// Unfortunately, Tx and Rx is needed for print debug. This cause conflict with the damper signals using gpios 1 and 3.
#if PRINT_DEBUG_MODE == 1
constexpr gpio_num_t gpio_damper_not_en = GPIO_NUM_NC;
constexpr gpio_num_t gpio_damper_ms1 = GPIO_NUM_NC;
#else
constexpr gpio_num_t gpio_damper_not_en = GPIO_NUM_3;
constexpr gpio_num_t gpio_damper_ms1 = GPIO_NUM_1;
#endif
constexpr gpio_num_t gpio_damper_ms2 = GPIO_NUM_22;
constexpr gpio_num_t gpio_damper_ms3 = GPIO_NUM_23;
constexpr gpio_num_t gpio_damper_not_rst = GPIO_NUM_27;
constexpr gpio_num_t gpio_damper_not_slp = GPIO_NUM_14;
constexpr gpio_num_t gpio_damper_step = GPIO_NUM_12;
constexpr gpio_num_t gpio_damper_dir = GPIO_NUM_13;

extern "C" void app_main(void)
{
    std::cout << "Howdy world!\n";

    /// \todo fix the warning message partial data write
    ble ble_conn;
    ble_conn.init();

    pwm blowfan(gpio_blowfan, 0);
    std::thread blowfan_thread = blowfan.pwm_run_thread();
    blowfan_thread.detach();

    a4988_driver hopper_controller(gpio_hopper_not_en, gpio_hopper_ms1,
                                   gpio_hopper_ms2, gpio_hopper_ms3,
                                   gpio_hopper_not_rst, gpio_hopper_not_slp,
                                   gpio_hopper_step, gpio_hopper_dir);
    std::thread hopper_thread = hopper_controller.a4988_run_thread();
    hopper_thread.detach();

    // ms1 and ms3 dont work
    a4988_driver damper_controller(gpio_damper_not_en, gpio_damper_ms1,
                                   gpio_damper_ms2, gpio_damper_ms3,
                                   gpio_damper_not_rst, gpio_damper_not_slp,
                                   gpio_damper_step, gpio_damper_dir);
    std::thread damper_thread = damper_controller.a4988_run_thread();
    damper_thread.detach();

    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait a second

#if PRINT_DEBUG_MODE == 0

    // Test the blowfan.
    std::thread blowfan_tester([&] {
        test_pwm(blowfan);
    });
    blowfan_tester.detach();

    // Test the hopper motor.
    std::thread hopper_tester([&] {
        test_a4988_driver(hopper_controller);
    });
    hopper_tester.detach();

    // Test the damper motor.
    std::thread damper_tester([&] {
        test_a4988_driver(damper_controller);
    });
    damper_tester.detach();

    while(true) {sleep(2);}

#endif

///\todo Migrate the cli to test directory

#if PRINT_DEBUG_MODE == 1
    /* Necessary magic to make the console function properly*/
    /* Not needed in final product */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,   // installs uart driver
            256, 0, 0, NULL, 0) );
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);           // uses uart driver
    setvbuf(stdin, NULL, _IONBF, 0);    // sets stdin to not buffer
    setvbuf(stdout, NULL, _IONBF, 0);   // sets stdout to not buffer
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);      // carriage return when ENTER
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);    // newline behavior


    std::cout << "Beginning Command Loop.\n"; 
    while (1) {
        std::string signal_name = "";
        std::string level_str;
        int level = -1;

        // roundabout cin
        char* line = linenoise("\nEnter Command: "); // will work with MobaXTerm
        std::istringstream s(line);
        linenoiseFree(line); // free the memory
        s >> signal_name >> level_str;
        try{level = std::stoi(level_str);}
        catch(...) {
            level = -1;
            std::cout << "Error: Second argument must be numerical.\n";
        }

        /* BLOWFAN COMMANDS */
        if (signal_name == "duty_cycle") {
            if (level >= 0 || level <= 100) {
                blowfan.set_duty_cycle(level);
            }
            else {
                std::cout << "Error: Blowfan duty cycle must be between 0 and 100.\n";
            }
        }

        /* HOPPER MOTOR COMMANDS */
        else if (signal_name == "h_not_en")
            hopper_controller.set_not_en(level);
        else if (signal_name == "h_ms1")
            hopper_controller.set_ms1(level);
        else if (signal_name == "h_ms2")
            hopper_controller.set_ms2(level);
        else if (signal_name == "h_ms3")
            hopper_controller.set_ms3(level);
        else if (signal_name == "h_not_rst")
            hopper_controller.set_not_rst(level);
        else if (signal_name == "h_not_slp")
            hopper_controller.set_not_slp(level);
        else if (signal_name == "h_dir")
            hopper_controller.set_dir(level); // 1 is clockwise, 0 is counterclockwise

        /* DAMPER MOTOR COMMANDS */
        else if (signal_name == "d_not_en")
            damper_controller.set_not_en(level);
        else if (signal_name == "d_ms1")
            damper_controller.set_ms1(level);
        else if (signal_name == "d_ms2")
            damper_controller.set_ms2(level);
        else if (signal_name == "d_ms3")
            damper_controller.set_ms3(level);
        else if (signal_name == "d_not_rst")
            damper_controller.set_not_rst(level);
        else if (signal_name == "d_not_slp")
            damper_controller.set_not_slp(level);
        else if (signal_name == "d_dir")
            damper_controller.set_dir(level); // 1 is clockwise, 0 is counterclockwise

        // UNKNOWN
        else {
            std::cout << "Error: Not a recognized command.\n";
        }
    }

#endif

}
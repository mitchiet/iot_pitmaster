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
#include <pthread.h>
#include <thread>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"

#include "ble.hpp"
#include "pwm.hpp"
#include "hopper_motor.hpp"

constexpr gpio_num_t gpio_blowfan{GPIO_NUM_21};

extern "C" void app_main(void)
{
    std::cout << "Howdy world!\n";

    /// \todo fix the warning message partial data write
    ble ble_conn;
    ble_conn.init();

    pwm blowfan(gpio_blowfan, 0);
    std::thread blowfan_thread = blowfan.pwm_run_thread();
    blowfan_thread.detach();

    hopper_motor hm;
    hm.launch_hopper_motor_thread();

    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait a second

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

        /* STEPPER MOTOR COMMANDS */
        else if (signal_name == "not_en") {
            if (level == 1 || level == 0) {
                hm.set_not_en(level);
            }
            else {
                std::cout << "Error: ~Enable direction can only be set to 1 or 0.\n";
            }
        }

        else if (signal_name == "ms1") {
            if (level == 1 || level == 0) {
                hm.set_ms1(level);
            }
            else {
                std::cout << "Error: MS1 can only be set to 1 or 0.\n";
            }
        }

        else if (signal_name == "ms2") {
            if (level == 1 || level == 0) {
                hm.set_ms2(level);
            }
            else {
                std::cout << "Error: MS2 can only be set to 1 or 0.\n";
            }
        }

        else if (signal_name == "ms3") {
            if (level == 1 || level == 0) {
                hm.set_ms3(level);
            }
            else {
                std::cout << "Error: MS3 can only be set to 1 or 0.\n";
            }
        }

        else if (signal_name == "not_rst") {
            if (level == 1 || level == 0) {
                hm.set_not_rst(level);
            }
            else {
                std::cout << "Error: ~Reset can only be set to 1 or 0.\n";
            }
        }

        else if (signal_name == "not_slp") {
            if (level == 1 || level == 0) {
                hm.set_not_slp(level);
            }
            else {
                std::cout << "Error: ~Sleep can only be set to 1 or 0.\n";
            }
        }

        else if (signal_name == "dir") {
            if (level == 1 || level == 0) {
                hm.set_dir(level); // 1 is clockwise, 0 is counterclockwise
            }
            else {
                std::cout << "Error: Stepper direction can only be set to 1 or 0.\n";
            }
        }

        else {
            std::cout << "Error: Not a recognized command.\n";
        }
    }
    

    /* Print chip information */
    /**
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Free heap: %d\n", esp_get_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
    */
}
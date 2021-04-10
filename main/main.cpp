/**
 * @file main.cpp
 * @author Mitchell Taylor
 * @brief 
 * @version 0.1
 * @date 2021-03-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <iostream>
#include <pthread.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

#include "ble.hpp"
#include "blowfan.hpp"
#include "hopper_motor.hpp"

extern "C" void app_main(void)
{
    std::cout << "Howdy world!\n";

    /// \todo fix the warning message partial data write
    ble ble_conn;
    ble_conn.init();

    blowfan bf(100);
    bf.launch_fan_thread();

    hopper_motor hm;
    hm.launch_hopper_motor_thread();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    // demo direction change
    hm.set_dir(1);
    // demo that new fan speed can be set after creating the fan thread
    bf.set_duty_cycle(20);

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
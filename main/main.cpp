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

// GPIO for stepper driver
const gpio_num_t GPIO_NOT_EN = GPIO_NUM_19;
const gpio_num_t GPIO_MS1 = GPIO_NUM_18;
const gpio_num_t GPIO_MS2 = GPIO_NUM_5;
const gpio_num_t GPIO_MS3 = GPIO_NUM_17;
const gpio_num_t GPIO_NOT_RST = GPIO_NUM_16;
const gpio_num_t GPIO_NOT_SLP = GPIO_NUM_4;
const gpio_num_t GPIO_STEP = GPIO_NUM_0;
const gpio_num_t GPIO_DIR = GPIO_NUM_2;

extern "C" void app_main(void)
{
    std::cout << "Howdy world!\n";

    /// \todo fix the warning message partial data write
    ble ble_conn;
    ble_conn.init();

    blowfan bf(100);
    bf.launch_fan_thread();

    // demo that new fan speed can be set after creating the fan thread
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    bf.set_duty_cycle(20);


    /// \todo move all the following to its own file and thread
    // ~enable
    gpio_reset_pin(GPIO_NOT_EN);
    gpio_set_direction(GPIO_NOT_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_EN, 0);

    // ms1
    gpio_reset_pin(GPIO_MS1);
    gpio_set_direction(GPIO_MS1, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS1, 0);

    // ms2
    gpio_reset_pin(GPIO_MS2);
    gpio_set_direction(GPIO_MS2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS2, 0);

    // ms3
    gpio_reset_pin(GPIO_MS3);
    gpio_set_direction(GPIO_MS3, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_MS3, 0);

    // ~reset
    gpio_reset_pin(GPIO_NOT_RST);
    gpio_set_direction(GPIO_NOT_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_RST, 1);

    // ~sleep
    gpio_reset_pin(GPIO_NOT_SLP);
    gpio_set_direction(GPIO_NOT_SLP, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NOT_SLP, 1);

    // step
    gpio_reset_pin(GPIO_STEP);
    gpio_set_direction(GPIO_STEP, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_STEP, 0);

    // dir
    gpio_reset_pin(GPIO_DIR);
    gpio_set_direction(GPIO_DIR, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DIR, 0);


    while (1) {
        gpio_set_level(GPIO_STEP, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);

        gpio_set_level(GPIO_STEP, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
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
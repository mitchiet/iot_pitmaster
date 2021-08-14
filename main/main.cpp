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
/**
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <pthread.h>

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
#include "blowfan.hpp"
#include "hopper_motor.hpp"

extern "C" void app_main(void)
{
    std::cout << "Howdy world!\n";

    /// \todo fix the warning message partial data write
    ble ble_conn;
    ble_conn.init();

    blowfan bf(0);
    bf.launch_fan_thread();

    hopper_motor hm;
    hm.launch_hopper_motor_thread();

    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait a second

    // Necessary magic to make the console function properly
    // Not needed in final product 
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

        // BLOWFAN COMMANDS
        if (signal_name == "duty_cycle") {
            if (level >= 0 || level <= 100) {
                bf.set_duty_cycle(level);
            }
            else {
                std::cout << "Error: Blowfan duty cycle must be between 0 and 100.\n";
            }
        }

        // STEPPER MOTOR COMMANDS
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
    */

    // Print chip information
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

// ignore compiler warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wreorder"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"

#include <vector>

const gpio_num_t GPIO_SDO = GPIO_NUM_25; // slave data out
const gpio_num_t GPIO_CLK = GPIO_NUM_19; // spi clock
const gpio_num_t GPIO_CS1 = GPIO_NUM_32; // chip select 1
const gpio_num_t GPIO_CS2 = GPIO_NUM_33; // chip select 2
const gpio_num_t GPIO_CS3 = GPIO_NUM_26; // chip select 3

typedef struct // variables containing data 
{   
    float internal_C;
    float thermocouple_C;
    float thermocouple_F;
    bool fault;

} max31855_data_t;

esp_err_t ret;

spi_bus_config_t buscfg = // configuring spi bus
{
    .mosi_io_num        = -1,
    .miso_io_num        = GPIO_SDO,
    .sclk_io_num        = GPIO_CLK,
    .quadwp_io_num      = -1,
    .quadhd_io_num      = -1,
    .max_transfer_sz    = 4, // size of data transfer 32 bits
    .flags              = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_MISO | SPICOMMON_BUSFLAG_SCLK 
};

/// \todo callbacks for devcfgs

spi_device_interface_config_t devcfg1 = 
{
    .command_bits       = 0,
    .address_bits       = 0,
    .dummy_bits         = 0,            // note this; if dropping bits at start of cycle
    .mode               = 0,            // 0,1,2,3.  has to do with clock polarity, phase, and edge.  See wikipedia.org for spi mode numbers
    .duty_cycle_pos     = 0,            // setting to 0 defaults to 128, 50% duty cycle
    .cs_ena_pretrans    = 0,            // amount of spi bit-cycles cs activated before transmission.
    .cs_ena_posttrans   = 0,            // sim to pretrans, but trailing
    .clock_speed_hz     = 4*1000*1000,  // Clock out at  4 MHz
    .spics_io_num       = GPIO_CS1,      // Chip Select pin
    .queue_size         = 1,            // nr transactions at a time
    .pre_cb             = NULL,         // enable_thermocouple_chip,         //callback before trans
    .post_cb            = NULL          // disable_thermocouple_chip          //callback after trans
};

spi_device_interface_config_t devcfg2 = 
{
    .command_bits       = 0,
    .address_bits       = 0,
    .dummy_bits         = 0,            // note this; if dropping bits at start of cycle
    .mode               = 0,            // 0,1,2,3.  has to do with clock polarity, phase, and edge.  See wikipedia.org for spi mode numbers
    .duty_cycle_pos     = 0,            // setting to 0 defaults to 128, 50% duty cycle
    .cs_ena_pretrans    = 0,            // amount of spi bit-cycles cs activated before transmission.
    .cs_ena_posttrans   = 0,            // sim to pretrans, but trailing
    .clock_speed_hz     = 4*1000*1000,  // Clock out at  4 MHz
    .spics_io_num       = GPIO_CS2,     // Chip Select pin
    .queue_size         = 1,            // nr transactions at a time
    .pre_cb             = NULL,         // enable_thermocouple_chip,         //callback before trans
    .post_cb            = NULL          // disable_thermocouple_chip          //callback after trans
};

spi_device_interface_config_t devcfg3 = 
{
    .command_bits       = 0,
    .address_bits       = 0,
    .dummy_bits         = 0,            // note this; if dropping bits at start of cycle
    .mode               = 0,            // 0,1,2,3.  has to do with clock polarity, phase, and edge.  See wikipedia.org for spi mode numbers
    .duty_cycle_pos     = 0,            // setting to 0 defaults to 128, 50% duty cycle
    .cs_ena_pretrans    = 0,            // amount of spi bit-cycles cs activated before transmission.
    .cs_ena_posttrans   = 0,            // sim to pretrans, but trailing
    .clock_speed_hz     = 4*1000*1000,  // Clock out at  4 MHz
    .spics_io_num       = GPIO_CS3,     // Chip Select pin
    .queue_size         = 1,            // nr transactions at a time
    .pre_cb             = NULL,         // enable_thermocouple_chip,         //callback before trans
    .post_cb            = NULL          // disable_thermocouple_chip          //callback after trans
};

spi_device_handle_t spi1, spi2, spi3;

typedef struct {   
    spi_device_handle_t* spi;
    const spi_device_interface_config_t* devcfg;
    const gpio_num_t gpio_num;
} spi_interface; 

std::vector<spi_interface> spi_interfaces {
    {&spi1, &devcfg1, GPIO_CS1},
    {&spi2, &devcfg2, GPIO_CS2},
    {&spi3, &devcfg3, GPIO_CS3}
};

extern "C" void app_main(void) {

    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);

    for(spi_interface& spi_dev : spi_interfaces) {
        gpio_set_direction(spi_dev.gpio_num, GPIO_MODE_OUTPUT);
        gpio_set_level(spi_dev.gpio_num, 1);
        ret = spi_bus_add_device(HSPI_HOST, spi_dev.devcfg, spi_dev.spi);
    }

    uint32_t thermocouple_data;
    max31855_data_t dt;

    spi_transaction_t t =
    {
        .flags = SPI_TRANS_USE_RXDATA,
        .cmd = 0,
        .addr = 0,
        .length = 32,
        .rxlength = 32,
        .user = NULL,
        .tx_buffer = NULL,
        .rx_buffer = NULL,
    };

    for(int a = 0; a < 5; a++)
    {
        for(spi_interface& spi_dev : spi_interfaces)
        {
            gpio_set_level(spi_dev.gpio_num, 0);
            spi_device_transmit(*spi_dev.spi, &t);
            gpio_set_level(spi_dev.gpio_num, 1);
            
            thermocouple_data = ((uint32_t)t.rx_data[0] << 24) | ((uint32_t)t.rx_data[1] << 16) | ((uint32_t)t.rx_data[2] << 8) | (uint32_t)t.rx_data[3];
    
            int16_t thermal_data = (int16_t)(thermocouple_data >> 18);
            dt.thermocouple_C = (thermal_data * 0.25f); // temp data gets changed to celcius 
        
            dt.thermocouple_F  = dt.thermocouple_C * 1.8f + 32.0f; // celcius converted to farenheit 
            
            dt.fault = (thermocouple_data >> 16) & 1; // shift 16 and mask with 1 since only one bit 
            
            printf("Chip Select: %d\n", spi_dev.gpio_num);
            printf("C: %4.2f \n", dt.thermocouple_C);
            printf("F: %4.2f \n\n", dt.thermocouple_F);
            printf("error: 0x%x\n\n", dt.fault);

            vTaskDelay(100 / portTICK_RATE_MS); // 100 ms delay 
        }
    }
}

#pragma GCC diagnostic pop        // Restore previous default behaviour
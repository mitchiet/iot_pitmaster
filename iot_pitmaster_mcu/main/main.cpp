/**
 * @file main.cpp
 * @brief Main Function
 * 
 */
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string>
#include <thread>

#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "sdkconfig.h"

#include "a4988_driver.hpp"
#include "bluetooth.hpp"
#include "max31855.hpp"
#include "pid_control.hpp"
#include "pwm.hpp"
#include "test.cpp"

using namespace std::chrono_literals;

// Blowfan PWM GPIO
constexpr gpio_num_t gpio_blowfan = GPIO_NUM_21;

// Hopper Motor Driver GPIO
constexpr gpio_num_t gpio_hopper_not_en = GPIO_NUM_18;
constexpr gpio_num_t gpio_hopper_ms1 = GPIO_NUM_5;
constexpr gpio_num_t gpio_hopper_ms2 = GPIO_NUM_17;
constexpr gpio_num_t gpio_hopper_ms3 = GPIO_NUM_16;
constexpr gpio_num_t gpio_hopper_not_rst = GPIO_NUM_4;
constexpr gpio_num_t gpio_hopper_not_slp = GPIO_NUM_4;
constexpr gpio_num_t gpio_hopper_step = GPIO_NUM_0;
constexpr gpio_num_t gpio_hopper_dir = GPIO_NUM_2;

// Damper Motor Driver GPIO
constexpr gpio_num_t gpio_damper_not_en = GPIO_NUM_15;
constexpr gpio_num_t gpio_damper_ms1 = GPIO_NUM_22;
constexpr gpio_num_t gpio_damper_ms2 = GPIO_NUM_23;
constexpr gpio_num_t gpio_damper_ms3 = GPIO_NUM_27;
constexpr gpio_num_t gpio_damper_not_rst = GPIO_NUM_14;
constexpr gpio_num_t gpio_damper_not_slp = GPIO_NUM_14;
constexpr gpio_num_t gpio_damper_step = GPIO_NUM_12;
constexpr gpio_num_t gpio_damper_dir = GPIO_NUM_13;

// MAX31855 GPIO
constexpr gpio_num_t gpio_clk = GPIO_NUM_19;
constexpr gpio_num_t gpio_signal_out = GPIO_NUM_25;
constexpr gpio_num_t gpio_chamber_chip_select = GPIO_NUM_32;
constexpr gpio_num_t gpio_meat1_chip_select = GPIO_NUM_33;
constexpr gpio_num_t gpio_meat2_chip_select = GPIO_NUM_26;

// SPI for Thermocouples
constexpr spi_bus_config_t spi_bus_cfg = // configuring spi bus
{
    .mosi_io_num        = -1,
    .miso_io_num        = gpio_signal_out,
    .sclk_io_num        = gpio_clk,
    .quadwp_io_num      = -1,
    .quadhd_io_num      = -1,
    .max_transfer_sz    = 4, // size of data transfer 32 bits
    .flags              = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_MISO | SPICOMMON_BUSFLAG_SCLK,
    .intr_flags         = 0
};

extern "C" void app_main(void)
{
    std::cout << "Howdy world!\n";

    // Initialize Bluetooth
    bt::init_bluetooth();

    // Blowfan Motor Thread
    pwm blowfan(gpio_blowfan, 0);
    std::thread blowfan_thread = std::thread([&]() {blowfan.pwm_run();});
    blowfan_thread.detach();

    // Hopper Auger Motor Object
    a4988_driver hopper_controller("Hopper Motor", gpio_hopper_not_en, gpio_hopper_ms1,
                                   gpio_hopper_ms2, gpio_hopper_ms3,
                                   gpio_hopper_not_rst, gpio_hopper_not_slp,
                                   gpio_hopper_step, gpio_hopper_dir);

    // Damper Controller Motor Object
    a4988_driver damper_controller("Damper Motor", gpio_damper_not_en, gpio_damper_ms1,
                                   gpio_damper_ms2, gpio_damper_ms3,
                                   gpio_damper_not_rst, gpio_damper_not_slp,
                                   gpio_damper_step, gpio_damper_dir);

    // Create the ADC objects for the thermocouples
    max31855 tc_chamber(gpio_clk, gpio_signal_out, gpio_chamber_chip_select);
    tc_chamber.name("Chamber1 Thermocouple");
    max31855 tc_meat1(gpio_clk, gpio_signal_out, gpio_meat1_chip_select);
    tc_meat1.name("Meat1 Thermocouple");
    max31855 tc_meat2(gpio_clk, gpio_signal_out, gpio_meat2_chip_select);
    tc_meat2.name("Meat2 Thermocouple");

    // Add the devices to the SPI bus
    if (spi_bus_initialize(HSPI_HOST, &spi_bus_cfg, 0) == 0) {
        spi_bus_add_device(HSPI_HOST, &tc_chamber.dev_cfg(), &tc_chamber.dev_handle());
        spi_bus_add_device(HSPI_HOST, &tc_meat1.dev_cfg(), &tc_meat1.dev_handle());
        spi_bus_add_device(HSPI_HOST, &tc_meat2.dev_cfg(), &tc_meat2.dev_handle());
    }

    // Wait a half second
    std::this_thread::sleep_for(500ms);

    // Object for PID/manual control algorithm
    pid_control main_pid_control(blowfan, hopper_controller, damper_controller, 
        tc_chamber, tc_meat1, tc_meat2);

    // Make sure Bluetooth messages get sent to the pid_control object just created
    bt::set_bt_msg_dest(&main_pid_control);

    // Make a separate thread for PID/manual control
    std::thread pid_control_thread = std::thread([&]() {main_pid_control.pid_control_run();});
    pid_control_thread.detach();

    // Neverending test loop, use MobaXTerm to input
    test::debug_print_loop(main_pid_control);
}
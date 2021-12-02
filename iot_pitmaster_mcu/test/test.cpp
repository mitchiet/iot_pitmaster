#include <chrono>
#include <sstream>
#include <thread>

#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"

#include "a4988_driver.hpp"
#include "bluetooth.hpp"
#include "max31855.hpp"
#include "pid_control.hpp"
#include "pwm.hpp"

using namespace std::chrono_literals;

namespace test {

void test_a4988_driver(a4988_driver& driver) {
    while(true) {
        driver.set_not_en(0);
        std::this_thread::sleep_for(3s);
        driver.set_ms1(1);
        std::this_thread::sleep_for(3s);
        driver.set_ms2(1);
        std::this_thread::sleep_for(3s);
        driver.set_ms3(1);
        std::this_thread::sleep_for(3s);
        driver.set_not_rst(0);
        std::this_thread::sleep_for(3s);
        driver.set_not_rst(1);
        std::this_thread::sleep_for(3s);
        driver.set_not_slp(0);
        std::this_thread::sleep_for(3s);
        driver.set_not_slp(1);
        std::this_thread::sleep_for(3s);
        driver.set_dir(1);
        std::this_thread::sleep_for(3s);
        driver.set_dir(0);
        std::this_thread::sleep_for(3s);
        driver.set_ms1(0);
        std::this_thread::sleep_for(3s);
        driver.set_ms2(0);
        std::this_thread::sleep_for(3s);
        driver.set_ms3(0);
        std::this_thread::sleep_for(3s);
        driver.set_not_en(1);
        std::this_thread::sleep_for(3s);
    }
}

void test_pwm(pwm& motor) {
    while(true) {
        motor.set_duty_cycle(20);
        std::this_thread::sleep_for(3s);
        motor.set_duty_cycle(60);
        std::this_thread::sleep_for(3s);
        motor.set_duty_cycle(100);
        std::this_thread::sleep_for(3s);
    }
}

void quick_test_motors(pwm& blowfan, a4988_driver& hopper_controller, a4988_driver& damper_controller) {

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

    while(true) {std::this_thread::sleep_for(2s);}
}

void debug_print_loop(pid_control& main_pid_control) {
    
    // Necessary magic to make the console function properly
    // Not needed in final product
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,   // installs uart driver
            256, 0, 0, NULL, 0) );
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);           // uses uart driver
    setvbuf(stdin, NULL, _IONBF, 0);    // sets stdin to not buffer
    setvbuf(stdout, NULL, _IONBF, 0);   // sets stdout to not buffer

    std::cout << "Beginning Command Loop.\n"; 
    while (1) {
        std::string signal_name = "";
        std::string level_str;
        int level = -1;

        // Roundabout cin
        char* line = linenoise(": "); // will work with MobaXTerm
        std::istringstream s(line);
        linenoiseFree(line); // free the memory
        s >> signal_name >> level_str;

        // Continue on empty string
        if (signal_name == "")
            continue;

        // Restart option
        if (signal_name == "restart") {
            esp_restart();
            continue;
        }

        // Run the hopper motor continuously for testing
        if (signal_name == "hopper_run") {
            std::thread hopper_thread = std::thread([&] {main_pid_control.hopper_controller()->run_motor_continuous();});
            hopper_thread.detach();
            continue;
        }

        // Stop the hopper motor
        if (signal_name == "hopper_stop") {
            main_pid_control.hopper_controller()->stop_motor();
            continue;
        }

        // Run the damper motor continuously for testing
        if (signal_name == "damper_run") {
            std::thread damper_thread = std::thread([&] {main_pid_control.damper_controller()->run_motor_continuous();});
            damper_thread.detach();
            continue;
        }

        // Stop the damper motor
        if (signal_name == "damper_stop") {
            main_pid_control.damper_controller()->stop_motor();
            continue;
        }

        // Simulate receive input fuel BT message
        if (signal_name == "input_fuel") {
            in_msg_hopper msg {MSG_HOPPER, true}; // true means input fuel
            main_pid_control.handle_bt_msg(&msg);
            continue;
        }

        // Simulate receive close damper BT message
        if (signal_name == "close_damper") {
            in_msg_damper msg {MSG_DAMPER, false}; // false means close the damper
            main_pid_control.handle_bt_msg(&msg);
            continue;
        }

        // Simulate receive open damper BT message
        if (signal_name == "open_damper") {
            in_msg_damper msg {MSG_DAMPER, true}; // true means open the damper
            main_pid_control.handle_bt_msg(&msg);
            continue;
        }

        // Simulate receive set chamber temp BT message
        if (signal_name == "set_chamber") {
            in_msg_temp_C msg {MSG_CHAMBER_TEMP, 320}; // set chamber temp to 320 C
            main_pid_control.handle_bt_msg(&msg);
            continue;
        }

        // Hello Bluetooth message
        if (signal_name == "send_bt_hello") {
            const uint64_t test_string = 0x000a6f6c6c6568; // "hello\n\0" little endian
            bt::send_data(test_string);
            continue;
        }

        // Test Bluetooth write for chamber
        if (signal_name == "send_bt_chamber") {
            // Send some sample data from the chamber thermocouple
            max31855_data_t test_temp_data = main_pid_control.tc_chamber()->read();
            out_msg_temp_C out_msg {MSG_CHAMBER_TEMP, test_temp_data};
            bt::send_data(out_msg);
            continue;
        }

        // Test Bluetooth write for meat1
        if (signal_name == "send_bt_meat1") {
            // send some sample data from the meat1 thermocouple
            max31855_data_t test_temp_data = main_pid_control.tc_meat1()->read();
            out_msg_temp_C out_msg {MSG_CHAMBER_TEMP, test_temp_data};
            bt::send_data(out_msg);
            continue;
        }

        // Test Bluetooth write for meat2
        if (signal_name == "send_bt_meat2") {
            // Send some sample data from the meat2 thermocouple
            max31855_data_t test_temp_data = main_pid_control.tc_meat2()->read();
            out_msg_temp_C out_msg {MSG_CHAMBER_TEMP, test_temp_data};
            bt::send_data(out_msg);
            continue;
        }

        // Everything else needs a level (motor speed, motor on/off status, or thermocouple)
        try{level = std::stoi(level_str);}
        catch(...) {
            level = -1;
            std::cout << "Error: Second argument must be numerical.\n";
            continue;
        }

        /* BLOWFAN COMMANDS */
        if (signal_name == "duty_cycle") {
            if (level >= 0 || level <= 100) {
                main_pid_control.blowfan()->set_duty_cycle(level);
            }
            else {
                std::cout << "Error: Blowfan duty cycle must be between 0 and 100.\n";
            }
        }

        /* HOPPER MOTOR COMMANDS */
        else if (signal_name == "h_not_en")
            main_pid_control.hopper_controller()->set_not_en(level);
        else if (signal_name == "h_ms1")
            main_pid_control.hopper_controller()->set_ms1(level);
        else if (signal_name == "h_ms2")
            main_pid_control.hopper_controller()->set_ms2(level);
        else if (signal_name == "h_ms3")
            main_pid_control.hopper_controller()->set_ms3(level);
        else if (signal_name == "h_not_rst")
            main_pid_control.hopper_controller()->set_not_rst(level);
        else if (signal_name == "h_not_slp")
            main_pid_control.hopper_controller()->set_not_slp(level);
        else if (signal_name == "h_dir")
            main_pid_control.hopper_controller()->set_dir(level); // 1 is clockwise, 0 is counterclockwise

        /* DAMPER MOTOR COMMANDS */
        else if (signal_name == "d_not_en")
            main_pid_control.damper_controller()->set_not_en(level);
        else if (signal_name == "d_ms1")
            main_pid_control.damper_controller()->set_ms1(level);
        else if (signal_name == "d_ms2")
            main_pid_control.damper_controller()->set_ms2(level);
        else if (signal_name == "d_ms3")
            main_pid_control.damper_controller()->set_ms3(level);
        else if (signal_name == "d_not_rst")
            main_pid_control.damper_controller()->set_not_rst(level);
        else if (signal_name == "d_not_slp")
            main_pid_control.damper_controller()->set_not_slp(level);
        else if (signal_name == "d_dir")
            main_pid_control.damper_controller()->set_dir(level); // 1 is clockwise, 0 is counterclockwise

        // THERMOCOUPLES
        else if (signal_name == "chamber" && level == 1)
            std::future<max31855_data_t> temp = main_pid_control.tc_chamber()->async_read();
        else if (signal_name == "meat" && level == 1)
            std::future<max31855_data_t> temp = main_pid_control.tc_meat1()->async_read();
        else if (signal_name == "meat" && level == 2)
            std::future<max31855_data_t> temp = main_pid_control.tc_meat2()->async_read();

        // UNKNOWN
        else {
            std::cout << "Error: Not a recognized command.\n";
        }
    }
}

}
/**
 * @file pid_control.hpp
 * @brief Proportional Integral Derivative
 * 
 */
#ifndef __PID_CONTROL_HPP__
#define __PID_CONTROL_HPP__

#include <chrono>
#include <functional>
#include <queue>

#include "a4988_driver.hpp"
#include "max31855.hpp"
#include "pwm.hpp"

using namespace std::chrono_literals;

// Function that creates a task queue thread (used specifically on the stepper motors)
std::thread create_task_queue(std::queue<std::function<void()>>& thread_queue);

// The type of message being sent or received
enum msg_type : uint8_t {
    MSG_MODE = 0,
    MSG_CHAMBER_TEMP = 1,
    MSG_MEAT1_TEMP = 2,
    MSG_MEAT2_TEMP = 3,
    MSG_BLOWFAN = 4,
    MSG_HOPPER = 5,
    MSG_DAMPER = 6
};

// Basic message
struct __attribute__ ((packed)) in_msg_basic {
    msg_type type;
};

// MSG_MODE
struct __attribute__ ((packed)) in_msg_mode {
    msg_type type;
    bool mode;
};

// MSG_CHAMBER_TEMP, MSG_MEAT1_TEMP, MSG_MEAT2_TEMP receive
struct __attribute__ ((packed)) in_msg_temp_C {
    msg_type type;
    int16_t temp_C; // temp in Celsius
};

// MSG_BLOWFAN
struct __attribute__ ((packed)) in_msg_blowfan {
    msg_type type;
    int8_t duty_cycle; // duty cycle (0-100)%;
};

// MSG_HOPPER
struct __attribute__ ((packed)) in_msg_hopper {
    msg_type type;
    bool input_fuel; // true means input fuel
};

// MSG_DAMPER
struct __attribute__ ((packed)) in_msg_damper {
    msg_type type;
    bool position_open; // open is true, closed is false
};

// Struct to send individual temperature data
// MSG_CHAMBER_TEMP, MSG_MEAT1_TEMP, MSG_MEAT2_TEMP
struct __attribute__ ((packed)) out_msg_temp_C {
    msg_type type;
    max31855_data_t temp_data_chamber;
};

// Struct to send all temperature and motor data
struct __attribute__ ((packed)) out_msg_all_data {
    max31855_data_t temp_data_chamber;
    max31855_data_t temp_data_meat1;
    max31855_data_t temp_data_meat2;
    int8_t duty_cycle; // duty cycle (0-100)%;
    bool input_fuel; // treat like bool, 1 means input fuel
    bool position_open; // treat like bool, open is true, closed is false
};


class pid_control {

    private:

        // Pointers to important pieces of the system
        pwm* m_blowfan;
        a4988_driver* m_hopper_controller;
        a4988_driver* m_damper_controller;
        max31855* m_tc_chamber;
        max31855* m_tc_meat1;
        max31855* m_tc_meat2;

        // Status variables
        float m_set_point {0};
        bool m_damper_open {false};

        // Mode and cook status
        bool m_mode_auto {true};
        bool m_cook_started {false};

        // Emergency ignore BT
        bool m_ignore_bt {false};

        // Task queues for the stepper motors
        std::queue<std::function<void()>> m_hopper_task_queue;
        std::queue<std::function<void()>> m_damper_task_queue;

    public:

        inline pid_control(pwm& blowfan, a4988_driver& hopper_controller, a4988_driver& damper_controller, 
                max31855& tc_chamber, max31855& tc_meat1, max31855& tc_meat2) {
            this->m_blowfan = &blowfan;
            this->m_hopper_controller = &hopper_controller;
            this->m_damper_controller = &damper_controller;
            this->m_tc_chamber = &tc_chamber;
            this->m_tc_meat1 = &tc_meat1;
            this->m_tc_meat2 = &tc_meat2;

            // Creates a tasker thread for the hopper task queue
            std::thread hopper_tasker = create_task_queue(this->m_hopper_task_queue);
            hopper_tasker.detach();

            // Creates a tasker thread for the damper task queue
            std::thread damper_tasker = create_task_queue(this->m_damper_task_queue);
            damper_tasker.detach();
        }

        // GETTERS
        pwm* blowfan() {return this->m_blowfan;}
        a4988_driver* hopper_controller() {return this->m_hopper_controller;}
        a4988_driver* damper_controller() {return this->m_damper_controller;}
        max31855* tc_chamber() {return this->m_tc_chamber;}
        max31855* tc_meat1() {return this->m_tc_meat1;}
        max31855* tc_meat2() {return this->m_tc_meat2;}

        // Creates a task to input fuel and adds it to the hopper task queue
        void task_input_fuel();

        // Shutdown all grill operation
        void emergency_shutdown();

        // Creates a task to open the damper and adds it to the damper task queue
        void task_open_damper();

        // Creates a task to close the damper and adds it to the damper task queue
        void task_close_damper();

        // Gathers all data to be sent to Android app
        out_msg_all_data get_system_status();

        // Main pid_control logic function
        void pid_control_run();

        // Function for handling BT messages received
        template <typename T>
        void handle_bt_msg(const T* p_msg) {

            const in_msg_basic* basic_msg = reinterpret_cast<const in_msg_basic*>(p_msg);
            switch(basic_msg->type) {

            // The Android app had a mode change
            case MSG_MODE: {
                const in_msg_mode* msg = reinterpret_cast<const in_msg_mode*>(p_msg);
                std::cout << "Received from Android App: change mode to mode " << msg->mode << ".\n\n";
                if (!this->m_ignore_bt) {
                    this->m_mode_auto = msg->mode;
                }
                else {
                    std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                }
                break;
            }

            // The Android app set a temperature for the chamber
            case MSG_CHAMBER_TEMP: {
                const in_msg_temp_C* msg = reinterpret_cast<const in_msg_temp_C*>(p_msg);
                std::cout << "Received from Android App: set the chamber temperature to " <<
                        std::dec << msg->temp_C << " degrees Celsius.\n\n";
                if (!this->m_ignore_bt) {
                    this->m_set_point = msg->temp_C;
                    this->m_cook_started = true;
                }
                else {
                    std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                }
                break;
            }

            // The Android app set a temperature for meat 1
            case MSG_MEAT1_TEMP: {
                const in_msg_temp_C* msg = reinterpret_cast<const in_msg_temp_C*>(p_msg);
                std::cout << "Received from Android App: set meat1 temperature to " <<
                        std::dec << msg->temp_C << " degrees Celsius.\n\n";
                if (this->m_ignore_bt) {
                    std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                }
                break;
            }

            // The Android app set a temperature for meat 2
            case MSG_MEAT2_TEMP: {
                const in_msg_temp_C* msg = reinterpret_cast<const in_msg_temp_C*>(p_msg);
                std::cout << "Received from Android App: set meat2 temperature to " <<
                        std::dec << msg->temp_C << " degrees Celsius.\n\n";
                if (this->m_ignore_bt) {
                    std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                }
                break;
            }

            // The Android app set a duty cycle for the blow fan
            case MSG_BLOWFAN: {
                const in_msg_blowfan* msg = reinterpret_cast<const in_msg_blowfan*>(p_msg);
                std::cout << "Received from Android App: set blowfan duty cycle to " <<
                        std::dec << msg->duty_cycle << "%.\n\n";
                if (!this->m_ignore_bt) {
                    this->m_blowfan->set_duty_cycle(msg->duty_cycle);
                }
                else {
                    std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                }
                break;
            }

            // The Android app told the hopper to input more fuel
            case MSG_HOPPER: {
                const in_msg_hopper* msg = reinterpret_cast<const in_msg_hopper*>(p_msg);
                if (msg->input_fuel) {
                    std::cout << "Received from Android App: input more fuel.\n\n";
                    if (!this->m_ignore_bt) {
                        this->task_input_fuel();
                    }
                    else {
                        std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                    }
                }
                else {
                    std::cout << "Received from Android App: don't input more fuel.\n\n";
                    if (this->m_ignore_bt) {
                        std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                    }
                }
                break;
            }

            // The Android app told the damper to open or close
            case MSG_DAMPER: {
                const in_msg_damper* msg = reinterpret_cast<const in_msg_damper*>(p_msg);
                if (msg->position_open) {
                    std::cout << "Received from Android App: open the damper.\n\n";
                    if (!this->m_ignore_bt) {
                        this->task_open_damper();
                    }
                    else {
                        std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                    }
                }
                else {
                    std::cout << "Received from Android App: close the damper.\n\n";
                    if (!this->m_ignore_bt) {
                        this->task_close_damper();
                    }
                    else {
                        std::cout << "Ignoring command since the system is in Emergency Shutdown Mode.\n\n";
                    }
                }
                break;
            }

            // Unknown message received
            default: {
                std::cout << "Received unknown Bluetooth message. Message type = " <<
                        std::dec << basic_msg->type << "\n\n";
                break;
            }
            }
        }
};

#endif /* __PID_CONTROL_HPP__ */
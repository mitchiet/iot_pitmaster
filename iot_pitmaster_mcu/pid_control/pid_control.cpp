#include "pid_control.hpp"

#include <chrono>
#include <functional>
#include <thread>

#include "a4988_driver.hpp"
#include "bluetooth.hpp"
#include "debug.hpp"
#include "max31855.hpp"
#include "pwm.hpp"

using namespace std::chrono_literals;

#define DAMPER_OPEN_CLOSE_STEP_COUNT (75)
#define HOPPER_INPUT_FUEL_STEP_COUNT (1600)
#define HOPPER_INPUT_FUEL_INTERVAL (500)

// PID Algorithm tuner variables
static float Kp = 1;
static float Ki = .08;
static float Kd = .5;
static float dt = 1;

static float integral_err {0};
static float prev_val {0};

// pid_control run function
void pid_control::pid_control_run() {
    while (true) {

        // Update interval
        std::this_thread::sleep_for(1s);

        // Get status of thermocouples and motors
        const out_msg_all_data system_data = this->get_system_status();

        // Send status to Android app
        if (bt::is_bt_connected()) {
            bt::send_data(system_data);
        }

        // Check to make sure the chamber is not on fire, shut down if so
        if (system_data.temp_data_chamber.thermocouple_C > 316 ||
            system_data.temp_data_meat1.thermocouple_C > 316 ||
            system_data.temp_data_meat2.thermocouple_C > 316) {
            std::cout << "Entering Emergency Shutdown Mode due to excessive heat.\n\n";
            this->emergency_shutdown();
        }

        // PID logic
        // Make sure a temp has been selected and is in autonomous mode
        if (this->m_cook_started && this->m_mode_auto) {

            float pv_err = this->m_set_point - system_data.temp_data_chamber.thermocouple_C;
            integral_err += (this->m_set_point - system_data.temp_data_chamber.thermocouple_C)*dt;
            float deriv_err = (system_data.temp_data_chamber.thermocouple_C - prev_val)/dt;

            float output = Kp*pv_err + Ki*integral_err + Kd*deriv_err;
            std::cout << "PID output: " << output << ".\n\n";

            // Set the blow fan duty cycle
            if (output > 100)
                this->blowfan()->set_duty_cycle(100);
            else if (output < 0)
                this->blowfan()->set_duty_cycle(0);
            else
                this->blowfan()->set_duty_cycle(static_cast<int8_t>(output));

            // Control damper based on current temperature
            // Need to heat up, open damper
            if (!system_data.position_open && pv_err > 5)
                this->task_open_damper();
            // Need to cool down, close damper
            else if(system_data.position_open && pv_err <= 0)
                this->task_close_damper();

            static int cycles = 0;
            if (cycles == HOPPER_INPUT_FUEL_INTERVAL) {
                this->task_input_fuel();
                cycles = 0;
            }
            cycles++;
        }
        else {
            // Start the algorithm from scratch next time, erase integral history
            integral_err = 0;
        }

        // Always record the previous temp value
        if (!system_data.temp_data_chamber.fault) {
            prev_val = system_data.temp_data_chamber.thermocouple_C;
        }
    }
}

// Gathers all data to be sent to Android app
out_msg_all_data pid_control::get_system_status() {
    // Read data from the thermocouples
    max31855_data_t chamber_data = this->m_tc_chamber->read();
    max31855_data_t meat1_data = this->m_tc_meat1->read();
    max31855_data_t meat2_data = this->m_tc_meat2->read();

    // For testing without a thermocouple available
    if constexpr (DEBUG_SEND_HARDCODED_TEMP) {
        chamber_data.thermocouple_C = 75;
        chamber_data.fault = 0;
        meat1_data.thermocouple_C = 72.25;
        meat1_data.fault = 0;
        meat2_data.thermocouple_C = 70.5;
        meat2_data.fault = 1;
    }

    const int8_t duty_cycle = this->m_blowfan->get_duty_cycle();
    const bool hopper_enabled = this->m_hopper_controller->is_enabled();
    const bool damper_open = this->m_damper_open;

    const out_msg_all_data out_data {chamber_data, meat1_data, meat2_data, duty_cycle, hopper_enabled, damper_open};
    return out_data;
}

// Creates a task to input fuel and adds it to the hopper task queue
void pid_control::task_input_fuel() {
    std::function<void()> input_fuel = [&]() {
        std::cout << "Inputting fuel.\n\n";
        this->m_hopper_controller->set_dir(0);
        this->m_hopper_controller->set_not_en(0);
        this->m_hopper_controller->run_motor_steps(HOPPER_INPUT_FUEL_STEP_COUNT);
        this->m_hopper_controller->set_not_en(1);
    };
    this->m_hopper_task_queue.push(input_fuel);
}

// Creates a task to open the damper and adds it to the damper task queue
void pid_control::task_open_damper() {
    std::function<void()> open_damper = [&]() {
        if (!this->m_damper_open) {
            std::cout << "Opening damper.\n\n";
            this->m_damper_controller->set_dir(0);
            this->m_damper_controller->set_not_en(0);
            this->m_damper_controller->run_motor_steps(DAMPER_OPEN_CLOSE_STEP_COUNT);
            this->m_damper_controller->set_not_en(1);
            this->m_damper_open = true;
        }
        else {
            std::cout << "Damper is already open.\n\n";
        }
    };
    this->m_damper_task_queue.push(open_damper);
}

// Creates a task to close the damper and adds it to the damper task queue
void pid_control::task_close_damper() {
    std::function<void()> close_damper = [&]() {
        if (this->m_damper_open) {
            std::cout << "Closing damper.\n\n";
            this->m_damper_controller->set_dir(1);
            this->m_damper_controller->set_not_en(0);
            this->m_damper_controller->run_motor_steps(DAMPER_OPEN_CLOSE_STEP_COUNT);
            this->m_damper_controller->set_not_en(1);
            this->m_damper_open = false;
        }
        else {
            std::cout << "Damper is already closed.\n\n";
        }
    };
    this->m_damper_task_queue.push(close_damper);
}

// Shutdown all grill operation
void pid_control::emergency_shutdown() {
    this->m_ignore_bt = true;
    this->m_cook_started = false;

    // Clear task queues
    this->m_hopper_task_queue = {};
    this->m_damper_task_queue = {};

    // Adjust grill parts to decrease temperature
    this->task_close_damper();
    this->blowfan()->set_duty_cycle(0);

    // Chill for a bit and restart the MCU
    std::this_thread::sleep_for(5s);
    esp_restart();
};

// Function that creates a task queue thread (used specifically on the stepper motors)
std::thread create_task_queue(std::queue<std::function<void()>>& func_queue) {

    std::thread tasker([&]() {
        while (true) {
            if(func_queue.empty()) {
                std::this_thread::sleep_for(100ms);
            }
            else {
                func_queue.front()();
                func_queue.pop();
            }
        }
    });
    return tasker;
}
/**
 * @file max31855.hpp
 * @brief Temperature Analog-to-Digital Converter
 * 
 */
#ifndef __MAX31855_HPP__
#define __MAX31855_HPP__

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"

#include <future>
#include <string>

// Struct that contains external temp and fault bit
struct __attribute__ ((packed)) max31855_data_t {
    float thermocouple_C;
    bool fault;
};

class max31855 {

    private:

        spi_device_interface_config_t m_dev_cfg = 
        {
            .command_bits       = 0,
            .address_bits       = 0,
            .dummy_bits         = 0,            // note this; if dropping bits at start of cycle
            .mode               = 0,            // 0,1,2,3.  has to do with clock polarity, phase, and edge.
            .duty_cycle_pos     = 0,            // setting to 0 defaults to 128, 50% duty cycle
            .cs_ena_pretrans    = 0,            // amount of spi bit-cycles cs activated before transmission.
            .cs_ena_posttrans   = 0,            // sim to pretrans, but trailing
            .clock_speed_hz     = 4*1000*1000,  // Clock out at  4 MHz
            .input_delay_ns     = 0,
            .spics_io_num       = -1,
            .flags              = 0,
            .queue_size         = 1,            // nr transactions at a time
            .pre_cb             = NULL,
            .post_cb            = NULL
        };

        spi_device_handle_t m_dev_handle;

        gpio_num_t m_clk;
        gpio_num_t m_signal_out;
        gpio_num_t m_chip_select;

        std::string m_name;

    public:

        inline max31855(const gpio_num_t clk, const gpio_num_t signal_out, const gpio_num_t chip_select) {
            this->m_clk = clk;
            this->m_signal_out = signal_out;
            this->m_chip_select = chip_select;

            this->m_dev_cfg.spics_io_num = this->m_chip_select;      // Chip Select pin

            gpio_set_direction(this->m_chip_select, GPIO_MODE_OUTPUT);
            gpio_set_level(this->m_chip_select, 1);
        }

        inline spi_device_interface_config_t& dev_cfg() {
            return m_dev_cfg;
        }

        inline spi_device_handle_t& dev_handle() {
            return m_dev_handle;
        }

        inline void name(std::string name) {
            this->m_name = name;
        }

        inline std::string name() {
            return this->m_name;
        }

        max31855_data_t read();
        std::future<max31855_data_t> async_read();
};

#endif /* __MAX31855_HPP__ */
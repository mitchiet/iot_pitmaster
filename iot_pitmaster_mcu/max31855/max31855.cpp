/**
 * @file max31855.cpp
 * @brief Temperature Analog-to-Digital Converter
 * 
 */
#include "max31855.hpp"

#include <future>
#include <iostream>

#include "debug.hpp"

struct max31855_data_t max31855::read() {

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

    try {
        gpio_set_level(this->m_chip_select, 0);
        spi_device_transmit(this->dev_handle(), &t);
        gpio_set_level(this->m_chip_select, 1);
    }
    catch(...) {
        std::cout << "Could not transmit SPI.\n\n";
    }

    max31855_data_t dt;

    const uint32_t thermocouple_data = (static_cast<uint32_t>(t.rx_data[0]) << 24) | (static_cast<uint32_t>(t.rx_data[1]) << 16) |
            (static_cast<uint32_t>(t.rx_data[2]) << 8) | (static_cast<uint32_t>(t.rx_data[3]));

    const bool external_sign_bit = thermocouple_data >> 31 & 1;
    if (external_sign_bit) {
        // use sign bit, and divide by a 4 for the 2-bit decimal value
        dt.thermocouple_C = 0.25f*static_cast<int16_t>((thermocouple_data >> 18) | 0xc000);
    }
    else {
        // ignore sign bit, and divide by a 4 for the 2-bit decimal value
        dt.thermocouple_C = 0.25f*static_cast<int16_t>((thermocouple_data >> 18) & 0x3fff);
    }
    dt.fault = (thermocouple_data >> 16) & 1; // the bit that reads 1 when any fault is observed

    const bool open_circuit_fault = thermocouple_data & 1; // the bit that reads 1 for an open circuit fault
    const bool short_gnd_fault = (thermocouple_data >> 1) & 1; // the bit that reads 1 for a short-to-ground fault
    const bool short_vcc_fault = (thermocouple_data >> 2) & 1; // the bit that reads 1 for a short-to-vcc fault

    const bool internal_sign_bit = (thermocouple_data >> 15) & 1;
    float internal_temp {0};
    if (internal_sign_bit) {
        // use sign bit, and divide by a 16 for the 4-bit decimal value
        internal_temp = 0.0625f*static_cast<int16_t>((thermocouple_data >> 4) | 0xfe00);
    }
    else {
        // ignore sign bit, and divide by a 16 for the 4-bit decimal value
        internal_temp = 0.0625f*static_cast<int16_t>((thermocouple_data >> 4) & 0x07ff);
    }

    if constexpr (DEBUG_THERMOCOUPLE) {
        if (!dt.fault) {
            std::cout << "Name: " << this->name() <<
                    "\nCelsius: " << dt.thermocouple_C << ", Fahrenheit: " << dt.thermocouple_C * 1.8f + 32.0f  <<
                    "\nInternal Celsius: " << internal_temp << "\n\n";
        }
        else if (open_circuit_fault) {
            std::cout << "An open circuit fault occured for " << this->name() << "\n\n";
        }
        else if (short_gnd_fault) {
            std::cout << "A short-to-ground fault occured for " << this->name() << "\n\n";
        }
        else if (short_vcc_fault) {
            std::cout << "A short-to-vcc fault occured for " << this->name() << "\n\n";
        }
    }

    return dt;
}

std::future<max31855_data_t> max31855::async_read() {
    return std::async(std::launch::async, &max31855::read, this);
}
/**
 * @file ble.hpp
 * @author Mitchell Taylor
 * @brief Bluetooth
 * 
 */
#ifndef __BLUETOOTH_HPP__
#define __BLUETOOTH_HPP__

#include <stdint.h>

#include "pid_control.hpp"

namespace bt {

// Initialize Bluetooth
bool init_bluetooth();

void set_bt_msg_dest(pid_control* bt_pid_control);

inline bool bt_connected {false};
inline void set_bt_connected(bool value) {
    bt_connected = value;
}
inline bool is_bt_connected() {
    return bt_connected;
}

bool write_uint8_p(uint8_t* p_data_packet, int len);
template <typename T>
bool send_data(T& data_packet) {
    return write_uint8_p((uint8_t*)&data_packet, sizeof(data_packet));
}

}

#endif /* __BLUETOOTH_HPP__ */
/**
 * @file bluetooth.cpp
 * @author Mitchell Taylor, and example_spp_acceptor_demo.c by ESP-IDF
 * @brief Bluetooth
 * 
 */
#include "bluetooth.hpp"

#include <iostream>
#include <string>

#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_err.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "debug.hpp"
#include "pid_control.hpp"

#define SPP_SERVER_NAME "SPP_SERVER"

static const esp_spp_mode_t esp_spp_mode {ESP_SPP_MODE_CB};
static const esp_spp_sec_t esp_spp_sec_mask {ESP_SPP_SEC_NONE};
static const esp_spp_role_t esp_spp_role {ESP_SPP_ROLE_SLAVE};

static uint32_t conn_handle {0};

pid_control* bt_pid_control_dest {nullptr};

void bt::set_bt_msg_dest(pid_control* bt_pid_control) {
    bt_pid_control_dest = bt_pid_control;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {

    // SPP is initialized
    case ESP_SPP_INIT_EVT: {
        std::cout << "SPP: Received ESP_SPP_INIT_EVT\n\n";
        esp_bt_dev_set_device_name("IoT Pitmaster");
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        esp_spp_start_srv(esp_spp_sec_mask, esp_spp_role, 0, SPP_SERVER_NAME);
        break;
    }

    // SPP is uninitialized
    case ESP_SPP_UNINIT_EVT: {
        std::cout << "SPP: Received ESP_SPP_UNINIT_EVT\n\n";
        break;
    }

    // Service Discovery Protocol (SDP) is complete
    case ESP_SPP_DISCOVERY_COMP_EVT: {
        std::cout << "SPP: Received ESP_SPP_DISCOVERY_COMP_EVT\n\n";
        break;
    }

    // SPP client connection open
    case ESP_SPP_OPEN_EVT: {
        std::cout << "SPP: Received ESP_SPP_OPEN_EVT\n\n";
        bt::set_bt_connected(true);
        break;
    }

    // SPP connection is closed
    case ESP_SPP_CLOSE_EVT: {
        std::cout << "SPP: Received ESP_SPP_CLOSE_EVT\n\n";
        bt::set_bt_connected(false);
        conn_handle = 0;
        break;
    }

    // SPP server is started
    case ESP_SPP_START_EVT: {
        std::cout << "SPP: Received ESP_SPP_START_EVT\n\n";
        break;
    }

    // SPP client initiates a connection
    case ESP_SPP_CL_INIT_EVT: {
        std::cout << "SPP: Received ESP_SPP_CL_INIT_EVT\n\n";
        break;
    }

    // SPP connection received data
    case ESP_SPP_DATA_IND_EVT: {
        std::string read_val(reinterpret_cast<char*>(param->data_ind.data), param->data_ind.len);

        if constexpr (DEBUG_READ_BT) {
            char hex_str[5*param->data_ind.len + 1];
            int idx = 0;
            for (auto& hex_char : read_val) {
                snprintf(hex_str + idx, 6, "0x%02hhx ", hex_char);
                idx += 5;
            }
            std::cout << "SPP: Received ESP_SPP_DATA_IND_EVT, length = " << std::to_string(param->data_ind.len) << "\n" <<
                    "Received Bits: " << hex_str << "\n\n";
        }

        // Send only the character data
        bt_pid_control_dest->handle_bt_msg(read_val.c_str());
        break;
    }

    // SPP connection congestion status changed
    case ESP_SPP_CONG_EVT: {
        std::cout << "SPP: Received ESP_SPP_CONG_EVT\n\n";
        break;
    }

    // SPP write operation completes
    case ESP_SPP_WRITE_EVT: {
        if constexpr (DEBUG_WRITE_BT)
            std::cout << "SPP: Received ESP_SPP_WRITE_EVT, length = " << std::to_string(param->write.len) << "\n\n";
        break;
    }

    // SPP Server connection open
    case ESP_SPP_SRV_OPEN_EVT: {
        conn_handle = param->start.handle;
        bt::set_bt_connected(true);
        std::cout << "SPP: Received ESP_SPP_SRV_OPEN_EVT\n\n";
        break;
    }

    // SPP server stopped
    case ESP_SPP_SRV_STOP_EVT: {
        std::cout << "SPP: Received ESP_SPP_SRV_STOP_EVT\n\n";
        break;
    }

    default: {
        break;
    }
    }
}

static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {

    case ESP_BT_GAP_CONFIG_EIR_DATA_EVT: {
        if constexpr (DEBUG_GAP_BT)
            std::cout << "GAP: Received ESP_BT_GAP_CONFIG_EIR_DATA_EVT\n\n";
        break;
    }

    case ESP_BT_GAP_MODE_CHG_EVT: {
        if constexpr (DEBUG_GAP_BT) {
            const int mode = param->mode_chg.mode;
            std::cout << "GAP: Received ESP_BT_GAP_MODE_CHG_EVT, mode: " << mode << "\n\n";
        }
        break;
    }

    default: {
        if constexpr (DEBUG_GAP_BT)
            std::cout << "GAP: Received event: #" << event << "\n\n";
        break;
    }
    }
    return;
}

// Initialize Bluetooth
bool bt::init_bluetooth() {

    // Init Non-Volatile Storage (NVS)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    // Init Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        std::cout << "Error: initialize bluetooth controller failed\n\n";
        return false;
    }

    // Get the base MAC address
    uint8_t base_mac_addr[6] = {0};
    ret = esp_efuse_mac_get_default(base_mac_addr);
    std::cout << "Using \"" << std::hex <<
            "0x" << static_cast<int>(base_mac_addr[0]) << " 0x" << static_cast<int>(base_mac_addr[1]) <<
            " 0x" << static_cast<int>(base_mac_addr[2]) << " 0x" << static_cast<int>(base_mac_addr[3]) <<
            " 0x" << static_cast<int>(base_mac_addr[4]) << " 0x" << static_cast<int>(base_mac_addr[5]) <<
            "\" as base MAC address\n\n" << std::dec;
    esp_base_mac_addr_set(base_mac_addr);

    // Get the Bluetooth MAC address
    uint8_t bt_mac_addr[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(bt_mac_addr, ESP_MAC_BT));
    std::cout << "Using \"" << std::hex <<
            "0x" << static_cast<int>(bt_mac_addr[0]) << " 0x" << static_cast<int>(bt_mac_addr[1]) <<
            " 0x" << static_cast<int>(bt_mac_addr[2]) << " 0x" << static_cast<int>(bt_mac_addr[3]) <<
            " 0x" << static_cast<int>(bt_mac_addr[4]) << " 0x" << static_cast<int>(bt_mac_addr[5]) <<
            "\" as BT MAC address\n\n" << std::dec;

    // Enable Bluetooth controller
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        std::cout << "Error: enable bluetooth controller failed.\n\n";
        return false;
    }

    // Set BT power, N0 and P3 are default
    if ((ret = esp_bredr_tx_power_set(ESP_PWR_LVL_N0, ESP_PWR_LVL_P3))) {
        std::cout << "Error: set BT power failed.\n\n";
        return false;
    }

    // Init Bluedroid
    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        std::cout << "Error: initialize bluedroid failed.\n\n";
        return false;
    }

    // Enable Bluedroid
    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        std::cout << "Error: enable bluedroid failed.\n\n";
        return false;
    }

    // Register Generic Access Profile (GAP) callback function
    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        std::cout << "Error: GAP register failed.\n\n";
        return false;
    }

    // Register Serial Port Profile (SPP) callback function
    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        std::cout << "Error: SPP register failed.\n\n";
        return false;
    }

    // Initialize Serial Port Profile (SPP)
    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        std::cout << "Error: initialize SPP failed.\n\n";
        return false;
    }

    // Successfully initialized
    return true;
}

bool bt::write_uint8_p(uint8_t * p_data_packet, int len) {
    if (is_bt_connected()) {
        esp_err_t ret = esp_spp_write(conn_handle, len, p_data_packet);
        if (ret == ESP_OK) {
            return true;
        }
    }
    else {
        std::cout << "Unable to send BT message since there is no connection.\n\n";
    }
    return false;
}
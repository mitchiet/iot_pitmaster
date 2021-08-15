/**
 * @file ble.cpp
 * @author Mitchell Taylor, some code repurposed from ESP-IDF 
 *                          ble_ancs example
 * @brief Bluetooth
 * 
 */
#include <iostream>
#include <cstring>
#include "ble.hpp"
#include "ble_cfg.hpp"

/// \todo Potentially add security measures such as passkey
/// \todo Add the little blue info sign that other bluetooth devices have

uint8_t ble::m_adv_cfg_done = 0;

// Constructor
ble::ble() {
}

// Deconstructor, potentially deinit the bluetooth
ble::~ble() {
}

// Initialize
bool ble::init() {
    esp_err_t ret;

    // Initialize NVS.
    // Stands for Non-volatile Flash Storage.
    // Must initialize this storage before init bluetooth.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Release the memory associated with bluetooth classic since we are only using BLE (Bluetooth Low Energy).
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));


    // Configure and enable the controller for bluetooth.
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        std::cout << __func__ << "() ERROR: Init bt controller failed: " << esp_err_to_name(ret) << "\n";
        return false;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        std::cout << __func__ << "() ERROR: Enable bt controller failed: " << esp_err_to_name(ret) << "\n";
        return false;
    }

    // Android bluetooth init and enable functions.
    ret = esp_bluedroid_init();
    if (ret) {
        std::cout << __func__ << "() ERROR: Init bluetooth failed: " << esp_err_to_name(ret) << "\n";
        return false;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        std::cout << __func__ << "() ERROR: Enable bluetooth failed: " << esp_err_to_name(ret) << "\n";
        return false;
    }
    
    // Callback for GATTC (Generic Attribute Profile - Client).
    // GATT describes how data is transferred once a bluetooth connection is made.
    ret = esp_ble_gattc_register_callback(gattc_event_handler);
    if (ret){
        std::cout << __func__ << "() ERROR: gGATTC register error, error code = " << ret << "\n";
        return false;
    }

    // Callback for GAP (Generic Access Profile).
    // GAP describes how the device makes itself available and establishes a connection.
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        std::cout << __func__ << "() ERROR: GAP register error, error code = " << ret << "\n";
        return false;
    }

    // Registers a GATTC callback with app_id = 0.
    ret = esp_ble_gattc_app_register(0);
    if (ret){
        std::cout << __func__ << "() ERROR: GATTC app register error, error code = " << ret << "\n";
        return false;
    }

    // Sets the MTU (Max Trasmission Unit (in bytes)) to 500.
    ret = esp_ble_gatt_set_local_mtu(500);
    if (ret){
        std::cout << __func__ << "() ERROR: Set local  MTU failed, error code = " << ret << "\n";
        return false;
    }
    
    // Successfully initialized
    return true;
}


void ble::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        
        // Event signalling the scan response data is set to complete.
        // Will start advertizing.
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT: {
            ble::m_adv_cfg_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (ble::m_adv_cfg_done == 0){
                std::cout << "Attempting to start advertizing.\n";
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        }
        
        // Event signalling the advertizing data is set to complete.
        // Will start advertizing.
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:{
            ble::m_adv_cfg_done &= (~ADV_CONFIG_FLAG);
            if (ble::m_adv_cfg_done == 0){
                std::cout << "Attempting to start advertizing.\n";
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        }
        
        // Event signalling advertising start success or fail.
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:{
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                std::cout << "ERROR: Advertising start failed, error status = " << param->adv_start_cmpl.status << "\n";
                break;
            }
            std::cout << "Advertising start success.\n";
            break;
        }

        // Event that sets up the config for the BLE connection.
        // This runs before advertizing is started.
        case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT: {
            if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                std::cout << "ERROR: config local privacy failed, error status = " << param->local_privacy_cmpl.status << "\n";
                break;
            }

            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_config);
            if (ret) {
                std::cout << "ERROR: config adv data failed, error code = " << ret << "\n";
            } else {
                ble::m_adv_cfg_done |= ADV_CONFIG_FLAG;
            }

            ret = esp_ble_gap_config_adv_data(&scan_rsp_config);
            if (ret) {
                std::cout << "ERROR: config adv data failed, error code = " << ret << "\n";
            } else {
                ble::m_adv_cfg_done |= SCAN_RSP_CONFIG_FLAG;
            }

            break;
        }

        default:
            break;

    }
}


void ble::gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
    switch (event) {
        
        // Event that occurs when the GATT client is registered
        case ESP_GATTC_REG_EVT: {
            std::cout << "GATT Client registered.\n";

            // set the device name
            esp_ble_gap_set_device_name("IoT_Pitmaster"); // causes a warning

            // sets a logo somewhere that I can't find yet
            esp_ble_gap_config_local_icon(ESP_BLE_APPEARANCE_GENERIC_PHONE);

            // generate a resolvable random address, starts the advertising later on
            esp_ble_gap_config_local_privacy(true);

            break;
        }

        // Made a connection with a device.
        case ESP_GATTC_CONNECT_EVT: {
            std::cout << "Connected to device.\n";
            gl_profile_tab[0].conn_id = param->connect.conn_id; // profile 0
            std::memcpy(gl_profile_tab[0].remote_bda, param->connect.remote_bda, sizeof(esp_bd_addr_t)); // profile 0
            int ret = esp_ble_gattc_send_mtu_req(gattc_if, param->connect.conn_id);
            if (ret)
                std::cout << "Error: MTU config error\n";
            
            // open the connection
            esp_ble_gattc_open(gl_profile_tab[0].gattc_if, gl_profile_tab[0].remote_bda, BLE_ADDR_TYPE_RANDOM, true); // profile 0
            break;
        }

        // The device disconnected, so advertizing needs to be started again.
        case ESP_GATTC_DISCONNECT_EVT: {
            std::cout << "Device disconnected.\n";
            std::cout << "Attempting to start advertizing again.\n";
            esp_ble_gap_start_advertising(&adv_params);
            break;
        }

        case ESP_GATTC_OPEN_EVT: {
            std::cout << "Connection opened.\n";
            break;
        }
        
        default:
            break;
    }
}


void ble::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {

    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            std::cout << "ERROR: Reg app failed, app_id %04x, status %d" << param->reg.app_id << param->reg.status << "\n";
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    // Currently there is only one profile: A
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
    
}
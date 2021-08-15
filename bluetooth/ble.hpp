/**
 * @file ble.hpp
 * @author Mitchell Taylor, some code repurposed from ESP-IDF 
 *                          ble_ancs example
 * @brief Bluetooth
 * 
 */

#ifndef __BLE_HPP__
#define __BLE_HPP__

#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatt_common_api.h"
#include "nvs_flash.h"



class ble {

    private:
        
    public:
        ble();
        ~ble();

        static bool init();

        // used in advertizing config
        static uint8_t m_adv_cfg_done;

        /**
         * Callback for GAP (Generic Access Profile)
         */
        static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

        /**
         * Callback for GATTC (Generic Attribute Profile - Client)
         */
        static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

        /**
         * GATTC handler for specific bluetooth profile
         */
        static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
};

#endif /* __BLE_HPP__ */
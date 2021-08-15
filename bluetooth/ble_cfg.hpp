/**
 * @file ble_cfg.hpp
 * @author Mitchell Taylor, some code repurposed from ESP-IDF 
 *                          ble_ancs example
 * @brief Bluetooth Config Structs
 * 
 */

#include "ble.hpp"

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

// hidd = Human Interface Device Daemon I think
// uuid = Universally Unique IDentifier
static uint8_t hidd_service_uuid128[] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};

// config struct for scanning BLE
static esp_ble_adv_data_t scan_rsp_config = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0, // defaulting to avoid warning
    .max_interval = 0, // defaulting to avoid warning
    .appearance = 0, // defaulting to avoid warning
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0, // defaulting to avoid warning
    .p_service_data = NULL, // defaulting to avoid warning
    .service_uuid_len = 0, // defaulting to avoid warning
    .p_service_uuid = NULL, // defaulting to avoid warning
    .flag = 0, // defaulting to avoid warning
};        

// config struct for advertizing BLE
static esp_ble_adv_data_t adv_config = {
    .set_scan_rsp = false,
    .include_name = true, // defaulting to avoid warning
    .include_txpower = false,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = ESP_BLE_APPEARANCE_GENERIC_HID,
    .manufacturer_len = 0, // defaulting to avoid warning
    .p_manufacturer_data = NULL, // defaulting to avoid warning
    .service_data_len = 0, // defaulting to avoid warning
    .p_service_data = NULL, // defaulting to avoid warning
    .service_uuid_len = sizeof(hidd_service_uuid128),
    .p_service_uuid = hidd_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// parameters used when advertizing
static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x100,
    .adv_int_max         = 0x100,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr           = {0},                     // initialize to avoid warning
    .peer_addr_type      = BLE_ADDR_TYPE_PUBLIC,    // initialize to avoid warning
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// a GATTC profile struct
struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t notification_source_handle;
    uint16_t data_source_handle;
    uint16_t contol_point_handle;
    esp_bd_addr_t remote_bda;
    uint16_t MTU_size;
};

#define PROFILE_NUM 1
// array of GATTC profiles
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [0] = {
        .gattc_cb = ble::gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        .app_id = 0,
        .conn_id = 0,
        .service_start_handle = 0,
        .service_end_handle = 0,
        .notification_source_handle = 0,
        .data_source_handle = 0,
        .contol_point_handle = 0,
        .remote_bda = {0},
        .MTU_size = 0,
    }
};
#ifndef SD_BLE_ANCS_NUS_H__
#define SD_BLE_ANCS_NUS_H__
#include "sdk_config.h"
#include "nrf_sdm.h"
#include "time_slot.h"
#include "wireless_sim_phone.h"
#include "flash_data_write.h"

#define ANCS_NOTIF_LED_PATTERN_TYPE_INCOMING_CALL         0
#define ANCS_NOTIF_LED_PATTERN_TYPE_MISSED_CALL           1
#define ANCS_NOTIF_LED_PATTERN_TYPE_SOCIAL                2
#define ANCS_NOTIF_LED_PATTERN_TYPE_EMAIL                 3
#define ANCS_NOTIF_LED_PATTERN_TYPE_DEFAULT               4

#define ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER                   (ANCS_NOTIF_LED_PATTERN_TYPE_DEFAULT + 1)

void nus_send_bytes(uint8_t *bytes_string, uint16_t bytes_length);
void init_ble_ancs_c(void);
void start_ble_ancs_c(void);
void softdevice_parameter_init(void);
void power_manage(void);

#endif


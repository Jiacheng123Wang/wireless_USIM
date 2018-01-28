/**
 * Copyright (c) 2012 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 *
 * @defgroup ble_sdk_apple_notification_main main.c
 * @{
 * @ingroup ble_sdk_app_apple_notification
 * @brief Apple Notification Client Sample Application main file. Disclaimer:
 * This client implementation of the Apple Notification Center Service can and
 * will be changed at any time by Nordic Semiconductor ASA.
 *
 * Server implementations such as the ones found in iOS can be changed at any
 * time by Apple and may cause this client implementation to stop working.
 *
 * This file contains the source code for a sample application using the Apple
 * Notification Center Service Client.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "peer_manager.h"
#include "app_timer.h"
#include "fds.h"
#include "nrf_delay.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "ble_db_discovery.h"
#include "nrf_ble_ancs_c.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define ATTR_DATA_SIZE                 BLE_ANCS_ATTR_DATA_MAX                       /**< Allocated size for attribute data. */

#define APP_BLE_OBSERVER_PRIO          1                                            /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG           1                                            /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_FAST_INTERVAL          40                                           /**< The advertising interval (in units of 0.625 ms). The default value corresponds to 25 ms. */
#define APP_ADV_SLOW_INTERVAL          3200                                         /**< Slow advertising interval (in units of 0.625 ms). The default value corresponds to 2 seconds. */
#define APP_ADV_FAST_TIMEOUT           180                                           /**< The advertising time-out in units of seconds. */
#define APP_ADV_SLOW_TIMEOUT           180                                          /**< The advertising time-out in units of seconds. */
#define ADV_INTERVAL_FAST_PERIOD       30                                           /**< The duration of the fast advertising period (in seconds). */

#define MIN_CONN_INTERVAL              MSEC_TO_UNITS(50, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL              MSEC_TO_UNITS(100, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                  0                                            /**< Slave latency. */
#define CONN_SUP_TIMEOUT               MSEC_TO_UNITS(4000, UNIT_10_MS)              /**< Connection supervisory time-out (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)                        /**< Time from initiating an event (connect or start of notification) to the first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(30000)                       /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT   3                                            /**< Number of attempts before giving up the connection parameter negotiation. */

#define MESSAGE_BUFFER_SIZE            18                                           /**< Size of buffer holding optional messages in notifications. */

#define SECURITY_REQUEST_DELAY         APP_TIMER_TICKS(1500)                        /**< Delay after connection until security request is sent, if necessary (ticks). */

#define SEC_PARAM_BOND                 1                                            /**< Perform bonding. */
#define SEC_PARAM_MITM                 0                                            /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                 0                                            /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS             0                                            /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES      BLE_GAP_IO_CAPS_NONE                         /**< No I/O capabilities. */
#define SEC_PARAM_OOB                  0                                            /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE         7                                            /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE         16                                           /**< Maximum encryption key size. */

#define DEAD_BEEF                      0xDEADBEEF                                   /**< Value used as error code on stack dump. Can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE      APP_TIMER_SCHED_EVENT_DATA_SIZE              /**< Maximum size of scheduler events. */
#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE               20                                           /**< Maximum number of events in the scheduler queue. More is needed in case of Serialization. */
#else
#define SCHED_QUEUE_SIZE               10                                           /**< Maximum number of events in the scheduler queue. */
#endif
//+
#include "sd_ble_ancs_nus.h"
#include "ble_nus.h"
#include "flash_file_address.h"

#define STRING_LENGTH_NUS                  32
uint8_t RECEIVED_STRING_NUS[STRING_LENGTH_NUS + 1];
volatile uint8_t ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER] = {0, 0, 0, 0, 0};
static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
BLE_NUS_DEF(m_nus);                                                                 /**< BLE NUS service instance. */
#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */
/* Name of the device. Will be included in the advertising data. */
char BLE_BROADCAST_DEVICE_NAME[16] = {15, 'w', 'i', 'r', 'e', 'l', 'e', 's', 's', 'S', 'I', 'M', '0', '0', '0', '0'};                                      

extern volatile uint8_t           FLASH_UPDATE_WAITING_STAGE;
//+


APP_TIMER_DEF(m_sec_req_timer_id);                                                  /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
BLE_ANCS_C_DEF(m_ancs_c);                                                           /**< Apple Notification Service Client instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                                    /**< DB Discovery module instance. */

static pm_peer_id_t m_whitelist_peers[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];            /**< List of peers currently in the whitelist. */
static uint32_t     m_whitelist_peer_cnt;                                           /**< Number of peers currently in the whitelist. */
static bool         m_is_wl_changed;                                                /**< Indicates if the whitelist has been changed since last time it has been updated in the Peer Manager. */
static pm_peer_id_t m_peer_id;                                                      /**< Device reference handle to the current bonded central. */
static uint16_t     m_cur_conn_handle = BLE_CONN_HANDLE_INVALID;                    /**< Handle of the current connection. */

static ble_ancs_c_evt_notif_t m_notification_latest;                                /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_attr_t      m_notif_attr_latest;                                  /**< Local copy of the newest notification attribute. */
static ble_ancs_c_attr_t      m_notif_attr_app_id_latest;                           /**< Local copy of the newest app attribute. */

static uint8_t m_attr_appid[ATTR_DATA_SIZE];                                        /**< Buffer to store attribute data. */
static uint8_t m_attr_title[ATTR_DATA_SIZE];                                        /**< Buffer to store attribute data. */
static uint8_t m_attr_subtitle[ATTR_DATA_SIZE];                                     /**< Buffer to store attribute data. */
static uint8_t m_attr_message[ATTR_DATA_SIZE];                                      /**< Buffer to store attribute data. */
static uint8_t m_attr_message_size[ATTR_DATA_SIZE];                                 /**< Buffer to store attribute data. */
static uint8_t m_attr_date[ATTR_DATA_SIZE];                                         /**< Buffer to store attribute data. */
static uint8_t m_attr_posaction[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */
static uint8_t m_attr_negaction[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */
static uint8_t m_attr_disp_name[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */

#if (IF_LOG_OUTPUT)
/**@brief String literals for the iOS notification categories. used then printing to UART. */
static char const * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
{
    "Other",
    "Incoming Call",
    "Missed Call",
    "Voice Mail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "Health And Fitness",
    "Business And Finance",
    "Location",
    "Entertainment"
};

/**@brief String literals for the iOS notification event types. Used then printing to UART. */
static char const * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
{
    "Added",
    "Modified",
    "Removed"
};

/**@brief String literals for the iOS notification attribute types. Used when printing to UART. */
static char const * lit_attrid[BLE_ANCS_NB_OF_NOTIF_ATTR] =
{
    "App Identifier",
    "Title",
    "Subtitle",
    "Message",
    "Message Size",
    "Date",
    "Positive Action Label",
    "Negative Action Label"
};

/**@brief String literals for the iOS notification attribute types. Used When printing to UART. */
static char const * lit_appid[BLE_ANCS_NB_OF_APP_ATTR] =
{
    "Display Name"
};
#endif

static void delete_bonds(void);

//+
/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
void nus_data_handler(ble_nus_evt_t * p_evt)
{
	uint16_t i;
	
  if (p_evt->type == BLE_NUS_EVT_RX_DATA)
  {
		
	*(RECEIVED_STRING_NUS) = (p_evt->params.rx_data.length < STRING_LENGTH_NUS ? p_evt->params.rx_data.length : STRING_LENGTH_NUS);
	
  for (i=0; i<(*RECEIVED_STRING_NUS); i++)
	{
		*(RECEIVED_STRING_NUS + i + 1) = *(p_evt->params.rx_data.p_data + i);
	}
		
#if (IF_LOG_OUTPUT)
	printf("\r\n========================================= BLE NUS received =========================================\r\n");
	printf("length = %d,  ", p_evt->params.rx_data.length);
  for (i=0; i<(*RECEIVED_STRING_NUS); i++)
	{
		printf("%c", *(RECEIVED_STRING_NUS + i + 1));
	}
	printf("\r\n====================================================================================================\r\n");
#endif	
  }
}

/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
				
#if (IF_LOG_OUTPUT)
        printf("Data len is set to 0x%X(%d)\r\n", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
#endif
    }
#if (IF_LOG_OUTPUT)
		printf("\r\n=====================================================================================\r\n");					
		printf("ATT MTU exchange completed. central 0x%x peripheral 0x%x\r\n", p_gatt->att_mtu_desired_central, p_gatt->att_mtu_desired_periph);
		printf("=====================================================================================\r\n");					
#endif
}

/**@brief Function for LED flashing to indicate an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
static void notif_led_flash(ble_ancs_c_evt_notif_t * p_notif)
{
	if (p_notif->evt_id == 0) /* Added */
	{
	  if (p_notif->category_id == 1) /* incoming call */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_INCOMING_CALL])++;
	  }
	  if (p_notif->category_id == 2) /* missed call */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_MISSED_CALL])++;
	  }
	  else if (p_notif->category_id == 4) /* social */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_SOCIAL])++;
	  }
	  else if (p_notif->category_id == 6) /* email */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_EMAIL])++;
	  }
		else /* others */
		{
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_DEFAULT])++;
		}
	}
	else if (p_notif->evt_id == 1) /* Modified */
	{
		
	}
	else if (p_notif->evt_id == 2) /* Removed */
	{
	  if (p_notif->category_id == 1) /* incoming call */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_INCOMING_CALL])--;
	  }
	  if (p_notif->category_id == 2) /* missed call */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_MISSED_CALL])--;
	  }
	  else if (p_notif->category_id == 4) /* social */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_SOCIAL])--;
	  }
	  else if (p_notif->category_id == 6) /* email */
	  {
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_EMAIL])--;
	  }
		else /* others */
		{
		  (ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_DEFAULT])--;
		}
	}
	
	return;
}
//+

/**@brief Callback function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product.
 *          You must analyze how your product should react to asserts.
 * @warning On assert from the SoftDevice, the system can recover only on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Fetch the list of peer manager peer IDs.
 *
 * @param[inout] p_peers   The buffer where to store the list of peer IDs.
 * @param[inout] p_size    In: The size of the @p p_peers buffer.
 *                         Out: The number of peers copied in the buffer.
 */
static void peer_list_get(pm_peer_id_t * p_peers, uint32_t * p_size)
{
    pm_peer_id_t peer_id;
    uint32_t     peers_to_copy;

    peers_to_copy = (*p_size < BLE_GAP_WHITELIST_ADDR_MAX_COUNT) ?
                     *p_size : BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

    peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    *p_size = 0;

    while ((peer_id != PM_PEER_ID_INVALID) && (peers_to_copy--))
    {
        p_peers[(*p_size)++] = peer_id;
        peer_id = pm_next_peer_id_get(peer_id);
    }
}

/**@brief Function for starting advertising. */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    }
    else
    {
        ret_code_t ret;

        memset(m_whitelist_peers, PM_PEER_ID_INVALID, sizeof(m_whitelist_peers));
        m_whitelist_peer_cnt = (sizeof(m_whitelist_peers) / sizeof(pm_peer_id_t));

        peer_list_get(m_whitelist_peers, &m_whitelist_peer_cnt);

        ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
        APP_ERROR_CHECK(ret);

        // Setup the device identies list.
        // Some SoftDevices do not support this feature.
        ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
        if (ret != NRF_ERROR_NOT_SUPPORTED)
        {
            APP_ERROR_CHECK(ret);
        }

        m_is_wl_changed = false;

        ret = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(ret);
    }
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t ret;
		uint32_t i;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Connected to previously bonded device\r\n");
            printf("=====================================================================================\r\n");					
#endif
            m_peer_id = p_evt->peer_id;
//+						
						for (i=0; i<ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER; i++)
						{
							ANCS_NOTIF_LED_PATTERN[i] = 0;
						}
//+						
        } break; // PM_EVT_BONDED_PEER_CONNECTED

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.\r\n",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
						printf("=====================================================================================\r\n");					
#endif
            m_peer_id = p_evt->peer_id;

            // Discover peer's services.
            ret  = ble_db_discovery_start(&m_db_disc, p_evt->conn_handle);
            APP_ERROR_CHECK(ret);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_CONN_SEC_FAILED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_CONN_SEC_CONFIG_REQ......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            ret = fds_gc();
            if (ret == FDS_ERR_BUSY || ret == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(ret);
            }
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_STORAGE_FULL......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
          advertising_start(false);
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_PEERS_DELETE_SUCCEEDED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
          // The local database has likely changed, send service changed indications.
          pm_local_database_has_changed();
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        {
            // Note: You should check on what kind of white list policy your application should use.
            if (     p_evt->params.peer_data_update_succeeded.flash_changed
                 && (p_evt->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_BONDING))
            {
#if (IF_LOG_OUTPUT)
                printf("\r\n=====================================================================================\r\n");					
                printf("\tm_whitelist_peer_cnt %ld, MAX_PEERS_WLIST %d",
                               m_whitelist_peer_cnt + 1,
                               BLE_GAP_WHITELIST_ADDR_MAX_COUNT);
							  printf("=====================================================================================\r\n");					
#endif

                if (m_whitelist_peer_cnt < BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
                {
                    // Bonded to a new peer, add it to the whitelist.
                    m_whitelist_peers[m_whitelist_peer_cnt++] = m_peer_id;

                    // The whitelist has been modified, update it in the Peer Manager.
                    ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
                    if (ret != NRF_ERROR_NOT_SUPPORTED)
                    {
                        APP_ERROR_CHECK(ret);
                    }

                    ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
                    APP_ERROR_CHECK(ret);
                }
            }
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
          // Assert.
          APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_PEER_DATA_UPDATE_FAILED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
          // Assert.
          APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_PEER_DELETE_FAILED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
          // Assert.
          APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_PEERS_DELETE_FAILED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
          // Assert.
          APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
#if (IF_LOG_OUTPUT)
          printf("\r\n=====================================================================================\r\n");					
          printf("-----------PM_EVT_ERROR_UNEXPECTED......\r\n");
					printf("=====================================================================================\r\n");					
#endif
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
static void sec_req_timeout_handler(void * p_context)
{
    ret_code_t           ret;
    pm_conn_sec_status_t status;

    if (m_cur_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        ret = pm_conn_sec_status_get(m_cur_conn_handle, &status);
        APP_ERROR_CHECK(ret);

        // If the link is still not secured by the peer, initiate security procedure.
        if (!status.encrypted)
        {
            ret = pm_conn_secure(m_cur_conn_handle, false);
            if (ret != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(ret);
            }
        }
    }
}

/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void)
{
    ret_code_t ret;

    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    ret = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

#if (IF_LOG_OUTPUT)
    printf("\r\n=====================================================================================\r\n");					
    printf("Notifications Enabled.\r\n");
    printf("=====================================================================================\r\n");					
#endif
}

/**@brief Function for printing an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{
#if (IF_LOG_OUTPUT)
	  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
    printf("\r\n=====================================================================================\r\n");					
    printf("\r\nNotification\r\n");
    printf("Event:       %s\r\n", (char *)lit_eventid[p_notif->evt_id]);
    printf("Category ID: %s\r\n", (char *)lit_catid[p_notif->category_id]);
    printf("Category Cnt:%u\r\n", (unsigned int) p_notif->category_count);
    printf("UID:         %u\r\n", (unsigned int) p_notif->notif_uid);

    printf("Flags:\r\n");
#endif
    if (p_notif->evt_flags.silent == 1)
    {
#if (IF_LOG_OUTPUT)
        printf(" Silent\r\n");
#endif
    }
    if (p_notif->evt_flags.important == 1)
    {
#if (IF_LOG_OUTPUT)
        printf(" Important\r\n");
#endif
    }
    if (p_notif->evt_flags.pre_existing == 1)
    {
#if (IF_LOG_OUTPUT)
        printf(" Pre-existing\r\n");
#endif
    }
    if (p_notif->evt_flags.positive_action == 1)
    {
#if (IF_LOG_OUTPUT)
        printf(" Positive Action\r\n");
#endif
    }
    if (p_notif->evt_flags.negative_action == 1)
    {
#if (IF_LOG_OUTPUT)
        printf(" Negative Action\r\n");
#endif
    }
		
#if (IF_LOG_OUTPUT)
    printf("=====================================================================================\r\n");					
#endif
}

/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS notification attribute.
 */
static void notif_attr_print(ble_ancs_c_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
#if (IF_LOG_OUTPUT && NRF_LOG_ENABLED)
        printf("\r\n=====================================================================================\r\n");	
        printf("%s: %s\r\n", (char *)lit_attrid[p_attr->attr_id], (char *)nrf_log_push((char *)p_attr->p_attr_data));
        printf("=====================================================================================\r\n");					
#endif
    }
    else if (p_attr->attr_len == 0)
    {
#if (IF_LOG_OUTPUT)
        printf("\r\n=====================================================================================\r\n");					
        printf("%s: (N/A)\r\n", (char *)lit_attrid[p_attr->attr_id]);
        printf("=====================================================================================\r\n");					
#endif
    }
}

/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS App attribute.
 */
static void app_attr_print(ble_ancs_c_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
#if (IF_LOG_OUTPUT)
        printf("\r\n=====================================================================================\r\n");					
        printf("%s: %s\r\n", (char *)lit_appid[p_attr->attr_id], (char *)p_attr->p_attr_data);
        printf("=====================================================================================\r\n");					
#endif
    }
    else if (p_attr->attr_len == 0)
    {
#if (IF_LOG_OUTPUT)
        printf("\r\n=====================================================================================\r\n");					
        printf("%s: (N/A)\r\n", (char *) lit_appid[p_attr->attr_id]);
        printf("=====================================================================================\r\n");					
#endif
    }
}

/**@brief Function for printing out errors that originated from the Notification Provider (iOS).
 *
 * @param[in] err_code_np Error code received from NP.
 */
static void err_code_print(uint16_t err_code_np)
{
    switch (err_code_np)
    {
        case BLE_ANCS_NP_UNKNOWN_COMMAND:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Error: Command ID was not recognized by the Notification Provider. \r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ANCS_NP_INVALID_COMMAND:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Error: Command failed to be parsed on the Notification Provider. \r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ANCS_NP_INVALID_PARAMETER:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Error: Parameter does not refer to an existing object on the Notification Provider. \r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ANCS_NP_ACTION_FAILED:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Error: Perform Notification Action Failed on the Notification Provider. \r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        default:
            break;
    }
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t ret;

    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    // Create security request timer.
    ret = app_timer_create(&m_sec_req_timer_id,
                           APP_TIMER_MODE_SINGLE_SHOT,
                           sec_req_timeout_handler);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    ret_code_t ret = NRF_SUCCESS;

    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Apple Notification Center Service discovered on the server.\r\n");
            printf("=====================================================================================\r\n");					
#endif
            ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
            APP_ERROR_CHECK(ret);
            apple_notification_setup();
            break;

        case BLE_ANCS_C_EVT_NOTIF:
            m_notification_latest = p_evt->notif;
            notif_print(&m_notification_latest);
//+
						notif_led_flash(&m_notification_latest);
//+												
            break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
            m_notif_attr_latest = p_evt->attr;
            notif_attr_print(&m_notif_attr_latest);
            if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
            {
                m_notif_attr_app_id_latest = p_evt->attr;
            }
            break;
						
        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Apple Notification Center Service not discovered on the server.\r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
            app_attr_print(&p_evt->attr);
            break;
						
        case BLE_ANCS_C_EVT_NP_ERROR:
            err_code_print(p_evt->err_code_np);
            break;
						
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing GAP connection parameters.
 *
 * @details Use this function to set up all necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    ret_code_t              ret;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    ret = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)(BLE_BROADCAST_DEVICE_NAME + 1), BLE_BROADCAST_DEVICE_NAME[0]);
    APP_ERROR_CHECK(ret);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    ret = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
	 ret_code_t ret;
	
    // ret = nrf_ble_gatt_init(&m_gatt, NULL);
    // APP_ERROR_CHECK(ret);
//+   	
	ret = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
	APP_ERROR_CHECK(ret);

	ret = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
	APP_ERROR_CHECK(ret);
//+   

}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             ret;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    // cp_init.disconnect_on_fail             = true;
    // cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;
//+		
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
//+

    ret = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           ret;

    ret = pm_init();
    APP_ERROR_CHECK(ret);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    ret = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(ret);

    ret = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(ret);
}

/**
 * @brief Delete all data stored for all peers
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

#if (IF_LOG_OUTPUT)
    printf("Erase bonds!\r\n");
#endif

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function is called for advertising events that are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t ret;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Fast advertising\r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ADV_EVT_SLOW:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Slow advertising\r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ADV_EVT_FAST_WHITELIST:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Fast advertising with Whitelist\r\n");
            printf("=====================================================================================\r\n");					
#endif
            break;

        case BLE_ADV_EVT_IDLE:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("BLE Adv. Idle........, re-start advertising \r\n");
            printf("=====================================================================================\r\n");					
#endif
						advertising_start(false);
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST:
        {
            ble_gap_addr_t whitelist_addrs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t  whitelist_irks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            uint32_t       addr_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            uint32_t       irk_cnt  = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

            ret = pm_whitelist_get(whitelist_addrs, &addr_cnt, whitelist_irks, &irk_cnt);
            APP_ERROR_CHECK(ret);
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("pm_whitelist_get returns %ld addr in whitelist and %ld irk whitelist\r\n",
                           addr_cnt,
                           irk_cnt);
			      printf("=====================================================================================\r\n");					
#endif

            // Apply the whitelist.
            ret = ble_advertising_whitelist_reply(&m_advertising,
                                                  whitelist_addrs,
                                                  addr_cnt,
                                                  whitelist_irks,
                                                  irk_cnt);
            APP_ERROR_CHECK(ret);
        }
        break;

        default:
            break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t ret = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Connected.\r\n");
            printf("=====================================================================================\r\n");					
#endif

            m_cur_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            ret               = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
            APP_ERROR_CHECK(ret);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("Disconnected, re-start advertising\r\n");
            printf("=====================================================================================\r\n");					
#endif
            m_cur_conn_handle = BLE_CONN_HANDLE_INVALID;
            ret               = app_timer_stop(m_sec_req_timer_id);
            APP_ERROR_CHECK(ret);

            if (p_ble_evt->evt.gap_evt.conn_handle == m_ancs_c.conn_handle)
            {
                m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
            }
            if (m_is_wl_changed)
            {
                // The whitelist has been modified, update it in the Peer Manager.
                ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
                APP_ERROR_CHECK(ret);

                ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
                if (ret != NRF_ERROR_NOT_SUPPORTED)
                {
                    APP_ERROR_CHECK(ret);
                }

                m_is_wl_changed = false;
            }
            break; // BLE_GAP_EVT_DISCONNECTED

#ifndef S140
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("PHY update request.\r\n");
            printf("=====================================================================================\r\n");					
#endif
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            ret = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(ret);
        } break;
#endif

        case BLE_GATTC_EVT_TIMEOUT:
				{
            // Disconnect on GATT Client timeout event.
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("GATT Client Timeout.\r\n");
            printf("=====================================================================================\r\n");					
#endif
            ret = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                        BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(ret);
            break;
				}

        case BLE_GATTS_EVT_TIMEOUT:
				{
            // Disconnect on GATT Server timeout event.
#if (IF_LOG_OUTPUT)
            printf("\r\n=====================================================================================\r\n");					
            printf("GATT Server Timeout.\r\n");
            printf("=====================================================================================\r\n");					
#endif
            ret = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                        BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(ret);
            break;
				}

//+
				case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
				{
#if (IF_LOG_OUTPUT)
            printf("------------- BLE_GAP_EVT_SEC_PARAMS_REQUEST ......\r\n");
#endif
						// // Pairing not supported
						// ret = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
						// APP_ERROR_CHECK(ret);
						break;
				}

#if !defined (S112)
		    case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
		    {
		        ble_gap_data_length_params_t dl_params;

		        // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
		        memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
		        ret = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
		        APP_ERROR_CHECK(ret);
		        break;
		    }
#endif //!defined (S112)

				case BLE_GATTS_EVT_SYS_ATTR_MISSING:
				{
						// No system attributes have been stored.
						ret = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
						APP_ERROR_CHECK(ret);
						break;
				}

		    case BLE_EVT_USER_MEM_REQUEST:
				{
		        ret = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
		        APP_ERROR_CHECK(ret);
		        break;
			  }

		    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
		    {
		        ble_gatts_evt_rw_authorize_request_t  req;
		        ble_gatts_rw_authorize_reply_params_t auth_reply;

		        req = p_ble_evt->evt.gatts_evt.params.authorize_request;

		        if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
		        {
		            if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
		                (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
		                (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
		            {
		                if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
		                {
		                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
		                }
		                else
		                {
		                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
		                }
		                auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
		                ret = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
		                                                               &auth_reply);
		                APP_ERROR_CHECK(ret);
		            }
		        }
#if (IF_LOG_OUTPUT)
            printf("------------- BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST ......\r\n");
#endif
		        break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
				}

				case BLE_GATTS_EVT_HVN_TX_COMPLETE:
				{
#if (IF_LOG_OUTPUT)
          printf("================================ Handle Value Notification transmission complete ================================\r\n");
#endif

					break;
				}
//+
	        default:
            // No implementation needed.
            break;
    }
    APP_ERROR_CHECK(ret);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for initializing the Apple Notification Center Service.
 */
static void services_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ret_code_t        ret;

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
                                  m_attr_appid,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
                                      BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
                                      m_attr_disp_name,
                                      sizeof(m_attr_disp_name));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                  m_attr_title,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
                                  m_attr_message,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
                                  m_attr_subtitle,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
                                  m_attr_message_size,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_DATE,
                                  m_attr_date,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
                                  m_attr_posaction,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
                                  m_attr_negaction,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    ret = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(ret);
		
//+
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    ret = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(ret);
//+		
}

/**@brief Function for initializing the advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             ret;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    static ble_uuid_t m_adv_uuids[1]; /**< Universally unique service identifiers. */

    m_adv_uuids[0].uuid = ANCS_UUID_SERVICE;
    m_adv_uuids[0].type = m_ancs_c.service.service.uuid.type;

    init.advdata.name_type                = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance       = true;
    init.advdata.flags                    = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt  = 0;
    init.advdata.uuids_complete.p_uuids   = NULL;
    init.advdata.uuids_solicited.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_solicited.p_uuids  = m_adv_uuids;

    init.config.ble_adv_whitelist_enabled = true;
    init.config.ble_adv_fast_enabled      = true;
    init.config.ble_adv_fast_interval     = APP_ADV_FAST_INTERVAL;
    init.config.ble_adv_fast_timeout      = APP_ADV_FAST_TIMEOUT;
    init.config.ble_adv_slow_enabled      = true;
    init.config.ble_adv_slow_interval     = APP_ADV_SLOW_INTERVAL;
    init.config.ble_adv_slow_timeout      = APP_ADV_SLOW_TIMEOUT;

    init.evt_handler = on_adv_evt;

    ret = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(ret);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing the Event Scheduler.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for initializing the database discovery module.
 */
static void db_discovery_init(void)
{
    ret_code_t ret = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for the Power manager.
 */
void power_manage(void)
{
    ret_code_t ret = sd_app_evt_wait();
    APP_ERROR_CHECK(ret);
}

/**@brief Function for register handler of events.
 *
 * @details registe handler of events.
 */
static void sdh_event_handler_register(void)
{
  /* Register a handler for SoC events. */
	/* flash erase/write handler */
	NRF_SDH_SOC_OBSERVER(m_sys_observer_flash_erase_write, 0, sys_event_flash_erase_write_handler, NULL);
	/* time slot signal handler */
	NRF_SDH_SOC_OBSERVER(m_sys_observer_time_slot, 0, nrf_evt_signal_handler, NULL);
}

/********************************************************************************/
void ble_device_name_init(void)
/*--------------------------------------------------------------------------------
| BLE broadcast deviece name
|
--------------------------------------------------------------------------------*/
{
	uint32_t i;
	uint32_t device_id0 = NRF_FICR->DEVICEID[0];
	
	if (*(uint8_t *)SMART_USIM_DEVICE_NAME_FLASH_ADDR < 16)
	{
		for (i=0; i<(*(uint8_t *)SMART_USIM_DEVICE_NAME_FLASH_ADDR) + 1; i++)
		{
			BLE_BROADCAST_DEVICE_NAME[i] = *((uint8_t *)SMART_USIM_DEVICE_NAME_FLASH_ADDR + i);
		}
	}
	else
	{
    sprintf((char *)BLE_BROADCAST_DEVICE_NAME + 12, "%x", (uint8_t)(device_id0 >> 8));
    sprintf((char *)BLE_BROADCAST_DEVICE_NAME + 14, "%x", (uint8_t)device_id0);
	}
}

/********************************************************************************/
void nus_send_bytes(uint8_t *bytes_string, uint16_t bytes_length)
/*--------------------------------------------------------------------------------
| send bytes string by NUS service
|
--------------------------------------------------------------------------------*/
{
	uint16_t       length;
  uint32_t       err_code;
	
  length = bytes_length;	
  do
  {
    err_code = ble_nus_string_send(&m_nus, bytes_string, &length);
    if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
    {
       APP_ERROR_CHECK(err_code);
    }
  } while (err_code == NRF_ERROR_BUSY);
}

/********************************************************************************/
void init_ble_ancs_c(void)
{
	ble_device_name_init( );
  timers_init();
  scheduler_init();
}

/********************************************************************************/
void start_ble_ancs_c(void)
{
  bool erase_bonds = 0;
  static bool if_initial_start_ble = 0;			
		
	if ((*(uint8_t *)BLE_BOND_DEL_STATUS_FLASH_ADDR) & 1)	
	{
		erase_bonds = 1;
		
	  BLE_BOND_DEL_STATUS_RAM = 0;
		
		start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
	}
		
  /* BLE softdevice stack initialization */
	init_ble_ancs_c( );
	
  ble_stack_init();
	/* start BLE stack and service */
  sdh_event_handler_register();
  gap_params_init();
  gatt_init();
  db_discovery_init();
  services_init();
  advertising_init();
  conn_params_init();

	if (if_initial_start_ble == 0)
	{
    peer_manager_init();
		if_initial_start_ble = 1;
	}

  // Start execution.
#if (IF_LOG_OUTPUT)
  printf("Apple Notification Center Service client example started. \r\n");
#endif

  advertising_start(erase_bonds);
}



/* --------------------------------------------------------------------------
Copyright (c) 2018, Jiacheng Wang
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------- */
#ifndef DEFINES_ALL_H__
#define DEFINES_ALL_H__

/* ISR type for function SWI3_EGU3_IRQHandler() */
#define SWI3_EGU3_ISR_PHONE_COMMAND                                   1
#define SWI3_EGU3_ISR_USIM_SERVER_SIM_COMMAND_AUTHENTICATION_stage3   2
#define SWI3_EGU3_ISR_USIM_SERVER_WIRELESS_AUTHENTICATION_stage1      3
#define SWI3_EGU3_ISR_SIM_CARD_STATE_RESET                            4
#define SWI3_EGU3_ISR_STOP_BLE_STACK                                  5

/* phone command fetch (0x12) type */
#define FETCH_COMMAND_TYPE_NULL_VALUE                                         0
#define FETCH_COMMAND_TYPE_DISPLAY_TEXT_ABOUT                                 1
#define FETCH_COMMAND_TYPE_SMART_USIM_TRAY_DEVICE_NAME_TEXT_DISPLAY           2
#define FETCH_COMMAND_TYPE_LED_STATUS_SYNCH_SERVER_TEXT_DISPLAY               3
#define FETCH_COMMAND_TYPE_BLE_STATUS_ON_DISPLAY_TEXT_DISPLAY                 4
#define FETCH_COMMAND_TYPE_BLE_STATUS_OFF_DISPLAY_TEXT_DISPLAY                5
#define FETCH_COMMAND_TYPE_CONFIG_RESTORE_TEXT_DISPLAY                        6
#define FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT_DISPLAY        7
#define FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT_DISPLAY       8
#define FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_ON_SERVER             9
#define FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_OFF_SERVER            10
#define FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_ON_SERVER             11
#define FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_OFF_SERVER            12
#define FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_CONNECTED_CLIENT            13
#define FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_CONNECTED_CLIENT            14
#define FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_DISCONNECTED_CLIENT         15
#define FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_DISCONNECTED_CLIENT         16
#define FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_OFF_ADDRESS_CLIENT                   17
#define FETCH_COMMAND_TYPE_BLE_BOND_DELETE_TEXT_DISPLAY                       18
#define FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY                           19
#define FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY_CLIENT                    20
#define FETCH_COMMAND_TYPE_PASSWORD_INPUT_TEXT_DISPLAY                        21
#define FETCH_COMMAND_TYPE_SYSTEM_KEY_INPUT_TEXT_DISPLAY                      22
#define FETCH_COMMAND_TYPE_IDLE_TEXT_CONFIG_SYCH_SERVER                        23
#define FETCH_COMMAND_TYPE_USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT              24
#define FETCH_COMMAND_TYPE_LED_PATTERN_UPDATED_TEXT_DISPLAY                   25
#define FETCH_COMMAND_TYPE_COMMAND_LINE_INPUT                                  26
#define FETCH_COMMAND_TYPE_CONFIG_SETTING_KEY_WORD_TEXT_DISPLAY               27
#define FETCH_COMMAND_TYPE_CONFIG_SETTINGS_KEY_WORD_LIST_TEXT_DISPLAY         28
#define FETCH_COMMAND_TYPE_LED_PATTERN_ERROR_TEXT_DISPLAY                     29
#define FETCH_COMMAND_TYPE_PASSWORD_ERROR_TEXT_DISPLAY                        30
#define FETCH_COMMAND_TYPE_SYSTEM_KEY_ERROR_TEXT_DISPLAY                      31
#define FETCH_COMMAND_TYPE_SMART_USIM_TRAY_DEVICE_NAME_ERROR_TEXT_DISPLAY     32
#define FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_TEXT_DISPLAY                       33
#define FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_TEXT_DISPLAY                       34
#define FETCH_COMMAND_TYPE_USIM_EF_UPDATE_ERROR_TEXT_DISPLAY                  35
#define FETCH_COMMAND_TYPE_WIRELESS_SIM_RESET_FETCH                           36
#define FETCH_COMMAND_TYPE_INITIAL_USAT_MENU_SETUP                            37
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT        38
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT        39
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT         40
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT         41
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_DISPLAY_TEXT                 42
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_DISPLAY_TEXT                 43
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_NO_DISPLAY_TEXT                  44
#define FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_NO_DISPLAY_TEXT                  45
#define FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY_CLIENT            46
#define FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY                   47
#define FETCH_COMMAND_TYPE_CONFIG_RESTORE_SUCCESSFULLY_TEXT_DISPLAY           48
#define FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_SUCCESSFULLY_TEXT_DISPLAY          49
#define FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_SUCCESSFULLY_TEXT_DISPLAY          50
#define FETCH_COMMAND_TYPE_BLE_BOND_DELETE_SUCCESSFULLY_TEXT_DISPLAY          51
#define FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_SUCCESSFULLY_TEXT_DISPLAY          52
#define FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_ERROR_TEXT_DISPLAY                 53
#define FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_ERROR_TEXT_DISPLAY            54
#define FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_SUCCESSFULLY_TEXT_DISPLAY     55
#define FETCH_COMMAND_TYPE_WIRELESS_USIM_WORK_MODE_ERROR_TEXT_DISPLAY         56
#define FETCH_COMMAND_TYPE_WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_TEXT_DISPLAY  57
#define FETCH_COMMAND_TYPE_DEFAULT_USED_USIM_ERROR_TEXT_DISPLAY               58
#define FETCH_COMMAND_TYPE_BLE_OFF_0X88_COMMAND_TEXT_DISPLAY                  59
#define FETCH_COMMAND_TYPE_BLE_ON_0X88_COMMAND_TEXT_DISPLAY                   60
#define FETCH_COMMAND_TYPE_BLE_ON_OFF_TEXT_DISPLAY                            61
#define FETCH_COMMAND_TYPE_WIRELESS_SIM_ON_OFF_TEXT_DISPLAY                   62
#define FETCH_COMMAND_TYPE_BLE_ON_OFF_0X88_TEXT_DISPLAY                       63
#define FETCH_COMMAND_TYPE_UPDATE_ICCID                                        64
#define FETCH_COMMAND_TYPE_UPDATE_ICCID_TEXT_DISPLAY                          65

/* idle text to indicate the wireless USIM status */
#define IDLE_MODE_TEXT_TYPE_WIRELESS_USIM0_SIM_ON_SERVER                      1
#define IDLE_MODE_TEXT_TYPE_WIRELESS_USIM1_SIM_ON_SERVER                      2
#define IDLE_MODE_TEXT_TYPE_WIRELESS_USIM0_SIM_OFF_SERVER                     3
#define IDLE_MODE_TEXT_TYPE_WIRELESS_USIM1_SIM_OFF_SERVER                     4
#define IDLE_MODE_TEXT_TYPE_SIM_ON_USIM0_CONNECTED_CLIENT                     5
#define IDLE_MODE_TEXT_TYPE_SIM_ON_USIM1_CONNECTED_CLIENT                     6
#define IDLE_MODE_TEXT_TYPE_SIM_ON_USIM0_DISCONNECTED_CLIENT                  7
#define IDLE_MODE_TEXT_TYPE_SIM_ON_USIM1_DISCONNECTED_CLIENT                  8
#define IDLE_MODE_TEXT_TYPE_SIM_OFF_ADDRESS_CLIENT                            9
#define IDLE_MODE_TEXT_TYPE_CONFIG_SYCH_SERVER                               10

/* USAT menu items */
#define USAT_MENU_ABOUT                                         1
#define USAT_MENU_WIRELESS_USIM_ON_OFF                          2
#define USAT_MENU_USIM_SWITCH                                   3
#define USAT_MENU_BLE_ON_OFF                                    4
#define USAT_MENU_CONFIG_SETTINGS                               5
#define USAT_MENU_UPDATE_ICCID                                  6
#define USAT_MENU_DEVICE_RESET                                  7

/* task queue position, require run task with BLE stopped */
#define BLE_STOPPED_TASK_CONFIG_RESTORE_POS                     1
#define BLE_STOPPED_TASK_USIM0_EF_UPDATE_POS                    2
#define BLE_STOPPED_TASK_USIM1_EF_UPDATE_POS                    3

#define MASK_SELECTED_FILE                              0xFFFFFFFFFFFF0000

/* USIM card EF data update task queue mark offset position in SIM_FILE_UPDATE_BINARY_TASK_QUEUE */
#define SIM_FILE_UPDATE_BINARY_6F08_OFFSET_POS                               0
#define SIM_FILE_UPDATE_BINARY_6F7E_OFFSET_POS                               1
#define SIM_FILE_UPDATE_BINARY_6F09_OFFSET_POS                               2
#define SIM_FILE_UPDATE_BINARY_6F5B_OFFSET_POS                               3
#define SIM_FILE_UPDATE_BINARY_6F73_OFFSET_POS                               4
#define SIM_FILE_UPDATE_BINARY_6F7B_OFFSET_POS                               5
#define SIM_FILE_UPDATE_BINARY_4F20_OFFSET_POS                               6
#define SIM_FILE_UPDATE_BINARY_4F52_OFFSET_POS                               7
#define SIM_FILE_UPDATE_BINARY_6F31_OFFSET_POS                               8
#define SIM_FILE_UPDATE_BINARY_6F78_OFFSET_POS                               9
#define SIM_FILE_UPDATE_BINARY_6FAD_OFFSET_POS                               10
#define SIM_FILE_UPDATE_BINARY_6F5C_OFFSET_POS                               11
#define SIM_FILE_UPDATE_BINARY_6FC4_OFFSET_POS                               12
#define SIM_FILE_UPDATE_BINARY_6F56_OFFSET_POS                               13
#define SIM_FILE_UPDATE_BINARY_6F42_OFFSET_POS                               14

/* flash data write check task queue mark offset position in FLASH_DATA_WRITE_CHECK_TASK_QUEUE */
#define FLASH_DATA_WRITE_TASK_OFFSET_POS                                     31
#define FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_OFFSET_POS                         0
#define FLASH_DATA_WRITE_CHECK_USER_PASSWORD_OFFSET_POS                      1
#define FLASH_DATA_WRITE_CHECK_SYSTEM_KEY_OFFSET_POS                         2
#define FLASH_DATA_WRITE_CHECK_LED_PATTERN_OFFSET_POS                        3
#define FLASH_DATA_WRITE_CHECK_SMART_USIM_DEVICE_NAME_OFFSET_POS             4
#define FLASH_DATA_WRITE_CHECK_WIRELESS_SIM_ON_OFF_OFFSET_POS                5
#define FLASH_DATA_WRITE_CHECK_USIM0_DATA_OFFSET_POS                         6
#define FLASH_DATA_WRITE_CHECK_USIM1_DATA_OFFSET_POS                         7
#define FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS                  8
#define FLASH_DATA_WRITE_CHECK_BLE_BOND_DELETE_OFFSET_POS                    9
#define FLASH_DATA_WRITE_CHECK_LOGICAL_ADDRESS_OFFSET_POS                    10
#define FLASH_DATA_WRITE_CHECK_ICCID_2FE2_DATA_MODE_OFFSET_POS               11
#define FLASH_DATA_WRITE_CHECK_WIRELESS_USIM_WORK_MODE_OFFSET_POS            12
#define FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_0X88_COMMAND_OFFSET_POS            13
#define FLASH_DATA_WRITE_CHECK_ICCID_2FE2_FIXED_DATA_OFFSET_POS              14

/* USIM EF data update time lot stage, for binary update command */
#define SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage0                  0
#define SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage1                  1
#define SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage2                  2
#define SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage3                  3
#define SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage4                  4

#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage0         7
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage1         8
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage2         9
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage3         10
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage4         11
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage5         12
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage6         13
#define PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage7         14


/* time in ms to wait before next time slot request try, if time slot request fails */
#define TIME_SLOT_REQUEST_RETRY_INTERVAL_MS               5
/* time to predicted phone command 0xf2, in ms.
|  within this time margin, stop time slot request to avoid miss phone command */
#define TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS             100

/* CC encoding and decoding time in us */
#define TIMER_SLOT_CC_ENCODING_TIME_US                    500
#define TIMER_SLOT_CC_DECODING_TIME_US                    5500
/* time length in us for USIM server wireless connection signal listening windows tiem slot */
#if (IF_CONNECTION_CC_CODED)
  #define TIMER_SLOT_SIM_CONNECTION_LISTENING_US          20000
#else
  #define TIMER_SLOT_SIM_CONNECTION_LISTENING_US          5000
#endif
/* time length in us for USIM client wireless connection request time slot */
#if (IF_CONNECTION_CC_CODED)
  #define TIMER_SLOT_PHONE_CONNECTION_REQUEST_US          10000
#else
  #define TIMER_SLOT_PHONE_CONNECTION_REQUEST_US          2300
#endif
/* time length in us for phone command 0xf2 process window time slot */
#define TIMER_SLOT_PHONE_COMMAND_0xF2_US                  1500
/* time length in us for phone command 0xa4 process window time slot */
#define TIMER_SLOT_PHONE_COMMAND_0xA4_US                  50000
/* time length in us for phone command 0xd6 process window time slot */
#define TIMER_SLOT_PHONE_COMMAND_0xD6_US                  60000
/* time length in us for default phone command process window time slot */
#define TIMER_SLOT_PHONE_COMMAND_DEFAULT_US               50000
/* time length in us for phone command 0x88 process window time slot, local USIM */
#define TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US              60000
/* time length in us for phone command 0x88 process window time slot, wireless USIM */
#define TIMER_SLOT_PHONE_COMMAND_SIM_0x88_WIRELESS_US     60000
/* time length in us for phone command 0xc0 process window time slot */
#define TIMER_SLOT_PHONE_COMMAND_SIM_0xC0_US              60000
/* time length in us for wireless data transmission between USIM server and client */
#define TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US             3500
/* time length in us for radio Tx/Rx state transit */
#define TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US      150
/* time length in us for ending clear time slot */
#define TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US              100
/* time length in us for radio disable */
#define TIMER_SLOT_DADIO_DISABLE_MARGIN_US                50
/* time length in us for USIM binary file data update time slot */
#define TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US             100000

/* function time_slot_callback() time slot request type */
#define  TIME_SLOT_SIGNAL_DEFAULT_VALUE                                      0xff
#define TIME_SLOT_SIGNAL_PHONE_COMMAND                                      1
#define TIME_SLOT_SIGNAL_PHONE_CONNECTION_REQUEST_RADIO_LINK                2
#define TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION             3
#define TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_GET_DATA                   4
#define TIME_SLOT_SIGNAL_USIM_SERVER_LISTENING_RADIO_LINK                   5
#define TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE           6
#define TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_DATA_SEND                 7
#define TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION               8
#define TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND    9
#define TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND  10
#define TIME_SLOT_SIGNAL_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE                11
#define TIME_SLOT_SIGNAL_PHONE_COMMAND_WIRELESS_AUTHENTICATION              12
#define TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_ADF_SELECTION              13
#define TIME_SLOT_SIGNAL_USIM_CARD_BINARY_UPDATE                            14

#define TIME_SLOT_EVENT_DEFAULT_VALUE                                       0xff
#define TIME_SLOT_EVENT_PHONE_COMMAND                                       1
#define TIME_SLOT_EVENT_PHONE_CONNECTION_REQUEST_RADIO_LINK                 2
#define TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_AUTHENTICATION              3
#define TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_GET_DATA                    4
#define TIME_SLOT_EVENT_USIM_SERVER_LISTENING_RADIO_LINK                    5
#define TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE            6
#define TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_SIM_DATA_SEND                  7
#define TIME_SLOT_EVENT_USIM_SERVER_RANDOM_NUMBER_GENERATION                8
#define TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND     9
#define TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND   10
#define TIME_SLOT_EVENT_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE                 11
#define TIME_SLOT_EVENT_PHONE_COMMAND_WIRELESS_AUTHENTICATION               12
#define TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_ADF_SELECTION               13
#define TIME_SLOT_EVENT_USIM_CARD_BINARY_UPDATE                             14

#define  SIGNAL_CALLBACK_RETURN_UNDO                                         0xff

#define USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage0              0
#define USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage1              1
#define USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage2              2
#define USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage3              3
#define USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage4              4
#define USIM_SERVER_COMMAND_GET_DATA_BLE_stage0                   5
#define USIM_SERVER_COMMAND_GET_DATA_BLE_stage1                   6
#define USIM_SERVER_COMMAND_GET_DATA_BLE_stage2                   7

#define SIM_LISTENING_RETURN_DATA_TRANSMISSION                    0
#define SIM_LISTENING_RETURN_NOT_RECEIVED                         1
#define SIM_LISTENING_RETURN_PHONE_CONNECTION                     2
#define SIM_LISTENING_RETURN_16M_OSC_ERROR                        3
#define SIM_LISTENING_RETURN_SIM_OFF                              4
#define SIM_LISTENING_RETURN_LOGICAL_ADD_RX_ERROR                 5
#define SIM_LISTENING_RETURN_CONNECT_TYPE_RX_ERROR                6
#define SIM_LISTENING_RETURN_FREQ_CARRIER_RX_ERROR                7
#define SIM_LISTENING_RETURN_PASSWD_RX_ERROR                      8
#define SIM_LISTENING_RETURN_CONFIRM_TX_ERROR                     9
#define SIM_LISTENING_RETURN_MESSAGE_RECEIVED                     10

#define MAIN_WATCH_PHONE_SIM_WIRELESS_BLE_Stage0                  0
#define MAIN_WATCH_PHONE_SIM_WIRELESS_BLE_Stage1                  1
#define USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage0            2
#define USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage1            3
#define USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage2            4

/* command type for wireless USIM interface with connection request link */
#define PHONE_CONNECTION_PIGGYBACK_UPDATE_BINARY                  0x11
#define USIM_PHONE_CONNECTION_COMMAND_CONFIG_SYNCH                0x33
#define USIM_PHONE_CONNECTION_COMMAND_PHONE_WIRELESS_RESET        0x44
#define USIM_PHONE_CONNECTION_COMMAND_CONFIG_RESTORE              0x55

#define REMOTE_TASK_CONFIG_RESTORE_POS                            1
#define CONFIG_SYNCH_POS                                          2

/* predefined watch remote configuration wireless send data type */
#define CONFIG_SYCH_REQUEST_DATA_TYPE                             0x88


#endif


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
#ifndef GLOBAL_DATA_EXCHANGE_H__
#define GLOBAL_DATA_EXCHANGE_H__

#include "define_config.h"
#include "define_all_data.h"

/* buffer for byte read between UICC-terminal interface */
uint8_t   READ_BYTE_UICC_TERMINAL[256];
/* buffer for global temporary variable */
uint8_t   GLOBAL_TMP_RAM[256];
/* buffer for phone command receive by USIM server side, from wireless SIM interface  */
uint8_t   USIM_SERVER_COMMAND_WIRELESS_RECEIVE[256];
/* phone command read buffer, 5 bytes, used for SIM-phone interface IO pin data read */
uint8_t   PHONE_COMMAND[6];
/* authentiocation responce data, used for wireless-SIM interfavce between USIM server and client */
uint8_t   AUTHENTICATE_RESULT[KEY_LENGTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* frequency hopping pattern, for wireless-SIM interface PHY transmission */
uint8_t   FREQ_BIN_PATTEN[83] = { 82,    27,    74,     9,    75,    68,     2,    56,    42,    24,
    35,    65,     1,    26,    23,    66,    31,    39,     4,    44,    47,    60,    50,    11,
    48,    17,    64,    76,     6,    54,    32,     3,    38,    45,    79,    15,    12,    67,
    63,    40,    33,    70,    41,     5,    77,    62,    20,    13,    28,    46,    55,    43,
    61,    69,    49,    73,    37,    78,    19,    22,    30,    71,    21,    72,    18,    81,
    53,    10,    59,    80,    51,    36,    58,    29,    57,     8,    14,     7,    82,    34,
    52,    16,    25};
/* index randomization for user password transmission over the wireless-SIM link */
uint8_t   PASSWORD_SHUFFLE[16] = {0, 5, 8, 2, 6, 13, 7, 15, 11, 1, 9, 4, 3, 12, 14, 10};
/* index randomization for user system key, for authentication check of wireless-SIM link */
uint8_t   SYSTEM_KEY_SHUFFLE[16] = {8, 14, 1, 5, 2, 9, 12, 13, 6, 11, 3, 10, 15, 0, 7, 4};
/* statistics for connection request random access delay, for log output */
uint32_t  PHONE_CONNECTION_REQUEST_TIME[7] = {0, 0, 0, 0, 0, 0, 0};
/* Fixed ICCID EF data, file 0x2FE2, used for network locked iPhone */
// uint8_t   ICCID_2FE2_DATA[10] = {0x98, 0x18, 0x3, 0x0, 0x2, 0x12, 0x37, 0x45, 0x57, 0x2}; No. 1
// uint8_t   ICCID_2FE2_DATA[10] = {0x98, 0x68, 0x60, 0x0, 0x5, 0x2, 0x0, 0x81, 0x70, 0x22}; No. 2
// uint8_t   ICCID_2FE2_DATA[10] = {0x98, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf1}; //No. 4
// uint8_t   ICCID_2FE2_DATA[10] = {0x98, 0x10, 0x14, 0x30, 0x72, 0x68, 0x38, 0x49, 0x59, 0x95}; // No. 5
// uint8_t   ICCID_2FE2_DATA[10] = {0x98, 0x10, 0x14, 0x30, 0x72, 0x59, 0x92, 0x51, 0x35, 0x39}; // No. 6,
// uint8_t   ICCID_2FE2_DATA[10] = {0x98, 0x10, 0x14, 0x30, 0x72, 0x59, 0x92, 0x51, 0x45, 0x10}; // No. 8, iPhone 8P
/*
	89014103279529155401 // iPhone 8P
	89014103279529155690
	89014103279529155708
	89014103279529155427
	89014103279529155435 // iPhone 7
	89014103279529155443
	89014103279529155450 //not valid
	89014103279529155484
	89014103279529155492
	89014103279529155500
	89014103279529155526
	89014103279529155534
	89014103279529155542
	89014103279529155559
	89014103279529155591
	89014103279529155625
	89014103279529155633
	89014103279529155641
	89014103279529155658
	89014103279529155666
	89014103279529155674
	89014103279529155682
	89014103279529155724
	89014103279529155732
	89014103279529155567
	89014103279529155575
	89014103279529155583
	89014103279529155740
	89014103279529155765
	89014103279529155773
	89014103279529155468 //not valid
	89014103279529155476
*/
/* ETU tick number for intial UICC-terminal ATR session */
volatile uint16_t   ETU_TICKS_PHONE_ATR_SESSION = 1831;
/* ETU tick number for normal UICC-terminal session, between smart SIM connector and phone */
volatile uint16_t   ETU_TICKS_PHONE = (315 >> 2);
/* ETU tick number for normal UICC-terminal session, between smart SIM connector and USIM card (work functions) */
volatile uint16_t   ETU_TICKS_SIM = (256 >> 2);
/* ETU tick number for normal UICC-terminal session, between smart SIM connector and USIM card 0 */
volatile uint16_t   ETU_TICKS_SIM0 = (256 >> 2);
/* ETU tick number for normal UICC-terminal session, between smart SIM connector and USIM card 1 */
volatile uint16_t   ETU_TICKS_SIM1 = (256 >> 2);
/* current USIM card selection EF between smart SIM connector and phone */
volatile uint64_t   SELECTED_FILE_SIM = 0;
/* current USIM card 0 selection EF between smart SIM connector and phone */
volatile uint64_t   SELECTED_FILE_USIM0 = 0;
/* current USIM card 1 selection EF between smart SIM connector and phone */
volatile uint64_t   SELECTED_FILE_USIM1 = 0;
/* CLK pin number for interface between smart SIM connector and USIM card */
volatile uint8_t    PIN_CLOCK_SIM = PIN_CLOCK_SIM0;
/* data IO pin number for interface between smart SIM connector and USIM card */
volatile uint8_t    PIN_DATA_SIM = PIN_DATA_SIM0;
/* RST pin number for interface between smart SIM connector and USIM card */
volatile uint8_t    PIN_RESET_SIM = PIN_RESET_SIM0;
/* VCC pin number for interface between smart SIM connector and USIM card */
volatile uint8_t    PIN_VCC_SIM = PIN_VCC_SIM0;
/* if the BLE stack softdevice is active, 0: softdevice is not active, 1: softdevice is active (running) */
volatile uint8_t    IF_SOFTDEVICE_RUNNING = 0;
/* logical address of USIM client for wireless-SIM interface, 1 - 7. logical adress 0 is reserved for USIM server */
volatile uint8_t    USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS = 0;
/* --------------------------------------------------------------------------
* PHONE_LOCAL_USED_USIM: the USIM data used by phone
* bit0 = 1: USIM0 is used
* bit1 = 1: USIM0 is detected
* bit2 = 1: USIM1 is used
* bit3 = 1: USIM1 is detected
* bit4 = 1: wireless USIM0 is used
* bit5 = 1: wireless USIM0 is updated
* bit6 = 1: wireless USIM1 is used
* bit7 = 1: wireless USIM1 is updated
 -------------------------------------------------------------------------- */
volatile uint8_t    PHONE_LOCAL_USED_USIM = 0;
/* --------------------------------------------------------------------------
* USIM_CARD_PRESENT: the USIM card present detection result
* bit0 = 1: USIM0 is existed
* bit1 = 1: USIM1 is existed
* bit2 ~ bit7: not used
 -------------------------------------------------------------------------- */
volatile uint8_t    USIM_CARD_PRESENCE = 0;
/* --------------------------------------------------------------------------
* PHONE_WIRELESS_USED_USIM: the USIM card number used for wireless SIM interface
* bit0 = 0: USIM0 is used
* bit1 = 1: USIM1 is used
* bit2 ~ bit7: not used
 -------------------------------------------------------------------------- */
volatile uint8_t    PHONE_WIRELESS_USED_USIM = 0;
/* flasg for USIM switch precedure is ongoing */
volatile uint8_t    FLAG_USIM_SWITCH = 0;
/* command counter for phone command 0xc2 */
volatile uint8_t    COMMOD_ENVELOPE_0XC2_COUNTER = 0;
/* text type for idle mode text display, to indicate the status of wireless-SIM connection */
volatile uint8_t    IDLE_MODE_TEXT_TYPE = 0;
/* the start time of time slot request for connection request from USIM client (ms), from RTC2 */
volatile uint32_t   PHONE_CONNECTION_REQUEST_START_TIME = 0;                
/* the start time of time slot request for connection request listening from USIM server (ms), from RTC2 */
volatile uint32_t   TIME_SLOT_REQUEST_START_TIME = 0;
/* EF file ID for USIM card binary update */
volatile uint32_t   BINARY_UPDATE_FILE_ID = 0;
/* if the phone call is on for USIM server side phone.
0: call off, only provide wireless-SIM authentication phone command 0x88 server,
1: call on, both USIM client and server can initial a phone call */
volatile uint8_t    USIM_SERVER_CALL_ON = 1;
/* connection status, each bit indicate a connection requenst for wireless-SIM, 1: connected, 0: disconnecteed. */
volatile uint32_t   CONNECTION_STATE = 0x00000000;
/* bytes number for USAT command, waiting for phone poll (phone command status, 0xf2) */
volatile uint8_t    USAT_BYTE_LENGTH_BACK = 0;
/* Phone command fetch 0x12 type, or different text display */
volatile uint8_t    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;
/* software interrupt 3 type, for different event handler */
volatile uint8_t    FLAG_SWI3_EGU3_IRQn = 0;
/* wireless-SIM connection status */
volatile uint8_t    WIRELESS_SIM_CONNECTION_STATUS = 0;
/* flag to indicate the phone command (5 bytes) read status.
0: no phone command read is ongoing, 1: phone command read is ongoing */
volatile uint8_t    FLAG_PHONE_COMMAND_READ = 0;
/* the next phone command status 0xf2 tiem prediction according the historical timing */
volatile uint32_t   NEXT_PHONE_COMMAND_0XF2_TIME;
/* BLE stack softdevice stopped time stamp */
volatile uint32_t   BLE_STOP_TIME = 0;
/* BLE stack softdevice started time stamp */
volatile uint32_t   BLE_START_TIME = 0;
/* flasg to indicate if the intial USAT menu are setup */
volatile uint32_t   FLAG_INITIAL_USAT_MENU_SETUP = 0;
/* flag for task queue, the task is run after BLE stopped */
volatile uint32_t   BLE_STOPPED_TASK_QUEUE = 0;
/* carrier frequency for data transmission, it is allocated by USIM server */
volatile uint8_t    DATA_CARRIER_FREQ = 0;
/* selected file between smart sim connector and phone */
volatile uint64_t   SELECTED_FILE = 0x0;
/* time slot task stage for USIM card binary update */
volatile uint8_t    SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = 0;
/* task queue for USIM card binary update */
volatile uint32_t   SIM_FILE_UPDATE_BINARY_TASK_QUEUE = 0;
/* time slot task stage for wireless-SIM authentication phone command 0x88, USIM client side */
volatile uint8_t    PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = 0;
/* time slot task stage for wireless-SIM get data phone command 0xc0, USIM client side */
volatile uint8_t    PHONE_COMMAND_GET_FILE_DATA_BLE_STAGE = 0;
/* flast write stage with BLE stach softdevice sctive */
volatile uint8_t    FLASH_UPDATE_WAITING_STAGE = 0;
/* mark the authentication from logical address 7 (reserved for iPAD) */
volatile uint8_t    FLAG_DEFAULT_RECEIVER = 1;
/* mark the authentication from logical address 7 (reserved for iPAD) */
volatile uint8_t    FLAG_RECEIVE_CALL_UPDATE = 0;
/* time slot task stage for USIM server local authentication phone command 0x88 */
volatile uint8_t    WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = 0;
/* time slot task stage forUSIM server local get data phone command 0xc0 */
volatile uint8_t    USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE = 0;
/* SIM data read number inside GPIOTE handler, for authentication USIM byte read */
volatile uint8_t    SIM_READ_BYTES_NUMBER = 0;
/* mark the ICCID EF 0x2fe2 selected, for USIM server local SIM use */
volatile uint8_t    IF_2FE2_SELECTED = 0;
/* time slot task stage for wireless-SIM authentication phone command 0x88, USIM server side */
volatile uint8_t    USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE = 0;
/* if the phone call is on for USIM server side phone.
0: call off, only provide wireless-SIM authentication phone command 0x88 server,
1: call on, both USIM client and server can initial a phone call
current in RAM  */
volatile uint8_t    USIM_SERVER_CALL_ON_FLAG_GLOBAL =0;
/* task queue for wireless-SIM connection command between USIM server and client */
volatile uint32_t   USIM_PHONE_CONNECTION_COMMAND = 0;
/* time stamp of the phome command 0xc0 start time */
volatile uint32_t   PHONE_COMMAND_GET_FILE_DATA_START_TIME = 0;
/* time stamp of receiving SIM data start time from wireless-SIM interface */
volatile uint32_t   SIM_DATA_RECEIVE_COMMAND_START_TIME = 0;
/* time stamp of the wireless authentication phone command start time */
volatile uint32_t   PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME = 0;
/* return value for time slot signal handler of wireless authentication at USIM client side */
volatile uint32_t   RETURN_PHONE_AUTHENTICATION_WIRELESS             = SIGNAL_CALLBACK_RETURN_UNDO;
/* return value for time slot signal handler of wireless data transmission at USIM client side */
volatile uint32_t   RETURN_PHONE_CONNECTION_REQUEST_DATA_SEND        = SIGNAL_CALLBACK_RETURN_UNDO;
/* return value for time slot signal handler of wireless EF data get (connection request) at USIM client side */
volatile uint32_t   RETURN_PHONE_GET_FILE_DATA_REQUEST               = SIGNAL_CALLBACK_RETURN_UNDO;
/* return value for time slot signal handler of wireless EF data get (data receive) at USIM client side */
volatile uint32_t   RETURN_PHONE_GET_FILE_DATA_RECEIVE               = SIGNAL_CALLBACK_RETURN_UNDO;
/* return value for time slot signal handler of wireless USIM connection request at USIM client side */
volatile uint32_t   RETURN_PHONE_CONNECTION_REQUEST                  = SIGNAL_CALLBACK_RETURN_UNDO;
/* return value for time slot signal handler of USIM card binary update at USIM server side */
volatile uint32_t   RETURN_USIM_FILE_BINAREY_UPDATE                  = SIGNAL_CALLBACK_RETURN_UNDO;
/* time slot request semaphore, 1: lock the time slot request semaphore. 0: free the time slot request semaphore */
volatile uint8_t    SEMAPHORE_TIME_SLOT_REQUEST = 0;  
/* time slot request task type in side handler  */
volatile uint8_t    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_DEFAULT_VALUE; 
/* time slot request task type in side time slot signal handler  */
volatile uint8_t    TIME_SLOT_SIGNAL_TYPE; 
/* time slot event type in side time slot event handler  */
volatile uint8_t    TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
/* time slot request time length, in us */ 
volatile uint32_t   TIME_SLOT_REQUEST_LENGTH_US = 1000; 
/* phone connection request type, to indicate the request is a random access request or data transmission */
volatile uint8_t    PHONE_CONNECTION_REQUEST_TYPE;
/* flash data write task queue for different data flash write */           
volatile uint32_t   FLASH_DATA_WRITE_CHECK_TASK_QUEUE = 0; 
/* start time in ms for connection time slot */
volatile uint32_t   CONNECTION_SLOT_TIME_MS = 0;
/* LED flash pattern */
volatile uint8_t    LED_PATTERN_BLE_FLASH[16];
	
#endif
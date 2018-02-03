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
#ifndef GLOBAL_DATA_EXCHANGE_EXTERN_H__
#define GLOBAL_DATA_EXCHANGE_EXTERN_H__

#include "define_config.h"

extern uint8_t   READ_BYTE_UICC_TERMINAL[256];
extern uint8_t   GLOBAL_TMP_RAM[256];
extern uint8_t   USIM_SERVER_COMMAND_WIRELESS_RECEIVE[256];
extern uint8_t   PHONE_COMMAND[6];
extern uint8_t   AUTHENTICATE_RESULT[KEY_LENGTH];
extern uint8_t   FREQ_BIN_PATTEN[83];
extern uint8_t   PASSWORD_SHUFFLE[16];	 
extern uint8_t   SYSTEM_KEY_SHUFFLE[16];	 
extern uint32_t  PHONE_CONNECTION_REQUEST_TIME[7];
extern uint8_t   ICCID_2FE2_DATA[10];
	
extern volatile uint16_t   ETU_TICKS_PHONE_ATR_SESSION;                  
extern volatile uint16_t   ETU_TICKS_PHONE;                           
extern volatile uint16_t   ETU_TICKS_SIM;                            
extern volatile uint16_t   ETU_TICKS_SIM0;                           
extern volatile uint16_t   ETU_TICKS_SIM1;                           
extern volatile uint64_t   SELECTED_FILE_SIM;                     
extern volatile uint64_t   SELECTED_FILE_USIM0;                      
extern volatile uint64_t   SELECTED_FILE_USIM1;                      
extern volatile uint8_t    PIN_CLOCK_SIM;                             
extern volatile uint8_t    PIN_DATA_SIM;                             
extern volatile uint8_t    PIN_RESET_SIM;                            
extern volatile uint8_t    PIN_VCC_SIM;                          
extern volatile uint8_t    IF_SOFTDEVICE_RUNNING;                    
extern volatile uint8_t    USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS;
extern volatile uint8_t    PHONE_WIRELESS_USED_USIM;
extern volatile uint8_t    PHONE_LOCAL_USED_USIM; 
extern volatile uint8_t    USIM_CARD_PRESENCE;
extern volatile uint8_t    FLAG_USIM_SWITCH;
extern volatile uint8_t    COMMOD_ENVELOPE_0XC2_COUNTER;
extern volatile uint8_t    IDLE_MODE_TEXT_TYPE;
extern volatile uint32_t   PHONE_CONNECTION_REQUEST_START_TIME;                                     
extern volatile uint32_t   TIME_SLOT_REQUEST_START_TIME;
extern volatile uint32_t   BINARY_UPDATE_FILE_ID;
extern volatile uint8_t    USIM_SERVER_CALL_ON;
extern volatile uint32_t   CONNECTION_STATE;
extern volatile uint8_t    USAT_BYTE_LENGTH_BACK;
extern volatile uint8_t    FETCH_COMMAND_TYPE;
extern volatile uint8_t    FLAG_SWI3_EGU3_IRQn;
extern volatile uint8_t    WIRELESS_SIM_CONNECTION_STATUS;   
extern volatile uint8_t    FLAG_PHONE_COMMAND_READ;
extern volatile uint32_t   NEXT_PHONE_COMMAND_0XF2_TIME;
extern volatile uint32_t   BLE_STOP_TIME;
extern volatile uint32_t   BLE_START_TIME;
extern volatile uint32_t   FLAG_INITIAL_USAT_MENU_SETUP;
extern volatile uint32_t   BLE_STOPPED_TASK_QUEUE;
extern volatile uint8_t    DATA_CARRIER_FREQ;
extern volatile uint64_t   SELECTED_FILE;
extern volatile uint8_t    SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE;
extern volatile uint32_t   SIM_FILE_UPDATE_BINARY_TASK_QUEUE;
extern volatile uint8_t    PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE;
extern volatile uint8_t    PHONE_COMMAND_GET_FILE_DATA_BLE_STAGE;
extern volatile uint8_t    FLASH_UPDATE_WAITING_STAGE;
extern volatile uint8_t    FLAG_DEFAULT_RECEIVER;
extern volatile uint8_t    FLAG_RECEIVE_CALL_UPDATE; 
extern volatile uint8_t    WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE;
extern volatile uint8_t    USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE;
extern volatile uint8_t    SIM_READ_BYTES_NUMBER; 
extern volatile uint8_t    IF_2FE2_SELECTED; 
extern volatile uint8_t    USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE;
extern volatile uint8_t    USIM_SERVER_CALL_ON_FLAG_GLOBAL;
extern volatile uint32_t   USIM_PHONE_CONNECTION_COMMAND;

extern volatile uint32_t   PHONE_COMMAND_GET_FILE_DATA_START_TIME;
extern volatile uint32_t   SIM_DATA_RECEIVE_COMMAND_START_TIME;                                   
extern volatile uint32_t   PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME;
extern volatile uint32_t   RETURN_PHONE_AUTHENTICATION_WIRELESS;
extern volatile uint32_t   RETURN_PHONE_CONNECTION_REQUEST_DATA_SEND;
extern volatile uint32_t   RETURN_PHONE_GET_FILE_DATA_RECEIVE;
extern volatile uint32_t   RETURN_PHONE_CONNECTION_REQUEST;
extern volatile uint32_t   RETURN_USIM_FILE_BINAREY_UPDATE;
extern volatile uint8_t    SEMAPHORE_TIME_SLOT_REQUEST;  
extern volatile uint8_t    TIME_SLOT_SIGNAL_TYPE_SET; 
extern volatile uint8_t    TIME_SLOT_SIGNAL_TYPE; 
extern volatile uint8_t    TIME_SLOT_EVENT_TYPE; 
extern volatile uint32_t   TIME_SLOT_REQUEST_LENGTH_US; 
extern volatile uint8_t    PHONE_CONNECTION_REQUEST_TYPE;                                 
extern volatile uint32_t   FLASH_DATA_WRITE_CHECK_TASK_QUEUE; 
extern volatile uint32_t   CONNECTION_SLOT_TIME_MS;
extern volatile uint8_t    LED_PATTERN_RAM[16];

#endif

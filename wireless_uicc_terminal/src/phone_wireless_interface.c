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
#include <string.h>
#include "define_config.h"
#include "phone_wireless_interface.h"
#include "phone_command_sim.h"
#include "comm_initial.h"
#include "comm_wireless_interface.h"
#include "comm_phone_command.h"
#include "comm_uicc_terminal_interface.h"
#include "flash_file_address.h"
#include "usim_server_command_sim.h"
#include "usim_server_sim_interface.h"
#include "nrf_nvic.h"
#include "wireless_sim_phone.h"
#include "time_slot.h"
#include "usim_server_wireless_interface.h"
#include "comm_phone_command.h"
#include "global_data_exchange_extern.h"
#include "flash_data_write.h"
#include "nrf_delay.h"
#include "define_all_data.h"

extern uint8_t AUTHENTICATE_RESULT[KEY_LENGTH];
extern uint8_t PHONE_COMMAND[6];
extern volatile uint8_t FETCH_COMMAND_TYPE;
extern volatile uint8_t USAT_BYTE_LENGTH_BACK;
extern volatile uint32_t CONNECTION_STATE;
extern uint8_t SIM_CLIENT_ON_USIM0_CONNECTED_STRING[];
extern uint8_t SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT[];
extern uint8_t SIM_CLIENT_ON_USIM1_CONNECTED_STRING[];
extern uint8_t SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT[];
extern volatile uint8_t WIRELESS_SIM_CONNECTION_STATUS;
extern uint8_t PASSWORD_SHUFFLE[16];
extern uint8_t SYSTEM_KEY_SHUFFLE[16];

extern volatile uint8_t FLAG_SWI3_EGU3_IRQn;

#if (IF_DATA_CC_CODED || IF_CONNECTION_CC_CODED)
  extern uint8_t CC_CODED_BYTE[256];
#endif

/********************************************************************************/
void phone_command_authentication_main(void)
/*--------------------------------------------------------------------------------
| phone command between phone-SIM interface at USIM server side locally and
| wireless USIM client aide.
| phone command 0x88: authentication
|
--------------------------------------------------------------------------------*/
{
  if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0) /* get authentication data wirelessly */
  {
    /* Set radio configuration parameters */
    radio_configure();
#if (IF_LOG_OUTPUT)
    printf("--------------------------- wireless authentication remotely, BLE off, 0x88 ---------------------------\r\n");
#endif
    phone_command_authentication_wireless(PHONE_COMMAND);
  }
  else /* get authentication data by local USIM card */
  {
#if (IF_LOG_OUTPUT)
    printf("--------------------------- local authentication from SIM, BLE off, 0x88 ---------------------------\r\n");
#endif
    /* USIM0 is used by phone locally */
    if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
    {
      set_default_usim0_etu( );
    }
#if (PIN_VCC_SIM1 != PIN_NULL)
    /* USIM1 is used by phone locally */
    else if (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1)
    {
      set_default_usim1_etu( );
    }
#endif
    else
    {
#if (IF_LOG_OUTPUT)
      printf("++++++++++++++++++ PHONE_LOCAL_USED_USIM value error, PHONE_LOCAL_USED_USIM = %x ++++++++++++++++++\r\n", PHONE_LOCAL_USED_USIM);
#endif
      PHONE_LOCAL_USED_USIM |= (1 << 0);
      set_default_usim0_etu( );
    }
    /* start sim clock signal */
    clock_sim_start_4m(PIN_CLOCK_SIM);

    sim_file_7fff_selected_check( );

    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;

    usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);

    /* stop SIM clock */
    clock_sim_stop_4m(PIN_CLOCK_SIM);
  }

  return;
}

/********************************************************************************/
void main_smart_phone_sim_local_wireless(uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone command between phone-SIM interface at smart phone side locally and
| over the air between phone and watch
| wireless phine-watch:    0x88: authentication
| local phone-SIM command: all other phone cammand
|
--------------------------------------------------------------------------------*/
{
  uint8_t tmp_byes[2];

  switch(*(PHONE_COMMAND + 2))
  {
    case 0xa4:
      phone_command_select_0xa4(PHONE_COMMAND, ETU_TICKS_PHONE, start_time_us, time_length_us);
      break;

    case 0xb0:
      phone_command_read_binary_0xb0(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0x10:
      phone_command_terminal_profile_0x10(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      break;

    case 0x14:
      phone_command_terminal_response_0x14(PHONE_COMMAND, ETU_TICKS_PHONE, start_time_us, time_length_us);
      break;

    case 0xb2:
      phone_command_read_record_0xb2(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0x2c:
      phone_command_unblock_pin_0x2c(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0x20:
      /* initial USAT menu setting */
      USAT_BYTE_LENGTH_BACK = USAT_BYTE_LENGTH_BACK_INIT;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_INITIAL_USAT_MENU_SETUP;
      phone_command_verify_0x20(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0xa2:
      phone_command_search_record_0xa2(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0xf2:
      phone_command_status_response_0xf2(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      // usim_server_command_status(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0xc0:
      if (*(PHONE_COMMAND + 5))
      {
#if (IF_LOG_OUTPUT)
        printf("--------------------------- get authentication data from SIM， BLE off, 0xc0 ---------------------------\r\n");
#endif
        /* start sim clock signal */
        clock_sim_start_4m(PIN_CLOCK_SIM);

        usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);

        /* stop SIM clock */
        clock_sim_stop_4m(PIN_CLOCK_SIM);
      }
      else
      {
        tmp_byes[0] = 0x90;
        tmp_byes[1] = 0x0;
        write_bytes(2, tmp_byes, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      }
      break;

    case 0x12:
      phone_command_fetch_0x12(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      break;

    case 0xd6:
      phone_command_update_binary_0xd6(PHONE_COMMAND, ETU_TICKS_PHONE, start_time_us, time_length_us);
      break;

    case 0x88:
      phone_command_authentication_main( );
      break;

    case 0xc2:
      phone_command_envelope_0xc2(PHONE_COMMAND, ETU_TICKS_PHONE, start_time_us, time_length_us);
      break;

    case 0x24:
    case 0x26:
    case 0x28:
      phone_command_pin_0x24(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    case 0xdc:
      phone_command_update_record_0xdc(PHONE_COMMAND, ETU_TICKS_PHONE, start_time_us, time_length_us);
      break;

    case 0x70:
      phone_command_manage_channel_0x70(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;

    default:
      phone_command_default_response(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      break;
  }

  set_event_phone_command( );
}

/********************************************************************************/
void phone_connection_state_check(uint8_t status_connection_request)
/*--------------------------------------------------------------------------------
| radio link connection status check at phone side, phone transmit an access request
| and listen the ACK feedback to check if there is radio link between phone and watch
|
--------------------------------------------------------------------------------*/
{
  static uint32_t volatile DISCONNECTION_COUNTER = 0;
  static uint32_t volatile CONNECTION_COUNTER = 0;

  /* stop 16M oscillator */
  if (!IF_SOFTDEVICE_RUNNING)
  {
    if (status_connection_request != SIM_LISTENING_RETURN_16M_OSC_ERROR)
    {
      stop_oscillator_16m( );
    }
  }

  if (status_connection_request)
  {
    DISCONNECTION_COUNTER++;
  }
  else
  {
    /* move b0 to b1, mark the previous connection state */
    if (CONNECTION_STATE & 0x1)
    {
      CONNECTION_STATE |= 0x2;
    }
    else
    {
      CONNECTION_STATE &= 0xFFFFFFFD;
    }
    /* mark b0 = 1 */
    CONNECTION_STATE |= 0x1;

    /* clear disconnection counter */
    DISCONNECTION_COUNTER = 0;
    /* increase connection counter */
    CONNECTION_COUNTER++;
  }

  /* disconnection counter overflow, phone assume there is no connection between phone-USIM */
  if (DISCONNECTION_COUNTER >= PHONE_DISCONNECT_TIME)
  {
    /* move b0 to b1, mark the previous connection state */
    if (CONNECTION_STATE & 0x1)
    {
      CONNECTION_STATE |= 0x2;
    }
    else
    {
      CONNECTION_STATE &= 0xFFFFFFFD;
    }
    /* mark b0 = 0 */
    CONNECTION_STATE &= 0xFFFFFFFE;

    /* clear disconnection and connection counter */
    DISCONNECTION_COUNTER = 0;
    CONNECTION_COUNTER = 0;

    /* connection state is from connection to disconnection */
    if((CONNECTION_STATE & 0x3) == 2)
    {
      /* phone disconnect from the network */
      /* to be added */
    }

#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      if (WIRELESS_SIM_ON_OFF_RAM)
      {
        printf("==================== Wireless SIM On, Not connected, CONNECTION_STATE = %ld ==================== \r\n", CONNECTION_STATE & 0x7);
      }
      else
       {
        printf("++++++++++++++++++++ Wireless SIM Off, Not connected, CONNECTION_STATE = %ld ++++++++++++++++++++ \r\n", CONNECTION_STATE & 0x7);
      }
    }
#endif

    /* if current phone-watch is connected */
    if ((WIRELESS_SIM_CONNECTION_STATUS == 1) && WIRELESS_SIM_ON_OFF_RAM)
    {
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_DISCONNECTED_CLIENT;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT);
      }
      else
      {
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_DISCONNECTED_CLIENT;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT);
      }
    }

    /* mark the phone-watch as disconnection state */
    if (WIRELESS_SIM_CONNECTION_STATUS == 1)
    {
      WIRELESS_SIM_CONNECTION_STATUS = 0;
    }
  }

  /* connection counter overflow, USIM assume there is connection between phone-USIM */
  if (CONNECTION_COUNTER)
  {
    CONNECTION_COUNTER = 0;

    /* connection state is from disconnection to connection */
    if((CONNECTION_STATE & 0x3) == 1)
    {
      /* mark the USAT bit for reset NAA, let phone access the network */
      CONNECTION_STATE |= 0x4;
    }

#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      printf("==================== Wireless SIM On, Connected, CONNECTION_STATE = %ld  ==================== \r\n", CONNECTION_STATE & 0x7);
    }
#endif

    /* if current phone-watch is disconnected */
    if (WIRELESS_SIM_CONNECTION_STATUS == 0)
    {
      /* mark the phone-watch as disconnection state */
      WIRELESS_SIM_CONNECTION_STATUS = 1;
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_CONNECTED_CLIENT;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM0_CONNECTED_STRING);
      }
      else
      {
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_CONNECTED_CLIENT;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM1_CONNECTED_STRING);
      }
    }
  }
}

/********************************************************************************/
uint32_t connection_request_phone(uint8_t connection_type)
/*--------------------------------------------------------------------------------
| radio link transmission at phone side for random access request
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t connection_byte[CONNECTION_REQUEST_BYTE];
  uint32_t tx_logical_address;
  uint32_t rx_logical_address;
#if (IF_CONNECTION_CC_CODED)
  uint32_t connection_request_wait_feedback_ms = PHONE_REQUEST_WAIT_TIME_MS + (TIMER_SLOT_CC_DECODING_TIME_US + TIMER_SLOT_CC_ENCODING_TIME_US) / 1000;
#else
  uint32_t connection_request_wait_feedback_ms = PHONE_REQUEST_WAIT_TIME_MS;
#endif
  uint32_t initial_timer;

  if (start_oscillator_16m( ))
  {
    return(3);
  }
  if (!WIRELESS_SIM_ON_OFF_RAM)
  {
#if (IF_LOG_OUTPUT)
    printf("++++++++++++++++++++ BLE off, Wireless SIM Off, no request transmitted ++++++++++++++++++++ \r\n");
#endif
    return(1);
  }

  tx_logical_address = (USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS & 7);
  rx_logical_address = (1 << tx_logical_address);

  /* transmit byte length */
  connection_byte[0] = 25;
  /* bit0-bit2: phone logical address, bit4: default used by wireless USIM, 0: USIM0, 1: USIM1. repeat 3 times */
  connection_byte[1] = (USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS & 7);
  connection_byte[1] |= ((PHONE_WIRELESS_USED_USIM & 0x0f) << 4);
  connection_byte[2] = connection_byte[1];
  connection_byte[3] = connection_byte[1];
  /* connection request type, repeat 3 times */
  connection_byte[4] = connection_type;
  connection_byte[5] = connection_type;
  connection_byte[6] = connection_type;
  /*  */
  connection_byte[7] = DATA_CARRIER_FREQ;
  connection_byte[8] = DATA_CARRIER_FREQ;
  connection_byte[9] = DATA_CARRIER_FREQ;
  /* insert the user passwaord in the connect request message */
  for (i=0; i<KEY_LENGTH; i++)
  {
    connection_byte[i + 10] = (*(USER_PASSWORD_RAM + PASSWORD_SHUFFLE[i])) ^ (*(SYSTEM_KEY_RAM + SYSTEM_KEY_SHUFFLE[i]));
  }

#if (IF_CONNECTION_CC_CODED)
  /* connection request message byte length, coding rate = 1/2 */
  CC_CODED_BYTE[0] = (CONNECTION_REQUEST_BYTE * 2);
  /* cc encoding for the connection request information bytes */
  ccencoder_byte_sequence_32(connection_byte, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
#else
#if (IF_USIM_BINARY_UPDATE)
  if ((SIM_FILE_UPDATE_BINARY_TASK_QUEUE) && (BINARY_UPDATE_FILE_ID == 0))
  {
    if (!usim_binary_update_piggyback(connection_byte + 26))
    {
      connection_byte[0] = 63;
    }
  }
  else
  {
    *(connection_byte + 26) = 0;
    *(connection_byte + 27) = 0;
    *(connection_byte + 28) = 0;
    *(connection_byte + 29) = 0;
  }
#endif
#endif


  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;

  /* the first time connection request transmission try */
  while (1)
  {
    radio_carrier_detection(MAX_CD_TIME_MS, TX_RSSI_THRED, FREQUENCY_CARRIER_ACCESS);
    /* transmitted successfully */
#if (IF_CONNECTION_CC_CODED)
    if (!packet_radio_with_parameters_tx(CC_CODED_BYTE, FREQUENCY_CARRIER_ACCESS,
           tx_logical_address, NRF_RTC2->COUNTER, PHONE_REQUEST_TX_TIME_MS, POWER_LEVEL_CONNECTION))
#else
    if (!packet_radio_with_parameters_tx(connection_byte, FREQUENCY_CARRIER_ACCESS,
           tx_logical_address, NRF_RTC2->COUNTER, PHONE_REQUEST_TX_TIME_MS, POWER_LEVEL_CONNECTION))
#endif
    {
      break;
    }
    /* Tx transmission tried time out */
    if (((NRF_RTC2->COUNTER - initial_timer) > DATA_RX_TIME_MS) || (NRF_RTC2->COUNTER < initial_timer))
    {
      /* wireless link mantenance, transmission fails, return successful */
      if (connection_type == 0x11)
      {
        return(0);
      }
      /* data transmission, transmission fails, return error */
      else
      {
        return(1);
      }
    }

    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
  }

  /* use global tmp variable */
  *GLOBAL_TMP_RAM = 0;
  /* connection request transmission try again, if connection response not received */
  while (packet_radio_with_parameters_rx(GLOBAL_TMP_RAM, DATA_CARRIER_FREQ, rx_logical_address,
         NRF_RTC2->COUNTER, connection_request_wait_feedback_ms))
  {
    /* connection request try transmission */
    while (1)
    {
      radio_carrier_detection(MAX_CD_TIME_MS, TX_RSSI_THRED, FREQUENCY_CARRIER_ACCESS);
      /* transmitted successfully */
#if (IF_CONNECTION_CC_CODED)
      if (!packet_radio_with_parameters_tx(CC_CODED_BYTE, FREQUENCY_CARRIER_ACCESS, tx_logical_address,
             NRF_RTC2->COUNTER, PHONE_REQUEST_TX_TIME_MS, POWER_LEVEL_CONNECTION))
#else
      if (!packet_radio_with_parameters_tx(connection_byte, FREQUENCY_CARRIER_ACCESS, tx_logical_address,
             NRF_RTC2->COUNTER, PHONE_REQUEST_TX_TIME_MS, POWER_LEVEL_CONNECTION))
#endif
      {
        break;
      }
      /* Tx transmission tried time out */
      if (((NRF_RTC2->COUNTER - initial_timer) > DATA_RX_TIME_MS) || (NRF_RTC2->COUNTER < initial_timer))
      {
        /* wireless link mantenance, transmission fails, return successful */
        if (connection_type == 0x11)
        {
          return(0);
        }
        /* data transmission, transmission fails, return error */
        else
        {
          return(1);
        }
      }

      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }

    /* Rx tried time out */
    if (((NRF_RTC2->COUNTER - initial_timer) > DATA_RX_TIME_MS) || (NRF_RTC2->COUNTER < initial_timer))
    {
#if (IF_LOG_OUTPUT)
      (PHONE_CONNECTION_REQUEST_TIME[6])++;
      printf("\r\n----------- BLE off, Time out for phone connection request, Rx time used = %ld -----------\r\n", NRF_RTC2->COUNTER - initial_timer);
      printf("\r\n---------------------------- PHONE_CONNECTION_REQUEST_TIME = %ld, %ld, %ld, %ld, %ld, %ld, %ld ----------------------------\r\n", PHONE_CONNECTION_REQUEST_TIME[0], PHONE_CONNECTION_REQUEST_TIME[1], PHONE_CONNECTION_REQUEST_TIME[2], PHONE_CONNECTION_REQUEST_TIME[3], PHONE_CONNECTION_REQUEST_TIME[4], PHONE_CONNECTION_REQUEST_TIME[5], PHONE_CONNECTION_REQUEST_TIME[6]);
#endif

      return(1);
    }

    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
  }

  return (connection_request_phone_post_decoding(connection_type, initial_timer));
}

/********************************************************************************/
uint32_t connection_request_phone_post_decoding(uint8_t connection_type, uint32_t initial_timer)
/*--------------------------------------------------------------------------------
| radio link transmission at phone side for random access request
|
--------------------------------------------------------------------------------*/
{
  uint8_t connection_byte[CONNECTION_REQUEST_BYTE];

  /* connection response message received */
#if (IF_CONNECTION_CC_CODED)
  /* cc decoding for connection response message */
  ccdecoder_byte_sequence_32(GLOBAL_TMP_RAM + 1, ENCODE_BLOCK_INDEX, connection_byte);
#else
  for (uint8_t i=0; i<CONNECTION_REQUEST_BYTE; i++)
  {
    *(connection_byte + i) = *(GLOBAL_TMP_RAM + i);
  }
#endif

  /* simple receiver error check */
  if ((*(connection_byte + 1) != *(connection_byte + 2)) || (*(connection_byte + 1) != *(connection_byte + 3)))
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n------------------------- BLE status = %d, phone logical address receive error in connection_request_phone -------------------------\r\n", IF_SOFTDEVICE_RUNNING);
    printf_log_rx(CONNECTION_REQUEST_BYTE, connection_byte);
#endif
    return(1);
  }
  if ((*(connection_byte + 4) != *(connection_byte + 5)) || (*(connection_byte + 4) != *(connection_byte + 6)))
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n------------------------- BLE BLE status = %d, phone frequency carrier receive error in connection_request_phone -------------------------\r\n", IF_SOFTDEVICE_RUNNING);
    printf_log_rx(CONNECTION_REQUEST_BYTE, connection_byte);
#endif
    return(1);
  }

  /* save the carrier frequency for data transmission use later */
  DATA_CARRIER_FREQ = connection_byte[4];
  USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS = (connection_byte[1] & 7);

  /* wireless USIM0 or USIM1 is used, feedback from wireless USIM server */
  if ((connection_byte[1] >> 4) != 0)
  {
    if (((PHONE_WIRELESS_USED_USIM & 1) == 0) && (((connection_byte[1] >> 5) & 1) == 1))
    {
      PHONE_WIRELESS_USED_USIM = 0;
      PHONE_WIRELESS_USED_USIM |= ((connection_byte[1] >> 4) & 0xfe);
    }
    else if (((PHONE_WIRELESS_USED_USIM & 1) == 1) && (((connection_byte[1] >> 6) & 1) == 1))
    {
      PHONE_WIRELESS_USED_USIM = 1;
      PHONE_WIRELESS_USED_USIM |= ((connection_byte[1] >> 4) & 0xfe);
    }
    else
    {
      // PHONE_WIRELESS_USED_USIM = (connection_byte[1] >> 4);

      /* change the default wireless USIM No. */
      /* current used wireless USIM0 */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        DEFAULT_USED_USIM_RAM = 1;
        /* set bit0 = 1 */
        PHONE_WIRELESS_USED_USIM |= 1;

        /* set bit4=0 */
        PHONE_LOCAL_USED_USIM &= (~(1 << 4));
        /* set bit6=1 */
        PHONE_LOCAL_USED_USIM |= (1 << 6);
      }
      else
      /* current used wireless USIM1 */
      {
        DEFAULT_USED_USIM_RAM = 0;
        /* set bit0 = 0 */
        PHONE_WIRELESS_USED_USIM &= 0xfe;

        /* set bit4=1 */
        PHONE_LOCAL_USED_USIM |= (1 << 4);
        /* set bit6=0 */
        PHONE_LOCAL_USED_USIM &= (~(1 << 6));
      }

      /* flash save */
      if (IF_SOFTDEVICE_RUNNING)
      {
        /* set flash write flag */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
        /* set the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS);

        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;
        USAT_BYTE_LENGTH_BACK = 0;
      }
      else
      {
        FLAG_USIM_SWITCH = 1;

        /* write to flash */
        start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
        /* set the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS);

        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_SIM_RESET_FETCH;
        USAT_BYTE_LENGTH_BACK = 11;
      }
    }
  }

#if (IF_LOG_OUTPUT)
  if (NRF_RTC2->COUNTER - initial_timer < 200)
  {
    (PHONE_CONNECTION_REQUEST_TIME[0])++;
  }
  else if (NRF_RTC2->COUNTER - initial_timer < 400)
  {
    (PHONE_CONNECTION_REQUEST_TIME[1])++;
  }
  else if (NRF_RTC2->COUNTER - initial_timer < 600)
  {
    (PHONE_CONNECTION_REQUEST_TIME[2])++;
  }
  else if (NRF_RTC2->COUNTER - initial_timer < 800)
  {
    (PHONE_CONNECTION_REQUEST_TIME[3])++;
  }
  else if (NRF_RTC2->COUNTER - initial_timer < 1000)
  {
    (PHONE_CONNECTION_REQUEST_TIME[4])++;
  }
  else if (NRF_RTC2->COUNTER - initial_timer < 1200)
  {
    (PHONE_CONNECTION_REQUEST_TIME[5])++;
  }
  else
  {
    (PHONE_CONNECTION_REQUEST_TIME[6])++;
  }

  printf("\r\n---------------------------- BLE status = %d, Phone data carrier frequency = %d, phone logical address = %d, PHONE_WIRELESS_USED_USIM = 0x%x, request time used = %ldms----------------------------\r\n", IF_SOFTDEVICE_RUNNING, DATA_CARRIER_FREQ, USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS, PHONE_WIRELESS_USED_USIM, NRF_RTC2->COUNTER - initial_timer);
  printf("\r\n---------------------------- PHONE_CONNECTION_REQUEST_TIME = %ld, %ld, %ld, %ld, %ld, %ld, %ld ----------------------------\r\n", PHONE_CONNECTION_REQUEST_TIME[0], PHONE_CONNECTION_REQUEST_TIME[1], PHONE_CONNECTION_REQUEST_TIME[2], PHONE_CONNECTION_REQUEST_TIME[3], PHONE_CONNECTION_REQUEST_TIME[4], PHONE_CONNECTION_REQUEST_TIME[5], PHONE_CONNECTION_REQUEST_TIME[6]);
#endif

  /* Config sych */
  if ((connection_byte[7] == USIM_PHONE_CONNECTION_COMMAND_CONFIG_SYNCH) &&
    (connection_byte[8] == USIM_PHONE_CONNECTION_COMMAND_CONFIG_SYNCH) &&
    (connection_byte[9] == USIM_PHONE_CONNECTION_COMMAND_CONFIG_SYNCH))
  {
    /* mark the config synch */
    USIM_PHONE_CONNECTION_COMMAND |= (1 << CONFIG_SYNCH_POS);
#if (IF_LOG_OUTPUT)
    printf("\r\n------------------- Config Sych -----------------------\r\n");
#endif
  }
  else if ((connection_byte[7] == USIM_PHONE_CONNECTION_COMMAND_PHONE_WIRELESS_RESET) &&
    (connection_byte[8] == USIM_PHONE_CONNECTION_COMMAND_PHONE_WIRELESS_RESET) &&
    (connection_byte[9] == USIM_PHONE_CONNECTION_COMMAND_PHONE_WIRELESS_RESET))
  {
    /* mark the USAT bit for reset NAA, let phone re-access the network */
    CONNECTION_STATE |= 0x4;
#if (IF_LOG_OUTPUT)
    printf("\r\n------------------- phone reset NAA -----------------------\r\n");
#endif
  }
#if (IF_USIM_BINARY_UPDATE)
  else if (connection_byte[7] == PHONE_CONNECTION_PIGGYBACK_UPDATE_BINARY)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n --------------------------------------------------------------------------------------------\r\n");
    printf("\r\n ------------------- USIM binary update completed, SIM_FILE_UPDATE_BINARY_TASK_QUEUE = 0x%X ---------------------------\r\n", (uint8_t)SIM_FILE_UPDATE_BINARY_TASK_QUEUE);
    printf("\r\n --------------------------------------------------------------------------------------------\r\n");
#endif
    usim_binary_update_file_id_clear(BINARY_UPDATE_FILE_ID);

    if (SIM_FILE_UPDATE_BINARY_TASK_QUEUE == 0) /* binary update task queue completed */
    {
      /* write to flash */
      start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);

      /* set the mark bit for flash data write check */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0) /* USIM0 is used by wireless SIM */
      {
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USIM0_DATA_OFFSET_POS);
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else /* USIM1 is used by wireless SIM */
      {
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USIM1_DATA_OFFSET_POS);
      }
#endif
      /* set flash write flag */
      FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
    }
  }
#endif
  else if ((connection_byte[7] == USIM_PHONE_CONNECTION_COMMAND_CONFIG_RESTORE) &&
    (connection_byte[8] == USIM_PHONE_CONNECTION_COMMAND_CONFIG_RESTORE) &&
    (connection_byte[9] == USIM_PHONE_CONNECTION_COMMAND_CONFIG_RESTORE))
  {
    /* set the mark bit to indicate user data restore */
    BLE_STOPPED_TASK_QUEUE |= (1 << BLE_STOPPED_TASK_CONFIG_RESTORE_POS);

#if (IF_LOG_OUTPUT)
    printf("\r\n------------------- USIM data restore -----------------------\r\n");
#endif
  }

  /* authentication for data transmission */
  if (connection_type  == 0x0)
  {
    /* authentication algorithm running result */
    wireless_sim_authentication_algorithm(USER_PASSWORD_RAM, SYSTEM_KEY_RAM, connection_byte + 10,
                                        AUTHENTICATE_RESULT);
  }

  return(0);
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
uint32_t connection_request_phone_ble(uint8_t connection_type, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| radio link transmission at phone side for random access request
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t connection_byte[CONNECTION_REQUEST_BYTE];
  uint32_t tx_logical_address;
  uint32_t rx_logical_address;

  if (!WIRELESS_SIM_ON_OFF_RAM)
  {
    return(2);
  }

  tx_logical_address = (USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS & 7);
  rx_logical_address = (1 << tx_logical_address);

  /* transmit byte length */
  connection_byte[0] = 25;
  /* bit0-bit2: phone logical address, bit4: default used by wireless USIM, 0: USIM0, 1: USIM1. repeat 3 times */
  connection_byte[1] = (USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS & 7);
  connection_byte[1] |= ((PHONE_WIRELESS_USED_USIM & 0x0f) << 4);
  connection_byte[2] = connection_byte[1];
  connection_byte[3] = connection_byte[1];
  /* connection request type, repeat 3 times */
  connection_byte[4] = connection_type;
  connection_byte[5] = connection_type;
  connection_byte[6] = connection_type;
  /*  */
  connection_byte[7] = DATA_CARRIER_FREQ;
  connection_byte[8] = DATA_CARRIER_FREQ;
  connection_byte[9] = DATA_CARRIER_FREQ;
  /* insert the user passwaord in the connect request message */
  for (i=0; i<KEY_LENGTH; i++)
  {
    connection_byte[i + 10] = (*(USER_PASSWORD_RAM + PASSWORD_SHUFFLE[i])) ^
                         (*(SYSTEM_KEY_RAM + SYSTEM_KEY_SHUFFLE[i]));
  }

#if (IF_CONNECTION_CC_CODED)
  /* connection request message byte length, coding rate = 1/2 */
  CC_CODED_BYTE[0] = (CONNECTION_REQUEST_BYTE * 2);
  /* cc encoding for the connection request information bytes */
  ccencoder_byte_sequence_32(connection_byte, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
#else
#if (IF_USIM_BINARY_UPDATE)
  if ((SIM_FILE_UPDATE_BINARY_TASK_QUEUE) && (BINARY_UPDATE_FILE_ID == 0))
  {
    if (!usim_binary_update_piggyback(connection_byte + 26))
    {
      connection_byte[0] = 63;
    }
  }
  else
  {
    *(connection_byte + 26) = 0;
    *(connection_byte + 27) = 0;
    *(connection_byte + 28) = 0;
    *(connection_byte + 29) = 0;
  }
#endif
#endif

  /* transmitted not successfully */
#if (IF_CONNECTION_CC_CODED)
  if (packet_radio_with_parameters_tx(CC_CODED_BYTE, FREQUENCY_CARRIER_ACCESS,
         tx_logical_address, start_time_us, time_length_us - TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US, POWER_LEVEL_CONNECTION))
#else
  if (packet_radio_with_parameters_tx(connection_byte, FREQUENCY_CARRIER_ACCESS,
         tx_logical_address, start_time_us, time_length_us - TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US, POWER_LEVEL_CONNECTION))
#endif
  {
     /* wireless link mantenance, transmission fails, return successful */
    if (connection_type == 0x11)
    {
      return(0);
    }
    /* data transmission, transmission fails, return error */
    else
    {
      return(1);
    }
  }

  PHONE_CONNECTION_REQUEST_TYPE = connection_type;
  *GLOBAL_TMP_RAM = 0;
  if (packet_radio_with_parameters_rx(GLOBAL_TMP_RAM, DATA_CARRIER_FREQ, rx_logical_address, start_time_us, time_length_us))
  {
    return(1);   /* feedback not received */
  }
  else
  {
    return(0);
  }
}
#endif

/********************************************************************************/
uint32_t phone_command_authentication_wireless(uint8_t *phone_command)
/*--------------------------------------------------------------------------------
| phone autherntication cammand 0x88 over the air
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t get_response_command_byte[6];
  uint8_t status_bytes[2] = {0x69, 0x85};
  uint8_t authentication_bytes[1] = {0x88};
  uint8_t return_connection_request;
#if (!IF_DATA_CC_CODED)
  uint32_t connection_request_start_time_ms;
#endif

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  write_bytes(1, authentication_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

  if(read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL + KEY_LENGTH + 5, PIN_DATA_PHONE, ETU_TICKS_PHONE))
  {
    return(1);
  }

  *(READ_BYTE_UICC_TERMINAL + 0) = *(phone_command + 5) + KEY_LENGTH + 5;
  *(READ_BYTE_UICC_TERMINAL + 1) = *(phone_command + 1);
  *(READ_BYTE_UICC_TERMINAL + 2) = *(phone_command + 2);
  *(READ_BYTE_UICC_TERMINAL + 3) = *(phone_command + 3);
  *(READ_BYTE_UICC_TERMINAL + 4) = *(phone_command + 4);
  *(READ_BYTE_UICC_TERMINAL + 5) = *(phone_command + 5);

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

#ifdef PIN_LED_BLUE
  nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
  nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif

#if (!IF_DATA_CC_CODED)
  connection_request_start_time_ms = NRF_RTC2->COUNTER;
#endif
  return_connection_request = connection_request_phone(0x0);
  if (return_connection_request)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n------------------- Hi, connect request time out ----------------\r\n");
#endif
    /* mark the phone-watch disconnection state */
    WIRELESS_SIM_CONNECTION_STATUS = 0;

    write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

    if (return_connection_request != SIM_LISTENING_RETURN_16M_OSC_ERROR)
    {
      stop_oscillator_16m( );
    }
    return(1);
  }
  else
  {
    /* mark the phone-watch connection state */
    WIRELESS_SIM_CONNECTION_STATUS = 1;
    /* addtional time for phone to wait for random challange data generation at watch side */
    nrf_delay_ms(RANDOM_BYTE_NUMBER_TIME_OUT_MS + 1);
  }

  /* add the wireless UICC-terminal authentication running result data */
  for (i=6; i<6 + KEY_LENGTH; i++)
  {
    *(READ_BYTE_UICC_TERMINAL + i) = AUTHENTICATE_RESULT[i-6];
  }

#if (IF_LOG_OUTPUT)
  printf_log_tx(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1);
#endif

#if (IF_DATA_CC_CODED)
  *(CC_CODED_BYTE + 0) = 128;
  ccencoder_byte_sequence_64(READ_BYTE_UICC_TERMINAL, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
  if (packet_wireless_transmit(CC_CODED_BYTE))
#else
  if (packet_wireless_transmit(READ_BYTE_UICC_TERMINAL))
#endif
  {
    // status_bytes[0] = 0x98;
    // status_bytes[1] = 0x62;
    // status_bytes[0] = 0x93;
    // status_bytes[1] = 0x0;
    // status_bytes[0] = 0x69;
    // status_bytes[1] = 0x85;
    write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

    stop_oscillator_16m( );
    return(1);
  }

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

#if (IF_DATA_CC_CODED)
  *CC_CODED_BYTE = 0;
  if (packet_wireless_receive(CC_CODED_BYTE, NRF_RTC2->COUNTER, DATA_RX_TIME_MS))
  {
    write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

    stop_oscillator_16m( );
    return(1);
  }
  ccdecoder_byte_sequence_64(CC_CODED_BYTE + 1, ENCODE_BLOCK_INDEX, READ_BYTE_UICC_TERMINAL);
#else
  if (packet_wireless_receive(READ_BYTE_UICC_TERMINAL, connection_request_start_time_ms, DATA_RX_TIME_MS))
  {
    write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

    stop_oscillator_16m( );
    return(1);
  }
#endif

#if (IF_LOG_OUTPUT)
  printf_log_rx(*READ_BYTE_UICC_TERMINAL, READ_BYTE_UICC_TERMINAL + 1);
#endif

  write_bytes(2, READ_BYTE_UICC_TERMINAL + 1, ETU_TICKS_PHONE, PIN_DATA_PHONE);

  if(read_bytes_phone(0x5, get_response_command_byte, PIN_DATA_PHONE, ETU_TICKS_PHONE))
  {
    stop_oscillator_16m( );
    return(1);
  }

  write_phone_response(*(READ_BYTE_UICC_TERMINAL + 2), READ_BYTE_UICC_TERMINAL + 3, ETU_TICKS_PHONE);

  stop_oscillator_16m( );

  return(0);
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
uint32_t phone_command_authentication_wireless_ble(uint8_t *phone_command,
         uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone autherntication cammand 0x88 over the air
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t get_response_command_byte[6];
  uint8_t status_bytes[2] = {0x69, 0x85};
  uint8_t authentication_bytes[1] = {0x88};

  /* stage 0 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage0)
  {
    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;

    write_bytes(1, authentication_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

    if(read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL + KEY_LENGTH + 5, PIN_DATA_PHONE,
      ETU_TICKS_PHONE, start_time_us, time_length_us))
    {
      return(1);
    }

    *(READ_BYTE_UICC_TERMINAL + 0) = *(phone_command + 5) + KEY_LENGTH + 5;
    *(READ_BYTE_UICC_TERMINAL + 1) = *(phone_command + 1);
    *(READ_BYTE_UICC_TERMINAL + 2) = *(phone_command + 2);
    *(READ_BYTE_UICC_TERMINAL + 3) = *(phone_command + 3);
    *(READ_BYTE_UICC_TERMINAL + 4) = *(phone_command + 4);
    *(READ_BYTE_UICC_TERMINAL + 5) = *(phone_command + 5);

    return (0);
  }
  /* stage 1 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage1)
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
    return(connection_request_phone_ble(0x0, start_time_us, time_length_us));
  }
  /* stage 2 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage2)
  {
    if (RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      /* mark the phone-watch disconnection state */
      WIRELESS_SIM_CONNECTION_STATUS = 0;

      return(1);
    }
    else
    {
      /* mark the phone-watch connection state */
      WIRELESS_SIM_CONNECTION_STATUS = 1;
    }

    /* add the wireless UICC-terminal authentication running result data */
    for (i=6; i<6 + KEY_LENGTH; i++)
    {
      *(READ_BYTE_UICC_TERMINAL + i) = AUTHENTICATE_RESULT[i-6];
    }

    return(0);
  }
  /* stage 3 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage3)
  {
#if (IF_DATA_CC_CODED)
    *(CC_CODED_BYTE + 0) = 128;
    ccencoder_byte_sequence_64(READ_BYTE_UICC_TERMINAL, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
    return (packet_wireless_transmit_ble(CC_CODED_BYTE, start_time_us, time_length_us));
#else
    return (packet_wireless_transmit_ble(READ_BYTE_UICC_TERMINAL, start_time_us, time_length_us));
#endif
  }
  /* stage 4 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage4)
  {
    if (RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

      return(1);
    }
    else
    {
      return(0);
    }
  }
  /* stage 5 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage5)
  {
#if (IF_DATA_CC_CODED)
    *CC_CODED_BYTE = 0;
    return(packet_wireless_receive_ble(CC_CODED_BYTE, start_time_us, time_length_us));
#else
    return(packet_wireless_receive_ble(READ_BYTE_UICC_TERMINAL, start_time_us, time_length_us));
#endif
  }
  /* stage 6 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage6)
  {
#if (IF_DATA_CC_CODED)
    if (RETURN_PHONE_AUTHENTICATION_WIRELESS == 1)
    {
      write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

      return(1);
    }
    else
    {
      ccdecoder_byte_sequence_64(CC_CODED_BYTE + 1, ENCODE_BLOCK_INDEX, READ_BYTE_UICC_TERMINAL);
    }
#else
    if (RETURN_PHONE_AUTHENTICATION_WIRELESS == 1)
    {
      write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);

      return(1);
    }
#endif
    write_bytes(2, READ_BYTE_UICC_TERMINAL + 1, ETU_TICKS_PHONE, PIN_DATA_PHONE);

    if(read_bytes_ble(0x5, get_response_command_byte, PIN_DATA_PHONE, ETU_TICKS_PHONE,
      start_time_us, time_length_us))
    {
      return(1);
    }
  }
  /* stage 7 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage7)
  {
    write_phone_response(*(READ_BYTE_UICC_TERMINAL + 2), READ_BYTE_UICC_TERMINAL + 3, ETU_TICKS_PHONE);
    set_event_phone_command( );
  }

  return (0);
}
#endif

/********************************************************************************/
uint32_t phone_command_get_file_data(uint16_t file_id, uint8_t *file_data)
/*--------------------------------------------------------------------------------
| get the EFs data from the watch over the air, send phone command: 0xbb
|
--------------------------------------------------------------------------------*/
{
#if (IF_SOFTDEIVE_USED)
  uint32_t i;

  /* read USIM file data from flash */
  if (IF_SOFTDEVICE_RUNNING)
  {
    for (i=0; i<ALL_USIM_EF_FLAG_SIZE; i++)
    {
      /* wireless USIM0 */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        *(file_data + i) = *((uint8_t *)(USIM0_EF_FLASH_ADDR) + i);
      }
      /* wireless USIM1 */
      else
      {
        *(file_data + i) = *((uint8_t *)(USIM1_EF_FLASH_ADDR) + i);
      }
    }
  }
  else
#endif
  {
    return(phone_command_get_file_data_blank(file_id, file_data));
  }

  return(0);
}

/********************************************************************************/
uint32_t phone_command_get_file_data_blank(uint16_t file_id, uint8_t *file_data)
/*--------------------------------------------------------------------------------
| get the EFs data from the watch over the air, send phone command: 0xbb
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t get_file_data[6 + KEY_LENGTH] = {0x5 + KEY_LENGTH, 0x0, 0xbb, 0x0, 0x0, 0xff};
  uint8_t *addr_EF_files;
  uint8_t return_connection_request;

  /* wireless USIM0 */
  if (file_id == 0xffff)
  {
    addr_EF_files = (uint8_t *)(USIM0_EF_FLASH_ADDR);
  }
  /* wireless USIM1 */
  else if (file_id == 0xffee)
  {
    addr_EF_files = (uint8_t *)(USIM1_EF_FLASH_ADDR);
  }
  else
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n++++++++++++++++++++++++++++++++++++ not supported file ID, file_id = 0x%X%X, ++++++++++++++++++++++++++++++++++++\r\n", (uint8_t)(file_id >> 8), (uint8_t)file_id);
#endif

    return (1);
  }

  get_file_data[0] = 0x5 + KEY_LENGTH;
  get_file_data[1] = 0x0;
  get_file_data[2] = 0xbb;
  get_file_data[3] = 0x0;
  get_file_data[4] = 0x0;
  get_file_data[5] = 0xff;

  get_file_data[3] = (uint8_t)(file_id >> 8);
  get_file_data[4] = (uint8_t)(file_id);

  /* prepare wireless connection */
  /* Set radio configuration parameters */
  radio_configure();
  nrf_delay_ms(3);

  /* read user password in flash */
  for (i=0; i<KEY_LENGTH; i++)
  {
    *(USER_PASSWORD_RAM + i) = *((uint8_t *)USER_PASSWORD_FLASH_ADDR + i);
    *(SYSTEM_KEY_RAM + i) = *((uint8_t *)SYSTEM_KEY_FLASH_ADDR + i);
  }

    /* turn off LED to prepare radio activity */
#ifdef PIN_LED_BLUE
  nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
  nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_YELLOW
  nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORNAGE
  nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
#ifdef PIN_LED_WHITE
  nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
  nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif

  /* connection request for wireless data transmission */
  return_connection_request = connection_request_phone(0x0);

  /* if there is no connection between phone and USIM, read the EF data from the code flash */
  if (return_connection_request)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n============= Hi, connection is not valid in phone_command_get_file_data, read USIM file data from flash =============\r\n");
#endif
    /* mark the phone-watch disconnection state */
    WIRELESS_SIM_CONNECTION_STATUS = 0;

    /* read EF files data from flash */
    if ((file_id == 0xffff) || (file_id == 0xffee))
    {
      for (i=0; i<ALL_USIM_EF_FLAG_SIZE; i++)
      {
        *(file_data + i) = *(addr_EF_files + i);
      }
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("Read EF data from falsh, to be updated......\r\n");
#endif
    }

    if (return_connection_request != SIM_LISTENING_RETURN_16M_OSC_ERROR)
    {
      stop_oscillator_16m( );
    }
  }
  /* if there is a connection between phone and USIM, get the EF data over the air */
  else
  {
    /* mark the phone-watch connection state */
    WIRELESS_SIM_CONNECTION_STATUS = 1;

    /* addtional time for phone to wait for random challange data generation at watch side */
    nrf_delay_ms(RANDOM_BYTE_NUMBER_TIME_OUT_MS + 1);
    /* add the wireless UICC-terminal authentication running result data */
    for (i=6; i<6 + KEY_LENGTH; i++)
    {
      get_file_data[i] = AUTHENTICATE_RESULT[i - 6];
    }

#if (IF_LOG_OUTPUT)
    printf_log_tx(5 + KEY_LENGTH, get_file_data + 1);
#endif

#if (IF_DATA_CC_CODED)
    *(CC_CODED_BYTE + 0) = 128;
    ccencoder_byte_sequence_64(get_file_data, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
    if (packet_wireless_transmit(CC_CODED_BYTE))
#else
    if (packet_wireless_transmit(get_file_data))
#endif
    {
      /* transmission fialed, read EF files data from flash */
      if ((file_id == 0xffff) || (file_id == 0xffee))
      {
        for (i=0; i<ALL_USIM_EF_FLAG_SIZE; i++)
        {
          *(file_data + i) = *(addr_EF_files + i);
        }
      }
    }
    /* transmitted successfully */
    else
    {
      /* re-load watch dog request register */
      NRF_WDT->RR[0] = 0x6E524635;

      /* receive SIM EF data */
      if (packet_wireless_receive(file_data, NRF_RTC2->COUNTER, DATA_RX_TIME_MS))
      {
        /* receiving fialed, read EF files data from flash */
        if ((file_id == 0xffff) || (file_id == 0xffee))
        {
          for (i=0; i<ALL_USIM_EF_FLAG_SIZE; i++)
          {
            *(file_data + i) = *(addr_EF_files + i);
          }
        }
      }
      /* received successfully */
      else
      {
        /* mark all the USIM file data is updated */
        for (i=FLAG_2FE2_OFFSET; i<ALL_USIM_EF_FLAG_SIZE; i++)
        {
          *(file_data + i) = 1;
        }

        /* write the EF data to the code flash */
        if ((file_id == 0xffff) || (file_id == 0xffee))
        {
          /* if the received ICCID is different from the ICCID saved in flash */
          if (byte_string_comparison(ALL_USIM_EF_FLAG_SIZE, file_data, addr_EF_files))
          {
            start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);

#if (IF_LOG_OUTPUT)
            printf("Different USIM data, write EF data to flash......\r\n");
#endif
          }
#if (IF_LOG_OUTPUT)
          else
          {
            printf("The same USIM data ......\r\n");
          }
#endif
        }
        else
        {
#if (IF_LOG_OUTPUT)
          printf("Write EF data to falsh, to be updated......\r\n");
#endif
        }
      }
    }

    stop_oscillator_16m( );
  }

#if (IF_LOG_OUTPUT)
  printf_log_rx(*(file_data + 0), file_data + 1);
#endif

  return(0);
}

/********************************************************************************/
uint32_t phone_command_send_data(uint8_t *send_data, uint8_t data_length, uint8_t data_type)
/*--------------------------------------------------------------------------------
| phone send data to watch, phone command: 0xcc,
| data type: data_type, for different data
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t return_connection_request;
  uint8_t data_received_confirm[6];

  *(READ_BYTE_UICC_TERMINAL + 0) = 5 + KEY_LENGTH + data_length;
  *(READ_BYTE_UICC_TERMINAL + 1) = 0x0;
  *(READ_BYTE_UICC_TERMINAL + 2) = 0xcc;
  *(READ_BYTE_UICC_TERMINAL + 3) = 0x0;
  *(READ_BYTE_UICC_TERMINAL + 4) = 0x0;
  *(READ_BYTE_UICC_TERMINAL + 5) = data_type;

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

#ifdef PIN_LED_BLUE
  nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
  nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_YELLOW
  nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
  nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
#ifdef PIN_LED_WHITE
  nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
  nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
  return_connection_request = connection_request_phone(0x0);
  if (return_connection_request) /* connection request fails */
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n============= Hi, connection is not valid in phone_command_send_data =============\r\n");
#endif
    /* mark the phone-watch disconnection state */
    WIRELESS_SIM_CONNECTION_STATUS = 0;

    if (return_connection_request != SIM_LISTENING_RETURN_16M_OSC_ERROR)
    {
      stop_oscillator_16m( );
    }

    return(1);
  }
  else /* data packet transmission */
  {
    /* mark the phone-watch connection state */
    WIRELESS_SIM_CONNECTION_STATUS = 1;
    /* addtional time for phone to wait for random challange data generation at watch side */
    nrf_delay_ms(RANDOM_BYTE_NUMBER_TIME_OUT_MS + 1);

    /* add the wireless UICC-terminal authentication running result data */
    for (i=6; i<6 + KEY_LENGTH; i++)
    {
      *(READ_BYTE_UICC_TERMINAL + i) = AUTHENTICATE_RESULT[i - 6];
    }
    /* add the phone send data */
    for (i=6 + KEY_LENGTH; i<6 + KEY_LENGTH + data_length; i++)
    {
      *(READ_BYTE_UICC_TERMINAL + i) = *(send_data + i - (6 + KEY_LENGTH));
    }

#if (IF_LOG_OUTPUT)
    printf_log_tx(5 + KEY_LENGTH + data_length, READ_BYTE_UICC_TERMINAL + 1);
#endif

#if (IF_DATA_CC_CODED)
    /* coded byte length is limited to 128 bytes, i.e. *(READ_BYTE_UICC_TERMINAL + 0) <= 63 */
    *(CC_CODED_BYTE + 0) = 128;
    ccencoder_byte_sequence_64(READ_BYTE_UICC_TERMINAL, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
    if (packet_wireless_transmit(CC_CODED_BYTE))
#else
    if (packet_wireless_transmit(READ_BYTE_UICC_TERMINAL))
#endif
    {
#if (IF_LOG_OUTPUT)
      printf("--------------- phone send data time out in phone_command_send_data --------------\r\n");
#endif
      stop_oscillator_16m( );

      return(1);
    }

    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;
    if ((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1)
    {
      if (packet_wireless_receive(GLOBAL_TMP_RAM, NRF_RTC2->COUNTER, DATA_RX_TIME_MS))
      {
#if (IF_LOG_OUTPUT)
        printf("------------- Config sych data receiving time out in phone_command_send_data --------------\r\n");
#endif
        stop_oscillator_16m( );
        return(1);
      }

      stop_oscillator_16m( );

      /* clear the flag */
      USIM_PHONE_CONNECTION_COMMAND &= (~(1 << CONFIG_SYNCH_POS));
#if (IF_LOG_OUTPUT)
      printf("-------------------------- Config sych data received ---------------------------\r\n");
      printf_log_rx(*(GLOBAL_TMP_RAM + 0), GLOBAL_TMP_RAM + 1);
#endif

      return(0);
    }
    else
    {
      if (packet_wireless_receive(data_received_confirm, NRF_RTC2->COUNTER, DATA_RX_TIME_MS))
      {
#if (IF_LOG_OUTPUT)
        printf("------------- confirm information receiving time out in phone_command_send_data --------------\r\n");
#endif
        stop_oscillator_16m( );

        return(1);
      }

      stop_oscillator_16m( );

      if ((*(data_received_confirm + 0) == 5) && (*(data_received_confirm + 1) == 0x11) && (*(data_received_confirm + 2) == 0x22))
      {
        return(0);
      }
      else
      {
        return(1);
      }
    }
  }
}

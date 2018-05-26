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
#include "comm_phone_command.h"
#include "phone_command_sim.h"
#include "time_slot.h"
#include "wireless_sim_phone.h"
#include "app_util_platform.h"
#include "nrf_nvic.h"
#include "usim_server_wireless_interface.h"
#include "comm_wireless_interface.h"
#include "comm_uicc_terminal_interface.h"
#include "flash_file_address.h"
#include "comm_initial.h"
#include "usim_server_sim_interface.h"
#include "flash_file_address.h"
#include "global_data_exchange_extern.h"
#include "flash_data_write.h"
#include "define_all_data.h"

#if (IF_DATA_CC_CODED || IF_CONNECTION_CC_CODED)
  uint8_t CC_CODED_BYTE[256];
#endif

extern uint8_t SIM_SERVER_ON_USIM0_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_OFF_USIM0_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_ON_USIM1_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_OFF_USIM1_DISPLAY_TEXT[];

static volatile uint8_t    IF_RUIM_USED = 0;

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
uint32_t main_usim_server_wireless_sim_ble_stage0(void)
/*--------------------------------------------------------------------------------
| phone command over the air between usim server and client, stage0 in side BLE stack time slot
| 0x88: authentication
| 0xbb: wireless get the SIM EF data
|
--------------------------------------------------------------------------------*/
{
  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  SIM_DATA_RECEIVE_COMMAND_START_TIME = NRF_RTC2->COUNTER;
  TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE;
#if (IF_DATA_CC_CODED)
  time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
  time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  return(0);
}
/********************************************************************************/
uint32_t main_usim_server_wireless_sim_ble_stage1(void)
/*--------------------------------------------------------------------------------
| phone command over the air between usim server and client, stage1 in side BLE stack time slot
| 0x88: authentication
| 0xbb: wireless get the SIM EF data
|
--------------------------------------------------------------------------------*/
{
  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  switch(*(USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 2))
  {
    case 0x88:
    {
      /* USIM0 is used by wireless SIM */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        set_default_usim0_etu( );
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      /* USIM1 is used by wireless SIM */
      else
      {
        set_default_usim1_etu( );
      }
#endif

      TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND;
      USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE = USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage0;
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_WIRELESS_US);

      break;
    }

    case 0xbb:
    {
      TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_DATA_SEND;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif

      break;
    }

    case 0xcc:
    {
      TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif

      break;
    }

    default:
    {
      break;
    }
  }

  /* postpone RTC2 compare0 event interrupt */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  return(0);
}

#endif

/********************************************************************************/
void main_watch_phone_sim_wireless(void)
/*--------------------------------------------------------------------------------
| phone command over the air between usim server and client
| 0x88: authentication
| 0xbb: wireless get the SIM EF data
|
--------------------------------------------------------------------------------*/
{
  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  if (received_command_sim(USIM_SERVER_COMMAND_WIRELESS_RECEIVE, ETU_TICKS_SIM))
  {
    *(USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 2) = 0x00;
  }


  switch(*(USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 2))
  {
    case 0x88:
    {
      /* USIM0 is used by wireless SIM */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        set_default_usim0_etu( );
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      /* USIM1 is used by wireless SIM */
      else
      {
        set_default_usim1_etu( );
      }
#endif

      /* for SIM logical address 7, mark the call receiver phone update */
      if ((USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS == 7) && FLAG_DEFAULT_RECEIVER)
      {
        FLAG_RECEIVE_CALL_UPDATE = 1;
      }
      /* start SIM card interface 4M clock signal */
      clock_sim_start_4m(PIN_CLOCK_SIM);
      /* start timer1 for UICC-terminal ETU */
      NRF_TIMER1->TASKS_START = 1;

      usim_server_authentication_wireless(USIM_SERVER_COMMAND_WIRELESS_RECEIVE);

      /* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );

      /* if the wireless USIM logical address is 7, let re-access network by local USIM */
      if ((USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS == 7) &&
        ((((PHONE_WIRELESS_USED_USIM & 1) == 0) && (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)) ||
        (((PHONE_WIRELESS_USED_USIM & 1) == 1) && (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1))))
      {
        CONNECTION_STATE |= 0x4;
#if (IF_LOG_OUTPUT)
      printf("\r\n-------------------------- Set phone re-access network, USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS = %d --------------------------------------------------------\r\n", USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS);
#endif
      }

      break;
    }

    case 0xbb:
      sim_command_send(USIM_SERVER_COMMAND_WIRELESS_RECEIVE, 0, 0);
      break;

    case 0xcc:
      usim_server_command_receive_data(USIM_SERVER_COMMAND_WIRELESS_RECEIVE, 0, 0);
      break;

    default:
#if (IF_LOG_OUTPUT)
      printf("\r\n**********************************************************\r\n");
      printf("Not supported USIM command,  command=%x,  skipped......\r\n", *(USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 2));
#endif
      break;
  }

  /* generate random challenge data for next use */
  random_vector_generate(AUTH_CHALL_DATA_RAM, KEY_LENGTH, NRF_RTC2->COUNTER, RANDOM_BYTE_NUMBER_TIME_OUT_MS, 0);

  /* postpone RTC2 compare0 event interrupt */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
}

/********************************************************************************/
void sim_connection_state_check(uint32_t conncetion_listen)
/*--------------------------------------------------------------------------------
| radio link connection status check at USIM server side, listening if there is an
| access resuest from USIM client side
|
--------------------------------------------------------------------------------*/
{
  static uint32_t volatile DISCONNECTION_COUNTER = 0;
  static uint32_t volatile CONNECTION_COUNTER = 0;

  /* if connection request received, and receive correct */
  if (conncetion_listen == SIM_LISTENING_RETURN_DATA_TRANSMISSION)
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
    /* clear disconnection counter */
    DISCONNECTION_COUNTER = 0;
    /* increase connection counter */
    CONNECTION_COUNTER++;

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

    /* phone command by wireless */
#if (IF_SOFTDEIVE_USED)
    if (IF_SOFTDEVICE_RUNNING)
    {
      main_usim_server_wireless_sim_ble_stage0( );
    }
    else
#endif
    {
      main_watch_phone_sim_wireless( );
    }

    return;
  }
  else if (conncetion_listen == SIM_LISTENING_RETURN_PHONE_CONNECTION)
  {
    /* clear disconnection counter */
    DISCONNECTION_COUNTER = 0;
    /* increase connection counter */
    CONNECTION_COUNTER++;

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
  }
  else
  {
    CONNECTION_COUNTER = 0;
    DISCONNECTION_COUNTER++;
  }

  if (!IF_SOFTDEVICE_RUNNING)
  {
    if ((conncetion_listen != SIM_LISTENING_RETURN_16M_OSC_ERROR) &&
      (conncetion_listen != SIM_LISTENING_RETURN_SIM_OFF))
    {
      stop_oscillator_16m( );
    }
  }

  /* disconnection counter overflow, USIM assume there is no connection between phone-USIM */
  if (DISCONNECTION_COUNTER >= SIM_DISCONNECT_TIME)
  {
    /* clear the connection and disconnection counter */
    DISCONNECTION_COUNTER = 0;
    CONNECTION_COUNTER = 0;

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

    /* from connection to disconnection */
    if ((CONNECTION_STATE & 0x3) == 2)
    {
      /* watch NAA access to the network */
      /* set USIM_SERVER_CALL_ON as 1, watch perform authenticate from SIM connect to the network */
      USIM_SERVER_CALL_ON = 1;
      /* mark the USAT bit for reset NAA, let watch access the network */
      CONNECTION_STATE |= 0x4;
    }
    /* if watch call off, since no connection between phone-watch, turn on watch call */
    if (USIM_SERVER_CALL_ON == 0)
    {
      /* watch NAA access to the network */
      /* set USIM_SERVER_CALL_ON as 1, watch perform authenticate from SIM connect to the network */
      USIM_SERVER_CALL_ON = 1;
      /* mark the USAT bit for reset NAA, let watch access the network */
      CONNECTION_STATE |= 0x4;
    }

#if (IF_LOG_OUTPUT)
    if (WIRELESS_SIM_ON_OFF_RAM)
    {
      if (!IF_SOFTDEVICE_RUNNING)
      {
        printf("==================== Wireless SIM On, Not connected, CONNECTION_STATE = %ld, USIM_SERVER_CALL_ON = %x, ==================== \r\n", CONNECTION_STATE & 0x7, USIM_SERVER_CALL_ON & 3);
      }
    }
    else
    {
      if (!IF_SOFTDEVICE_RUNNING)
      {
        printf("++++++++++++++++++++ Wireless SIM Off, Not connected, CONNECTION_STATE = %ld, USIM_SERVER_CALL_ON = %x, ++++++++++++++++++++ \r\n", CONNECTION_STATE & 0x7, USIM_SERVER_CALL_ON & 3);
      }
    }
#endif

    /* mark the phone-watch as disconnection state */
    if (WIRELESS_SIM_CONNECTION_STATUS == 1)
    {
      WIRELESS_SIM_CONNECTION_STATUS = 0;
    }
  }

  /* connection counter overflow, USIM assume there is connection between phone-USIM */
  if (CONNECTION_COUNTER)
  {
    /* clear the counter */
    CONNECTION_COUNTER = 0;

#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      printf("==================== Wireless SIM On, Connected, CONNECTION_STATE = %ld, USIM_SERVER_CALL_ON = %x ==================== \r\n", CONNECTION_STATE & 0x7, USIM_SERVER_CALL_ON & 3);
    }
#endif

    /* mark the phone-watch as connection state */
    if (WIRELESS_SIM_CONNECTION_STATUS == 0)
    {
      WIRELESS_SIM_CONNECTION_STATUS = 1;
    }
  }
}

/********************************************************************************/
uint32_t connection_listening_sim(uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| radio link connection listen at USIM server side
|
--------------------------------------------------------------------------------*/
{
  uint8_t rx_connect_request_data[CONNECTION_REQUEST_BYTE];
  uint32_t tx_logical_address;
  uint32_t rx_logical_address;
  uint32_t i;
  uint8_t connection_type;
  uint8_t connection_carrier;
  uint8_t user_password_rx[KEY_LENGTH];
  uint32_t start_time_local_radio;
  uint32_t time_length_local_radio;

  if (!IF_SOFTDEVICE_RUNNING)
  {
    if (start_oscillator_16m( ))
    {
      return (SIM_LISTENING_RETURN_16M_OSC_ERROR);
    }
  }

  if (!WIRELESS_SIM_ON_OFF_RAM)
  {
    return(SIM_LISTENING_RETURN_SIM_OFF);
  }

  rx_logical_address = 0xff;

  /* if connection request received */
#if (IF_CONNECTION_CC_CODED)
  *CC_CODED_BYTE = 0;
  if (IF_SOFTDEVICE_RUNNING)
  {
    start_time_local_radio = start_time_us;
    time_length_local_radio = time_length_us - TIMER_SLOT_CC_DECODING_TIME_US - TIMER_SLOT_CC_ENCODING_TIME_US - TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US;
  }
  else
  {
    start_time_local_radio = NRF_RTC2->COUNTER;
    time_length_local_radio = SIM_LISTEN_TIME_MS;
  }
  /* no connection request message received, return with error code */
  if (packet_radio_with_parameters_rx(CC_CODED_BYTE, FREQUENCY_CARRIER_ACCESS, rx_logical_address,
        start_time_local_radio, time_length_local_radio))
  {
    return(SIM_LISTENING_RETURN_NOT_RECEIVED);
  }
  /* if CRC check OK, give a ACK feeaback */
  if (NRF_RADIO->CRCSTATUS == 1U)
  {
    /* cc decoding */
    ccdecoder_byte_sequence_32(CC_CODED_BYTE + 1, ENCODE_BLOCK_INDEX,  rx_connect_request_data);
#else
  *GLOBAL_TMP_RAM = 0;
  if (IF_SOFTDEVICE_RUNNING)
  {
    start_time_local_radio = start_time_us;
    time_length_local_radio = time_length_us - TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US;
  }
  else
  {
    start_time_local_radio = NRF_RTC2->COUNTER;
    time_length_local_radio = SIM_LISTEN_TIME_MS;
  }
  /* no connection request message received, return with error code */
  if (packet_radio_with_parameters_rx(GLOBAL_TMP_RAM, FREQUENCY_CARRIER_ACCESS, rx_logical_address,
          start_time_local_radio, time_length_local_radio))
  {
    return(SIM_LISTENING_RETURN_NOT_RECEIVED);
  }
  /* if CRC check OK, give a ACK feeaback */
  if (NRF_RADIO->CRCSTATUS == 1U)
  {
    for (i=0; i<CONNECTION_REQUEST_BYTE; i++)
    {
      *(rx_connect_request_data + i) = *(GLOBAL_TMP_RAM + i);
    }
#endif

    /* simple receiver error check */
    if ((*(rx_connect_request_data + 1) != *(rx_connect_request_data + 2)) ||
      (*(rx_connect_request_data + 1) != *(rx_connect_request_data + 3)))
    {
      return(SIM_LISTENING_RETURN_LOGICAL_ADD_RX_ERROR);
    }
    if ((*(rx_connect_request_data + 4) != *(rx_connect_request_data + 5)) ||
      (*(rx_connect_request_data + 4) != *(rx_connect_request_data + 6)))
    {
      return(SIM_LISTENING_RETURN_CONNECT_TYPE_RX_ERROR);
    }
    if ((*(rx_connect_request_data + 7) != *(rx_connect_request_data + 8)) ||
      (*(rx_connect_request_data + 7) != *(rx_connect_request_data + 9)))
    {
      return(SIM_LISTENING_RETURN_FREQ_CARRIER_RX_ERROR);
    }

    /* if wireless USIM server is not  in Config sync listening state,
    |  perform password check */
    if (!((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1))
    {
      for (i=0; i<KEY_LENGTH; i++)
      {
        user_password_rx[PASSWORD_SHUFFLE[i]] = (*(rx_connect_request_data + 10 + i)) ^
                                           (*(SYSTEM_KEY_RAM + SYSTEM_KEY_SHUFFLE[i]));
      }
      /* wireless UICC-terminal interface password check. if fails,
      |  return with error code SIM_LISTENING_RETURN_PASSWD_RX_ERROR */
      if (byte_string_comparison(KEY_LENGTH, USER_PASSWORD_RAM, user_password_rx))
      {
        return(SIM_LISTENING_RETURN_PASSWD_RX_ERROR);
      }
    }

    /* save the connection type */
    connection_type = rx_connect_request_data[4];
    /* save the connection frequency carrier */
    connection_carrier = rx_connect_request_data[7];

    /* save the logical address to send ACK message */
    tx_logical_address = (rx_connect_request_data[1] & 7);
    /* save the remote wireless USIM used */
    PHONE_WIRELESS_USED_USIM = (rx_connect_request_data[1] >> 4);

    /* The default initial phone logical address is 0, if a phone use logical address 0
    |  to request access, allocate a different logical address between 1 : 7 */
    if ((rx_connect_request_data[1] & 7) == 0)
    {
      rx_connect_request_data[1] = phone_logical_address_search( );
    }

    /* put the USIM existence status */
    /* if USIM0 existed */
    if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
    {
      /* mark USIM0 existed, pit bit1=1 */
      PHONE_WIRELESS_USED_USIM |= 0x2;
    }
    else
    {
      /* mark USIM0 not existed, put bit1=0 */
      PHONE_WIRELESS_USED_USIM &= 0xfd;
      /* if request wireless USIM0, chahnged to wireless USIM1 */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
      {
        PHONE_WIRELESS_USED_USIM |= 1;
      }
    }
    /* if USIM1 existed */
    if (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1)
    {
      /* mark USIM1 existed, pit bit1=2 */
      PHONE_WIRELESS_USED_USIM |= 0x4;
    }
    else
    {
      /* mark USIM1 not existed, pit bit2=0 */
      PHONE_WIRELESS_USED_USIM &= 0xfb;
      /* if request wireless USIM1, chahnged to wireless USIM0 */
      if ((PHONE_WIRELESS_USED_USIM & 1) == 1)
      {
        PHONE_WIRELESS_USED_USIM &= 0xfe;
      }
    }

    /* save the PHONE_WIRELESS_USED_USIM to confirm */
    rx_connect_request_data[1] &= 0x0F;
    rx_connect_request_data[1] |= (PHONE_WIRELESS_USED_USIM << 4);
    rx_connect_request_data[2] = rx_connect_request_data[1];
    rx_connect_request_data[3] = rx_connect_request_data[1];

    /* search the best carrier frequency for data transmission */
    if (IF_SOFTDEVICE_RUNNING)
    {
      rx_connect_request_data[4] = radio_carrier_search_ble( );
    }
    else
    {
      rx_connect_request_data[4] = radio_carrier_search(CARRIER_FREQ_SEARCH);
    }
    rx_connect_request_data[5] = rx_connect_request_data[4];
    rx_connect_request_data[6] = rx_connect_request_data[4];
    /* save the carrier frequency for data transmission between SIM-Phone */
    DATA_CARRIER_FREQ = rx_connect_request_data[4];

    /* feedback to confirm config sych */
    if ((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1)
    {
      rx_connect_request_data[7] = USIM_PHONE_CONNECTION_COMMAND_CONFIG_SYNCH;
      rx_connect_request_data[8] = rx_connect_request_data[7];
      rx_connect_request_data[9] = rx_connect_request_data[7];
    }
    else if ((USIM_PHONE_CONNECTION_COMMAND >> REMOTE_TASK_CONFIG_RESTORE_POS) & 1)
    {
      USIM_PHONE_CONNECTION_COMMAND &= (~(1 << REMOTE_TASK_CONFIG_RESTORE_POS));
      rx_connect_request_data[7] = USIM_PHONE_CONNECTION_COMMAND_CONFIG_RESTORE;
      rx_connect_request_data[8] = rx_connect_request_data[7];
      rx_connect_request_data[9] = rx_connect_request_data[7];
    }
#if (IF_USIM_BINARY_UPDATE)
    /* USIM binary update phone wireless connection piggyback check */
    else if ((rx_connect_request_data[0] == 63) && (rx_connect_request_data[26] == rx_connect_request_data[27]))
    {
      /* piggyback data is EF binary update */
      if (rx_connect_request_data[26] == PHONE_CONNECTION_PIGGYBACK_UPDATE_BINARY)
      {
        usim_binary_update_wireless_connection(rx_connect_request_data + 26);
        rx_connect_request_data[7] = PHONE_CONNECTION_PIGGYBACK_UPDATE_BINARY;
        rx_connect_request_data[8] = rx_connect_request_data[7];
        rx_connect_request_data[9] = rx_connect_request_data[7];
      }
    }
#endif
    else
    {
      rx_connect_request_data[7] = 0;
      rx_connect_request_data[8] = 0;
      rx_connect_request_data[9] = 0;
    }

    /* get authentication challenge data for phone connection request */
    for (i=0; i<KEY_LENGTH; i++)
    {
      *(rx_connect_request_data + 10 + i) = *(AUTH_CHALL_DATA_RAM + i);
    }
    /* transmit byte length */
    *(rx_connect_request_data + 0) = 25;

#if (IF_CONNECTION_CC_CODED)
    /* connection response information byte length, coding rate = 1/2 */
    *(CC_CODED_BYTE + 0) = (CONNECTION_REQUEST_BYTE * 2);
    /* cc encoding for connection response message trfansmission */
    ccencoder_byte_sequence_32(rx_connect_request_data, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
#endif

    /* if connection response transmission successfully, return with successful code */
    if (IF_SOFTDEVICE_RUNNING)
    {
      start_time_local_radio = start_time_us;
      time_length_local_radio = time_length_us;
    }
    else
    {
      start_time_local_radio = NRF_RTC2->COUNTER;
      time_length_local_radio = PHONE_REQUEST_TX_TIME_MS;
    }

#if (IF_CONNECTION_CC_CODED)
    if (!packet_radio_with_parameters_tx(CC_CODED_BYTE, connection_carrier,
             tx_logical_address, start_time_local_radio, time_length_local_radio, POWER_LEVEL_ACK))
    {
#else
    if (!packet_radio_with_parameters_tx(rx_connect_request_data, connection_carrier,
           tx_logical_address, start_time_local_radio, time_length_local_radio, POWER_LEVEL_ACK))
    {
#endif
      USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS = (rx_connect_request_data[1] & 7);

      /* connection request for wireless link mantenance, connection_type = 0x11 */
      if (connection_type == 0x11)
      {
        return(SIM_LISTENING_RETURN_PHONE_CONNECTION);
      }
      /* connection request for data transmission, connection_type = 0x0 */
      else
      {
        /* authentication algorithm running result */
        wireless_sim_authentication_algorithm(USER_PASSWORD_RAM, SYSTEM_KEY_RAM, rx_connect_request_data + 10,
                                          AUTHENTICATE_RESULT);
        return(SIM_LISTENING_RETURN_DATA_TRANSMISSION);
      }
    }
    /* connection response message transmission fails, return with error code */
    else
    {
      return(SIM_LISTENING_RETURN_CONFIRM_TX_ERROR);
    }
  }
  /* CRC check error, return with error code */
  else
  {
    return(SIM_LISTENING_RETURN_NOT_RECEIVED);
  }
}

#if (IF_LOG_OUTPUT)
/********************************************************************************/
void connection_listening_sim_return_message(uint32_t return_code_usim_listening)
/*--------------------------------------------------------------------------------
| print the wireless USIM connection request listening return message
|
--------------------------------------------------------------------------------*/
{
  if (return_code_usim_listening == SIM_LISTENING_RETURN_DATA_TRANSMISSION)
  {
    printf("\r\ndata send request: --------------------- BLE stautus = %d, SIM data carrier frequency = %d, phone logical address = %d, PHONE_WIRELESS_USED_USIM = %d ---------------------\r\n", IF_SOFTDEVICE_RUNNING, DATA_CARRIER_FREQ, USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS, PHONE_WIRELESS_USED_USIM);
  }
  else if (return_code_usim_listening == SIM_LISTENING_RETURN_PHONE_CONNECTION)
  {
    printf("\r\nphone connection request: --------------------- BLE stautus = %d, SIM data carrier frequency = %d, phone logical address = %d, PHONE_WIRELESS_USED_USIM = %d ---------------\r\n", IF_SOFTDEVICE_RUNNING, DATA_CARRIER_FREQ, USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS, PHONE_WIRELESS_USED_USIM);
  }
  else if (return_code_usim_listening == SIM_LISTENING_RETURN_LOGICAL_ADD_RX_ERROR)
  {
    printf("\r\n------------------------- BLE stautus = %d, phone logical address receive error in connection_listening_SIM -------------------------------------\r\n", IF_SOFTDEVICE_RUNNING);
  }
  else if (return_code_usim_listening == SIM_LISTENING_RETURN_CONNECT_TYPE_RX_ERROR)
  {
    printf("\r\n------------------------- BLE stautus = %d, phone connection request type receive error in connection_listening_SIM ------------------------------\r\n", IF_SOFTDEVICE_RUNNING);
  }
  else if (return_code_usim_listening == SIM_LISTENING_RETURN_FREQ_CARRIER_RX_ERROR)
  {
    printf("\r\n------------------------- BLE stautus = %d, phone connection request frequency carrier error in connection_listening_SIM -------------------------\r\n", IF_SOFTDEVICE_RUNNING);
  }
  else if (return_code_usim_listening == SIM_LISTENING_RETURN_PASSWD_RX_ERROR)
  {
    printf("\r\n--------------------------------------- BLE stautus = %d, password check error in connection_listening_SIM ---------------------------------------\r\n", IF_SOFTDEVICE_RUNNING);
  }
  else if (return_code_usim_listening == SIM_LISTENING_RETURN_CONFIRM_TX_ERROR)
  {
    printf("\r\n------------------------- BLE stautus = %d, return confirm message send error in connection_listening_SIM ---------------------------------------\r\n", IF_SOFTDEVICE_RUNNING);
  }
}
#endif

/********************************************************************************/
uint32_t radio_carrier_search(uint32_t max_serach_time_ms)
/*--------------------------------------------------------------------------------
| search a radio carrier according the received RSSI
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t threshold = CARRIER_FREQ_THRE;
  uint8_t default_carrier_frequency = 83;
  uint32_t measured_RSSI;
  static uint8_t frequency_bin = 0;
  uint32_t initial_timer;

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;

  while (1)
  {
    for (i=0; i<83; i++)
    {
      frequency_bin++;
      if (frequency_bin == 83)
      {
        frequency_bin = 0;
      }

      measured_RSSI = radio_rssi_measurement(FREQ_BIN_PATTEN[frequency_bin]);
      if (measured_RSSI > threshold)
      {
        return(FREQ_BIN_PATTEN[frequency_bin]);
      }
    }

    /* if maximal search reached, break out, use default carrier frequency */
    if (((NRF_RTC2->COUNTER - initial_timer) > max_serach_time_ms) || (NRF_RTC2->COUNTER < initial_timer))
    {
      break;
    }
    /* decrease the threshold, search again */
    else
    {
      threshold -= 3;
      if (threshold < 40)
      {
        threshold = 40;
      }
    }
  }

  /* use default carrier frequency */
  return(default_carrier_frequency);
}

/********************************************************************************/
uint32_t phone_logical_address_search(void)
/*--------------------------------------------------------------------------------
| search logical address for wireless USIM interface
|
--------------------------------------------------------------------------------*/
{
  static uint32_t logical_address_counter = 0;

  logical_address_counter++;

  /* logical address 1, ..., 7 */
  return (logical_address_counter % 7 + 1);

}

/********************************************************************************/
uint32_t radio_carrier_search_ble(void)
/*--------------------------------------------------------------------------------
| search a radio carrier according the received RSSI, inside BLE stack time slot
|
--------------------------------------------------------------------------------*/
{
  static uint32_t frequency_bin = 0;

  frequency_bin++;

  /* frequecny bin index 0,...,82 */
  return(FREQ_BIN_PATTEN[frequency_bin % 83]);
}

/********************************************************************************/
uint32_t received_command_sim(uint8_t *bytes_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| USIM server receive the phone command over the air
|
--------------------------------------------------------------------------------*/
{

#if (IF_DATA_CC_CODED)
  if (packet_wireless_receive(CC_CODED_BYTE, NRF_RTC2->COUNTER, DATA_RX_TIME_MS))
  {
    return(1);
  }
  ccdecoder_byte_sequence_64(CC_CODED_BYTE + 1, ENCODE_BLOCK_INDEX, bytes_command);
#else
  if (packet_wireless_receive(bytes_command, NRF_RTC2->COUNTER, DATA_RX_TIME_MS))
  {
    return(1);
  }
#endif

#if (IF_LOG_OUTPUT)
  if (!IF_SOFTDEVICE_RUNNING)
  {
    printf_log_rx(*(bytes_command + 0), bytes_command + 1);
  }
#endif

  /* wireless UICC-terminal interface authentication check */
  if (byte_string_comparison(KEY_LENGTH, bytes_command + 6, AUTHENTICATE_RESULT) && (!((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1)))
  {
#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      printf("\r\n------------------------- authentication error in received_command_sim -------------------------\r\n");
      printf("transmitted: ");
      printf_log_tx(KEY_LENGTH, AUTHENTICATE_RESULT);
    }
#endif

    return(1);
  }

  return(0);
}

/********************************************************************************/
uint32_t usim_server_authentication_wireless(uint8_t *phone_command)
/*--------------------------------------------------------------------------------
| wireless SIM authentication phone command of server side, 0x88
|
--------------------------------------------------------------------------------*/
{
  uint8_t get_response_command[5] = {0x0, 0xc0, 0x0, 0x0, 0x35};
  uint8_t response_length_byte[3];

  write_bytes(5, phone_command + 1, ETU_TICKS_SIM, PIN_DATA_SIM);

  if(read_bytes_sim(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
  {
    return(1);
  }

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  write_bytes(*(phone_command + 5), phone_command + KEY_LENGTH + 6, ETU_TICKS_SIM, PIN_DATA_SIM);

  if(read_bytes_sim(2, response_length_byte, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
  {
    return(1);
  }
  if (!((*(response_length_byte + 1) == 0x9f) || (*(response_length_byte + 1) == 0x61)))
  {
#if (IF_LOG_OUTPUT)
    printf("Authentication error......\r\n");
#endif

    return(1);
  }

  get_response_command[4]  = *(response_length_byte + 2);
  write_bytes(5, get_response_command, ETU_TICKS_SIM, PIN_DATA_SIM);

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  if(read_bytes_sim(get_response_command[4] + 3, READ_BYTE_UICC_TERMINAL + 1, PIN_DATA_SIM, ETU_TICKS_SIM, 0))
  {
    return(1);
  }

  *(READ_BYTE_UICC_TERMINAL + 0) = *(response_length_byte + 2) + 2;
  *(READ_BYTE_UICC_TERMINAL + 1) = *(response_length_byte + 1);
  *(READ_BYTE_UICC_TERMINAL + 2) = *(response_length_byte + 2);

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

#if (IF_DATA_CC_CODED)
  *(CC_CODED_BYTE + 0) = 128;
  ccencoder_byte_sequence_64(READ_BYTE_UICC_TERMINAL, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
  if (packet_wireless_transmit(CC_CODED_BYTE))
#else
  if (packet_wireless_transmit(READ_BYTE_UICC_TERMINAL))
#endif
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n---------------- Time out for data package transmission in usim_server_authentication_wireless --------------------\r\n");
#endif

    return(1);
  }

#if (IF_LOG_OUTPUT)
  printf("========================= returned from usim_server_authentication_wireless =========================\r\n");
#endif

  return(0);
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
uint32_t usim_server_authentication_wireless_ble(uint8_t *phone_command, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| wireless SIM authentication phone command of server side, 0x88
|
--------------------------------------------------------------------------------*/
{
  uint8_t get_response_command[5] = {0x0, 0xc0, 0x0, 0x0, 0x35};
#if (IF_DATA_CC_CODED)
  uint32_t i;
#endif

  /* stage 0 */
  if (USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE == USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage0)
  {
    /* start timer1, for ETU timing */
    NRF_TIMER1->TASKS_START = 1;
    /* start sim clock signal */
    clock_sim_start_4m(PIN_CLOCK_SIM);

    write_bytes(5, phone_command + 1, ETU_TICKS_SIM, PIN_DATA_SIM);

    if(read_bytes_ble(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, start_time_us, time_length_us))
    {
      /* set SWI3_EGU3 to stop BLE softdevice */
      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
      FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;

      /* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );

      return(1);
    }
    if (*(READ_BYTE_UICC_TERMINAL +1) !=0x88)
    {

    }

    write_bytes(*(phone_command + 5), phone_command + KEY_LENGTH + 6, ETU_TICKS_SIM, PIN_DATA_SIM);

    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;

    SIM_READ_BYTES_NUMBER = 2;
    WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = 0xFF;
    sd_nvic_SetPendingIRQ(GPIOTE_IRQn);

    return (0);
  }
  /* stage 1 */
  if (USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE == USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage1)
  {
    get_response_command[4]  = *(READ_BYTE_UICC_TERMINAL + 252);
    write_bytes(5, get_response_command, ETU_TICKS_SIM, PIN_DATA_SIM);

    if(read_bytes_ble(get_response_command[4] + 3, READ_BYTE_UICC_TERMINAL + 1, PIN_DATA_SIM, ETU_TICKS_SIM, start_time_us, time_length_us))
    {
      /* set SWI3_EGU3 to stop BLE softdevice */
      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
      FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;

      /* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
      return(1);
    }

    *(READ_BYTE_UICC_TERMINAL + 0) = *(READ_BYTE_UICC_TERMINAL + 252) + 2;
    *(READ_BYTE_UICC_TERMINAL + 1) = *(READ_BYTE_UICC_TERMINAL + 251);
    *(READ_BYTE_UICC_TERMINAL + 2) = *(READ_BYTE_UICC_TERMINAL + 252);

#if (IF_DATA_CC_CODED)
    *(CC_CODED_BYTE + 0) = 128;
    ccencoder_byte_sequence_64(READ_BYTE_UICC_TERMINAL, ENCODE_BLOCK_INDEX, CC_CODED_BYTE + 1);
    for (i=0; i<129; i++)
    {
      *(READ_BYTE_UICC_TERMINAL  + i) = *(CC_CODED_BYTE + i);
    }
#endif

    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;

    /* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );
    /* set USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE, to send the USIM authentication results */
    USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE = USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage2;
  }

  return(0);
}
#endif

/********************************************************************************/
uint32_t sim_command_send(uint8_t *phone_command, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| USIM server send SIM EF file content to phone over the air
|
--------------------------------------------------------------------------------*/
{
  uint16_t file_ID = *(phone_command + 3) * 0x100 + *(phone_command + 4);

  switch(file_ID)
  {
    /* wireless USIM0 */
    case 0xffff:
    {
#if (IF_LOG_OUTPUT)
      if (IF_SOFTDEVICE_RUNNING == 0)
      {
        printf_log_tx(*(USIM0_EF_DATA_RAM + 0), USIM0_EF_DATA_RAM + 1);
      }
#endif

      if (IF_SOFTDEVICE_RUNNING)
      {
        return(packet_wireless_transmit_ble(USIM0_EF_DATA_RAM, start_time_us, time_length_us));
      }
      else
      {
        return(packet_wireless_transmit(USIM0_EF_DATA_RAM));
      }

      break;
    }

    /* wireless USIM1 */
    case 0xffee:
    {
#if (IF_LOG_OUTPUT)
      if (IF_SOFTDEVICE_RUNNING == 0)
      {
        printf_log_tx(*(USIM1_EF_DATA_RAM + 0), USIM1_EF_DATA_RAM + 1);
      }
#endif

      if (IF_SOFTDEVICE_RUNNING)
      {
        return(packet_wireless_transmit_ble(USIM1_EF_DATA_RAM, start_time_us, time_length_us));
      }
      else
      {
        return(packet_wireless_transmit(USIM1_EF_DATA_RAM));
      }

      break;
    }

    default:
    {
#if (IF_LOG_OUTPUT)
      printf("------------------------ Wrong file ID, error........------------------------\r\n");
      printf("------------------------------ file ID = %x, ------------------------------\r\n", file_ID);
#endif
      break;
    }
  }

  return(0);
}

/********************************************************************************/
uint32_t sim_file_update(uint32_t read_file_id, uint8_t *update_bytes_all_file)
/*--------------------------------------------------------------------------------
| update local USIM EF read_file_ID content data
|
--------------------------------------------------------------------------------*/
{
  uint8_t read_binay_command[5] = {0x0, 0xb0, 0x0, 0x0, 0x0};
  uint32_t record_length = 0;
  uint32_t record_length_mandatory = 28;
  uint8_t file_size = 0;
  uint32_t file_offset = 0;
  uint32_t flag_offset = 0;
  uint32_t i;

  /* for 0x6f42, the read command is read record */
  if (read_file_id == 0x6f42)
  {
    read_binay_command[0] = 0x0;
    read_binay_command[1] = 0xb2;
    read_binay_command[2] = 0x1;
    read_binay_command[3] = 0x4;
    read_binay_command[4] = 0x00;
  }

  /* USIM command select, prepare for EF data read */
  switch (read_file_id)
  {
    case 0x2fe2:
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_file_3f00_selected_check( ))
      {
        return(1);
      }
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if ((SELECTED_FILE_SIM != 0x3f00) && ((SELECTED_FILE_SIM >> 16) != 0x3f00))
      {
        if(sim_command_select(0x3f00, ETU_TICKS_SIM))
        {
          return(1);
        }

        SELECTED_FILE_SIM = 0x3f00;
        if (PIN_DATA_SIM == PIN_DATA_SIM0)
        {
          SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
        }
#if (PIN_VCC_SIM1 != PIN_NULL)
        else if (PIN_DATA_SIM == PIN_DATA_SIM1)
        {
          SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
        }
#endif
      }
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_command_select(0x2fe2, ETU_TICKS_SIM))
      {
        return(1);
      }

      SELECTED_FILE_SIM = 0x3f002fe2;
      if (PIN_DATA_SIM == PIN_DATA_SIM0)
      {
        SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else if (PIN_DATA_SIM == PIN_DATA_SIM1)
      {
        SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
      }
#endif

      break;
    }

    case 0x2f05:
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_file_3f00_selected_check( ))
      {
        return(1);
      }
      if ((SELECTED_FILE_SIM != 0x3f00) && ((SELECTED_FILE_SIM >> 16) != 0x3f00))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        if(sim_command_select(0x3f00, ETU_TICKS_SIM))
        {
          return(1);
        }

        SELECTED_FILE_SIM = 0x3f00;
        if (PIN_DATA_SIM == PIN_DATA_SIM0)
        {
          SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
        }
#if (PIN_VCC_SIM1 != PIN_NULL)
        else if (PIN_DATA_SIM == PIN_DATA_SIM1)
        {
          SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
        }
#endif
      }
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_command_select(0x3f002f05, ETU_TICKS_SIM))
      {
        return(1);
      }

      SELECTED_FILE_SIM = 0x3f002f05;
      if (PIN_DATA_SIM == PIN_DATA_SIM0)
      {
        SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else if (PIN_DATA_SIM == PIN_DATA_SIM1)
      {
        SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
      }
#endif

      break;
    }

    case 0x4f20:
    {
      if (((SELECTED_FILE_SIM != 0x7fff5f3b) && ((SELECTED_FILE_SIM >> 16) != 0x7fff5f3b)))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        if (sim_file_7fff_selected_check( ))
        {
          return(1);
        }
        if (sim_command_select(0x5f3b, ETU_TICKS_SIM))
        {
          return(1);
        }
      }
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_command_select(0x4f20, ETU_TICKS_SIM))
      {
        return(1);
      }

      SELECTED_FILE_SIM = 0x7fff5f3b4f20;
      if (PIN_DATA_SIM == PIN_DATA_SIM0)
      {
        SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else if (PIN_DATA_SIM == PIN_DATA_SIM1)
      {
        SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
      }
#endif

      break;
    }

    case 0x4f52:
    {
      if (((SELECTED_FILE_SIM != 0x7fff5f3b) && ((SELECTED_FILE_SIM >> 16) != 0x7fff5f3b)))
      {
        if (sim_file_7fff_selected_check( ))
        {
          return(1);
        }
        if (sim_command_select(0x5f3b, ETU_TICKS_SIM))
        {
          return(1);
        }
      }

      if (sim_command_select(0x4f52, ETU_TICKS_SIM))
      {
        return(1);
      }

      SELECTED_FILE_SIM = 0x7fff5f3b4f52;
      if (PIN_DATA_SIM == PIN_DATA_SIM0)
      {
        SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else if (PIN_DATA_SIM == PIN_DATA_SIM1)
      {
        SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
      }
#endif

      break;
    }

    default:
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_file_7fff_selected_check( ))
      {
      return(1);
      }
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if (sim_command_select(read_file_id, ETU_TICKS_SIM))
      {
        return(1);
      }

      SELECTED_FILE_SIM = 0x7fff0000;
      SELECTED_FILE_SIM |= (read_file_id & 0xffff);
      if (PIN_DATA_SIM == PIN_DATA_SIM0)
      {
        SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else if (PIN_DATA_SIM == PIN_DATA_SIM1)
      {
        SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
      }
#endif

      break;
    }
  }

  /* get the offset of EF */
  get_usim_file_position_index(read_file_id, &file_size, &file_offset, &flag_offset);

  /* set the EF data read length */
  if (read_file_id == 0x6f42)
  {
    record_length = *(READ_BYTE_UICC_TERMINAL +  9);
    read_binay_command[4] = record_length;
  }
  else
  {
    read_binay_command[4] = file_size;
  }

  /* USIM command to read the EF data */
  write_bytes(5, read_binay_command, ETU_TICKS_SIM, PIN_DATA_SIM);
  if (read_bytes_sim(read_binay_command[4] + 3, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 0))
  {
    return(1);
  }

  /* update the read EF data to RAM */
  switch (read_file_id)
  {
    case 0x2fe2:
    case 0x2f05:
    case 0x6f07:
    case 0x6f08:
    case 0x6f09:
    case 0x6f31:
    case 0x6f78:
    case 0x6f7e:
    case 0x6f73:
    case 0x6f7b:
    case 0x6fad:
    case 0x6f5c:
    case 0x6f5b:
    case 0x6fc4:
    case 0x6f56:
    case 0x4f20:
    case 0x4f52:
    {
      *(update_bytes_all_file + flag_offset) = 1;
      for (i=0; i<file_size; i++)
      {
        *(update_bytes_all_file + file_offset + i) = *(READ_BYTE_UICC_TERMINAL + i + 2);
      }

      break;
    }

    case 0x6f42:
    {
      *(update_bytes_all_file + FLAG_6F42_OFFSET) = 1;
      for (i=0; i<record_length_mandatory; i++)
      {
        *(update_bytes_all_file + EF_6F42_OFFSET + (file_size - record_length_mandatory) + i) =
          *(READ_BYTE_UICC_TERMINAL + (record_length - record_length_mandatory) + i + 2);
      }

      break;
    }

    default:
    {
#if (IF_LOG_OUTPUT)
      printf("error file ID for USIM file datat update, read_file_id = 0x%X%X......\r\n", (uint8_t)(read_file_id >> 8), (uint8_t)(read_file_id));
#endif
      return(1);
    }
  }

  /* for RUIM card (CDMA system, china telecom, ICCID: 898603.../898606.../898611...) */
  /* CMCC: 898600.../898602.../898604.../898607...*/
  /* CU:   898601...*/
  if (read_file_id == 0x2fe2)
  {
    if ((*(READ_BYTE_UICC_TERMINAL + 4) == 0x30) || (*(READ_BYTE_UICC_TERMINAL + 4) == 0x60) || (*(READ_BYTE_UICC_TERMINAL + 4) == 0x11))
    {
      IF_RUIM_USED = 1;
    }
  }
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  return(0);
}

/********************************************************************************/
uint32_t usim_server_command_receive_data(uint8_t *received_data, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| watch receive the data send from phone wireless
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  switch(*(received_data + 5))
  {
    case CONFIG_SYCH_REQUEST_DATA_TYPE:
    {
      *(GLOBAL_TMP_RAM + 0) = USER_CONFIG_ALL_BYTES_LENGTH;
      for (i=0; i<(*(GLOBAL_TMP_RAM + 0)); i++)
      {
        *(GLOBAL_TMP_RAM + i + 1) = *((uint8_t *)USER_CONFIG_FLASH_ADDR + i);
      }

      if (IF_SOFTDEVICE_RUNNING)
      {
        if (packet_wireless_transmit_ble(GLOBAL_TMP_RAM, start_time_us, time_length_us))
        {
          return(1);
        }
      }
      else
      {
        if (packet_wireless_transmit(GLOBAL_TMP_RAM))
        {
          return(1);
        }
      }

      /*clear the the flag */
      USIM_PHONE_CONNECTION_COMMAND &= (~(1 << CONFIG_SYNCH_POS));

      /* update idle text for USIM server mode side */
      if ((PHONE_LOCAL_USED_USIM & 0xF0) == 0)
      {
        /* set idle mode text to indicate Wireless SIM ON/OFF state */
        if (WIRELESS_SIM_ON_OFF_RAM)
        {
          if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_ON_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM0_DISPLAY_TEXT);
          }
          else if (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_ON_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM1_DISPLAY_TEXT);
          }
        }
        else
        {
          if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_OFF_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM0_DISPLAY_TEXT);
          }
          else if (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_OFF_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM1_DISPLAY_TEXT);
          }
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  return(0);
}

/********************************************************************************/
uint32_t usim_files_data_read_flash_write_usim(uint8_t usim_no, uint8_t *usim_files_all_data)
/*--------------------------------------------------------------------------------
| second USIM (not used by phone) files update to flash
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  /* all the USIM file ID for EF data update */
  uint32_t file_id_update[18] = {0x2fe2, 0x2f05, 0x4f20, 0x4f52, 0x6f07, 0x6f08, 0x6f09, 0x6f31, 0x6f78,
                                 0x6f7e, 0x6f73, 0x6f7b, 0x6fad, 0x6f5c, 0x6f5b, 0x6fc4, 0x6f56, 0x6f42};

  /* USIM pin set */
  if (usim_no == 0)
  {
    set_default_usim0_etu( );
  }
#if (PIN_VCC_SIM1 != PIN_NULL)
  else if (usim_no == 1)
  {
    set_default_usim1_etu( );
  }
#endif
  else
  {
#if (IF_LOG_OUTPUT)
#if (PIN_VCC_SIM1 != PIN_NULL)
    printf("++++++++++++++++++++++++ Wrong USIM card number, it can only be 0 or 1, input usim_no = %d ++++++++++++++++++++++++\r\n", usim_no);
#else
    printf("++++++++++++++++++++++++ Wrong USIM card number, it can only be 0, input usim_no = %d ++++++++++++++++++++++++\r\n", usim_no);
#endif
#endif
    return(1);
  }

  /* USIM EF data update */
  for (i=0; i<18; i++)
  {
    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    if (sim_file_update(file_id_update[i], usim_files_all_data))
    {
      return (1);
    }
  }

  /* all files data updated check */
  for (i=FLAG_2FE2_OFFSET; i<FLAG_6F42_OFFSET + 1; i++)
  {
    if (*(usim_files_all_data + i) == 0)
    {
      return (1);
    }
  }

  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
  /* save the data to flash */
  if (usim_no == 0)
  {
    if (byte_string_comparison(FILE_SIZE_2FE2, usim_files_all_data + EF_2FE2_OFFSET, (uint8_t *)USIM0_EF_2FE2_FLASH_ADDR))
    {
      start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
#if (IF_LOG_OUTPUT)
      printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
      printf("Different USIM0 data, write EF data to flash......\r\n");
      printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
#endif
    }
#if (IF_LOG_OUTPUT)
    else
    {
      printf("============================================================================\r\n");
      printf("The same USIM0 data ......\r\n");
      printf("============================================================================\r\n");
    }
#endif
  }
  else if (usim_no == 1)
  {
    if (byte_string_comparison(FILE_SIZE_2FE2, usim_files_all_data + EF_2FE2_OFFSET, (uint8_t *)USIM1_EF_2FE2_FLASH_ADDR))
    {
      start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
#if (IF_LOG_OUTPUT)
      printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
      printf("Different USIM1 data, write EF data to flash......\r\n");
      printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
#endif
    }
#if (IF_LOG_OUTPUT)
    else
    {
      printf("============================================================================\r\n");
      printf("The same USIM1 data ......\r\n");
      printf("============================================================================\r\n");
    }
#endif
  }

  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  return (0);
}

/********************************************************************************/
uint32_t usim_files_data_read_flash_write_ruim(uint8_t usim_no, uint8_t *usim_files_all_data)
/*--------------------------------------------------------------------------------
| RUIM data flash wrtie, to be updated
|
--------------------------------------------------------------------------------*/
{
  return(0);
}
/********************************************************************************/
uint32_t usim_files_data_read_flash_write(uint8_t usim_no, uint8_t *usim_files_all_data)
/*--------------------------------------------------------------------------------
| second USIM (not used by phone) files update to flash
|
--------------------------------------------------------------------------------*/
{
  /* for RUIM card (CDMA system, china telecom, ICCID: 898603/898606/898611) */
  if (IF_RUIM_USED == 1)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n==================================================================================================================\r\n");
    printf("++++++++++++++++++++++++++++++++++++++ Skip USIM data update! ++++++++++++++++++++++++++++++++++++++\r\n");
    printf_log_rx(FILE_SIZE_2FE2, (uint8_t *)USIM0_EF_DATA_RAM + EF_2FE2_OFFSET);
    printf("\r\n==================================================================================================================\r\n");
#endif
    return(usim_files_data_read_flash_write_ruim(usim_no, usim_files_all_data));
  }
  else
  {
    return(usim_files_data_read_flash_write_usim(usim_no, usim_files_all_data));
  }
}


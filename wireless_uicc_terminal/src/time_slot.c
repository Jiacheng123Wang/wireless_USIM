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
#include "define_config.h"
#include <stdint.h>
#include <stdbool.h>
#include "define_config.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_nvic.h"
#include "comm_phone_command.h"
#include "phone_wireless_interface.h"
#include "time_slot.h"
#include "comm_initial.h"
#include "usim_server_command_sim.h"
#include "usim_server_wireless_interface.h"
#include "comm_wireless_interface.h"
#include "comm_uicc_terminal_interface.h"
#include "wireless_sim_phone.h"
#include "usim_server_wireless_interface.h"
#include "usim_server_sim_interface.h"
#include "phone_command_sim.h"
#include "flash_file_address.h"
#include "global_data_exchange_extern.h"
#include "phone_command_sim.h"
#include "nrf_delay.h"
#include "define_all_data.h"

#if (IF_SOFTDEIVE_USED)
/* Constants for timeslot API */
static nrf_radio_request_t    m_timeslot_request;
static nrf_radio_signal_callback_return_param_t signal_callback_return_param;
static volatile uint32_t   RETURN_WATCH_SIM_LISTENING                = SIGNAL_CALLBACK_RETURN_UNDO;
static volatile uint32_t   RETURN_WATCH_SIM_DATA_RECEIVE             = SIGNAL_CALLBACK_RETURN_UNDO;
static volatile uint32_t   RETURN_WATCH_SIM_DATA_SEND                = SIGNAL_CALLBACK_RETURN_UNDO;
static volatile uint32_t   RETURN_AUTHENTICATION_WIRELESS_DATA_SEND  = SIGNAL_CALLBACK_RETURN_UNDO;
static volatile uint32_t   RETURN_PHONE_CONFIG_DATA_SEND             = SIGNAL_CALLBACK_RETURN_UNDO;

/********************************************************************************/
void start_timer0(void)
/*--------------------------------------------------------------------------------
| start TIMER0 for timing in time slot
|
--------------------------------------------------------------------------------*/
{
    NRF_TIMER0->TASKS_STOP = 1;
    NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER0->TASKS_CLEAR = 1;
    /* timer tick is 1us */
    NRF_TIMER0->PRESCALER   = 4;
    NRF_TIMER0->BITMODE     = TIMER_BITMODE_BITMODE_32Bit;
    NRF_TIMER0->TASKS_START = 1;               // Start timer
}

/********************************************************************************/
void time_slot_request_time_out(void)
/*--------------------------------------------------------------------------------
| time out action for time slot request
|
--------------------------------------------------------------------------------*/
{
  switch (TIME_SLOT_SIGNAL_TYPE_SET)
  {
    case TIME_SLOT_SIGNAL_PHONE_COMMAND:
    {
      /* for phone command status, since the time is less, write the reaponse byte to phone directly */
      if (*(PHONE_COMMAND + 2) == 0xf2)
      {
        phone_command_status_response_0xf2(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      }
      set_event_phone_command( );
      break;
    }

    /* phone command waiting status set, ready for read the nect phone command */
    case TIME_SLOT_SIGNAL_PHONE_COMMAND_WIRELESS_AUTHENTICATION:
    case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION:
    case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_GET_DATA:
    case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_ADF_SELECTION:
    {
      set_event_phone_command( );
      break;
    }

    /* do nothing, just extend the CC value of RTC2 */
    case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND:
    case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE:
    case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE:
    case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_DATA_SEND:
    case TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION:
    case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND:
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      break;
    }

    case TIME_SLOT_SIGNAL_USIM_CARD_BINARY_UPDATE:
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      break;
    }

    default:
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      break;
    }
  }
}

/********************************************************************************/
void time_slot_request_queue(void)
/*--------------------------------------------------------------------------------
| time slot request
|
--------------------------------------------------------------------------------*/
{
  /* time slot request time out */
  if (((NRF_RTC2->COUNTER - TIME_SLOT_REQUEST_START_TIME) > DATA_TX_TIME_MS) ||
        (NRF_RTC2->COUNTER < TIME_SLOT_REQUEST_START_TIME))
  {
#if (IF_LOG_OUTPUT)
    printf("----------------- time out in time_slot_request_queue --------------\r\n");
#endif
    time_slot_request_time_out( );

    return;
  }

  /* if there is another time slot task ongoing, wait for some time and request again */
  if (SEMAPHORE_TIME_SLOT_REQUEST)
  {
    NRF_RTC2->CC[1] = NRF_RTC2->COUNTER + TIME_SLOT_REQUEST_RETRY_INTERVAL_MS;
    return;
  }
  m_timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
  m_timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED;
  m_timeslot_request.params.earliest.priority    = NRF_RADIO_PRIORITY_HIGH;
  m_timeslot_request.params.earliest.length_us   = TIME_SLOT_REQUEST_LENGTH_US;
  m_timeslot_request.params.earliest.timeout_us  = 1500000;

  /* time slot request successfully */
  if (sd_radio_request(&m_timeslot_request) == NRF_SUCCESS)
  {
    /* lock the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 1;
    /* set the time slot callback signal type */
    TIME_SLOT_SIGNAL_TYPE = TIME_SLOT_SIGNAL_TYPE_SET;
    /* set the time slot callback event type */
    TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
  }
  else /* time slot request error, wait for some time and request again */
  {
    NRF_RTC2->CC[1] = NRF_RTC2->COUNTER + TIME_SLOT_REQUEST_RETRY_INTERVAL_MS;
  }
}

/********************************************************************************/
void nrf_evt_signal_handler(uint32_t evt_id, void * p_context)
/*--------------------------------------------------------------------------------
| Time slot signal handler
|
--------------------------------------------------------------------------------*/
{
  switch (evt_id)
  {
    case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
      /* No implementation needed */
      break;

    /* the task handler is finished in time slot */
    case NRF_EVT_RADIO_SESSION_IDLE:
    {
      switch (TIME_SLOT_EVENT_TYPE)
      {
        case TIME_SLOT_EVENT_PHONE_COMMAND:
        {
          break;
        }
        case TIME_SLOT_EVENT_PHONE_CONNECTION_REQUEST_RADIO_LINK:
        {
          handler_time_slot_event_signal_usim_client_random_access_connection_request( );
          break;
        }

        case TIME_SLOT_EVENT_PHONE_COMMAND_WIRELESS_AUTHENTICATION:
        {
          handler_time_slot_event_signal_usim_client_wireless_authentication( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_LISTENING_RADIO_LINK:
        {
          handler_time_slot_event_signal_usim_server_radio_link_listening( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE:
        {
          handler_time_slot_event_signal_usim_server_wireless_sim_command_receive( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_SIM_DATA_SEND:
        {
          handler_time_slot_event_signal_usim_server_send_file_data( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE:
        {
          handler_time_slot_event_signal_usim_server_sim_config_data_receive( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_RANDOM_NUMBER_GENERATION:
        {
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND:
        {
          handler_time_slot_event_signal_usim_server_wireless_authentication_sim_command( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND:
        {
          handler_time_slot_event_signal_usim_server_wireless_authentication_data_send( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_AUTHENTICATION:
        {
          handler_time_slot_event_signal_usim_server_sim_command_authentication_0x88( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_GET_DATA:
        {
          handler_time_slot_event_signal_usim_server_sim_command_get_data_0xc0( );
          break;
        }

        case TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_ADF_SELECTION:
        {
          handler_time_slot_event_signal_usim_server_sim_command_adf_selection( );
          break;
        }

#if (IF_USIM_BINARY_UPDATE)
        case TIME_SLOT_EVENT_USIM_CARD_BINARY_UPDATE:
        {
          handler_time_slot_event_signal_usim_server_ef_binary_update( );
          break;
        }
#endif
        default:
        {
          /* do nothing */
           break;
        }
      }

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;

      break;
    }

    case NRF_EVT_RADIO_SESSION_CLOSED:
    {
      /* No implementation needed, session ended */
    }
      break;

    /* the tiem slot request is blocked */
    case NRF_EVT_RADIO_BLOCKED:
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++++  time slot request BLOCKED, re-request... ++++++++++++++++++++++++++++++++\r\n");
#endif
      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;

      switch (TIME_SLOT_SIGNAL_TYPE)
      {
        case TIME_SLOT_SIGNAL_PHONE_COMMAND:
        {
          if (*(PHONE_COMMAND + 2) == 0xf2)
          {
            time_slot_request(TIMER_SLOT_PHONE_COMMAND_0xF2_US);
          }
          else
          {
            time_slot_request(TIMER_SLOT_PHONE_COMMAND_DEFAULT_US);
          }

          break;
        }

        case TIME_SLOT_SIGNAL_PHONE_CONNECTION_REQUEST_RADIO_LINK:
        {
          if ((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) && ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS))
          {
            break;
          }
          else
          {
            time_slot_request(TIMER_SLOT_PHONE_CONNECTION_REQUEST_US);
            break;
          }
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION:
        {
          time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_GET_DATA:
        {
          time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0xC0_US);
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_LISTENING_RADIO_LINK:
        {
          if ((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) && ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS))
          {
            break;
          }
          else
          {
            time_slot_request(TIMER_SLOT_SIM_CONNECTION_LISTENING_US);
            break;
          }
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE:
        {
#if (IF_DATA_CC_CODED)
          time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
          time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_DATA_SEND:
        {
#if (IF_DATA_CC_CODED)
          time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
          time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION:
        {
#if (IF_DATA_CC_CODED)
          time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
          time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND:
        {
          time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_WIRELESS_US);
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND:
        {
#if (IF_DATA_CC_CODED)
          time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
          time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
          break;
        }

#if (IF_USIM_BINARY_UPDATE)
        case TIME_SLOT_EVENT_USIM_CARD_BINARY_UPDATE:
        {
          time_slot_request(TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US);
          break;
        }
#endif
      }
      break;
    }

    case NRF_EVT_RADIO_CANCELED:
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++++  time slot request CANCELLED, re-request... ++++++++++++++++++++++++++++++++\r\n");
#endif
      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;

      break;
    }

    default:
    {
      break;
    }
  }
}

/********************************************************************************/
nrf_radio_signal_callback_return_param_t *time_slot_callback(uint8_t signal_type)
/*--------------------------------------------------------------------------------
| Time slot event handler
|
--------------------------------------------------------------------------------*/
{
  uint32_t start_time_stamp_timer0;
  uint32_t request_time_slot_length_us;

  switch(signal_type)
  {
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
    {
      /* set the time slot callback return parameters */
      signal_callback_return_param.params.request.p_next = NULL;
      signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_END;
      /* get the time slot start time stamp */
      start_timer0( );
      NRF_TIMER0->TASKS_CAPTURE[0] = 1;
      start_time_stamp_timer0 = NRF_TIMER0->CC[0];
      /* break immediately if phone command read is ongoing */
      if ((FLAG_PHONE_COMMAND_READ) &&
        (TIME_SLOT_SIGNAL_TYPE != TIME_SLOT_SIGNAL_PHONE_COMMAND) &&
        (TIME_SLOT_SIGNAL_TYPE != TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION) &&
        (TIME_SLOT_SIGNAL_TYPE != TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_GET_DATA) &&
        (TIME_SLOT_SIGNAL_TYPE != TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_ADF_SELECTION) &&
        (TIME_SLOT_SIGNAL_TYPE != TIME_SLOT_SIGNAL_PHONE_COMMAND_WIRELESS_AUTHENTICATION))
      {
        TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;

        break;
      }

      /* different time slot request type */
      switch (TIME_SLOT_SIGNAL_TYPE)
      {
        case TIME_SLOT_SIGNAL_PHONE_COMMAND:
        {
          if (*(PHONE_COMMAND + 2) == 0xf2)
          {
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_0xF2_US;
          }
          else if (*(PHONE_COMMAND + 2) == 0xa4)
          {
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_0xA4_US;
          }
          else if ((*(PHONE_COMMAND + 2) == 0xd6) || (*(PHONE_COMMAND + 2) == 0xc2) || (*(PHONE_COMMAND + 2) == 0x14))
          {
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_0xD6_US;
          }
          else
          {
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_DEFAULT_US;
          }

          main_smart_phone_sim_local_wireless(start_time_stamp_timer0,
          request_time_slot_length_us - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);

          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_PHONE_COMMAND;

          break;
        }

        case TIME_SLOT_SIGNAL_PHONE_CONNECTION_REQUEST_RADIO_LINK:
        {
          CONNECTION_SLOT_TIME_MS = NRF_RTC2->COUNTER;

          /* the the time slot is too close to next status phone command 0xf2, break immediately  */
          if ((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) &&
            ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS / 2))
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
          }
          else
          {
            NRF_RADIO->POWER = 1;
            radio_configure();

            RETURN_PHONE_CONNECTION_REQUEST = connection_request_phone_ble(0x11, start_time_stamp_timer0,
              TIMER_SLOT_PHONE_CONNECTION_REQUEST_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);

            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_PHONE_CONNECTION_REQUEST_RADIO_LINK;
            NRF_RADIO->POWER = 0;
          }

          break;
        }

        case TIME_SLOT_SIGNAL_PHONE_COMMAND_WIRELESS_AUTHENTICATION:
        {
          if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage1)
          {
            request_time_slot_length_us = TIMER_SLOT_PHONE_CONNECTION_REQUEST_US;
          }
          else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage3)
          {
#if (IF_DATA_CC_CODED)
            request_time_slot_length_us = 2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US + TIMER_SLOT_CC_ENCODING_TIME_US;
#else
            request_time_slot_length_us = TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US;
#endif
          }
          else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage5)
          {
#if (IF_DATA_CC_CODED)
            request_time_slot_length_us = 2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US;
#else
            request_time_slot_length_us = TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US;
#endif
          }
          else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage6)
          {
#if (IF_DATA_CC_CODED)
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US + TIMER_SLOT_CC_DECODING_TIME_US;
#else
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US;
#endif
          }
          else
          {
            request_time_slot_length_us = TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US;
          }

          NRF_RADIO->POWER = 1;
          radio_configure();

          RETURN_PHONE_AUTHENTICATION_WIRELESS = phone_command_authentication_wireless_ble(PHONE_COMMAND, start_time_stamp_timer0,
            request_time_slot_length_us - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);

          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_PHONE_COMMAND_WIRELESS_AUTHENTICATION;
          NRF_RADIO->POWER = 0;
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_LISTENING_RADIO_LINK:
        {
          CONNECTION_SLOT_TIME_MS = NRF_RTC2->COUNTER;

          /* the the time slot is too close to next status phone command 0xf2, break immediately  */
          if ((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) &&
            ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS / 2))
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
          }
          else
          {
            NRF_RADIO->POWER = 1;
            radio_configure();

            RETURN_WATCH_SIM_LISTENING = connection_listening_sim(start_time_stamp_timer0,
              TIMER_SLOT_SIM_CONNECTION_LISTENING_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);

            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_LISTENING_RADIO_LINK;
            NRF_RADIO->POWER = 0;
          }

          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE:
        {
          NRF_RADIO->POWER = 1;
          radio_configure();

#if (IF_DATA_CC_CODED)
          RETURN_WATCH_SIM_DATA_RECEIVE = packet_wireless_receive_ble(USIM_SERVER_COMMAND_WIRELESS_RECEIVE,
            start_time_stamp_timer0, 2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#else
          RETURN_WATCH_SIM_DATA_RECEIVE = packet_wireless_receive_ble(USIM_SERVER_COMMAND_WIRELESS_RECEIVE,
            start_time_stamp_timer0, TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#endif

          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_SIM_COMMAND_RECEIVE;
          NRF_RADIO->POWER = 0;
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_SIM_DATA_SEND:
        {
          NRF_RADIO->POWER = 1;
          radio_configure();

#if (IF_DATA_CC_CODED)
          RETURN_WATCH_SIM_DATA_SEND = sim_command_send(USIM_SERVER_COMMAND_WIRELESS_RECEIVE,
            start_time_stamp_timer0, 2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#else
          RETURN_WATCH_SIM_DATA_SEND = sim_command_send(USIM_SERVER_COMMAND_WIRELESS_RECEIVE,
            start_time_stamp_timer0, TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#endif

          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_SIM_DATA_SEND;
          NRF_RADIO->POWER = 0;
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION:
        {
          /* generate random challenge data for next use */
#if (IF_DATA_CC_CODED)
          random_vector_generate(AUTH_CHALL_DATA_RAM, KEY_LENGTH, start_time_stamp_timer0,
            2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US, 0);
#else
          random_vector_generate(AUTH_CHALL_DATA_RAM, KEY_LENGTH, start_time_stamp_timer0,
            TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US, 0);
#endif

          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_RANDOM_NUMBER_GENERATION;
          /* renew RTC2 CC event interrupt and watch-dog */
          rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND:
        {
          if (usim_server_authentication_wireless_ble(USIM_SERVER_COMMAND_WIRELESS_RECEIVE, start_time_stamp_timer0,
            TIMER_SLOT_PHONE_COMMAND_SIM_0x88_WIRELESS_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US))
          {
            /* renew RTC2 CC event interrupt and watch-dog */
            rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
          }
          else
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND;
          }
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND:
        {
          NRF_RADIO->POWER = 1;
          radio_configure();
#if (IF_DATA_CC_CODED)
          RETURN_AUTHENTICATION_WIRELESS_DATA_SEND = packet_wireless_transmit_ble(READ_BYTE_UICC_TERMINAL,
            start_time_stamp_timer0, 2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#else
          RETURN_AUTHENTICATION_WIRELESS_DATA_SEND = packet_wireless_transmit_ble(READ_BYTE_UICC_TERMINAL,
            start_time_stamp_timer0, TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#endif
          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND;
          NRF_RADIO->POWER = 0;
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE:
        {
          NRF_RADIO->POWER = 1;
          radio_configure();

#if (IF_DATA_CC_CODED)
          RETURN_PHONE_CONFIG_DATA_SEND = usim_server_command_receive_data(USIM_SERVER_COMMAND_WIRELESS_RECEIVE,
            start_time_stamp_timer0, 2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#else
            RETURN_PHONE_CONFIG_DATA_SEND = usim_server_command_receive_data(USIM_SERVER_COMMAND_WIRELESS_RECEIVE,
              start_time_stamp_timer0, TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);
#endif
          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_SIM_CONFIG_DATA_RECEIVE;
          NRF_RADIO->POWER = 0;
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION:
        {
          /* watch command GSM algorithm */
          if (usim_server_command_GSM_algorithm_ble(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM,
            start_time_stamp_timer0, TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US))
          {
            set_event_phone_command( );
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
          }
          else
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_AUTHENTICATION;
          }
          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_GET_DATA:
        {
          if (usim_server_command_get_data_ble(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM,
            start_time_stamp_timer0, TIMER_SLOT_PHONE_COMMAND_SIM_0xC0_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US))
          {
            set_event_phone_command( );
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
          }
          else
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_GET_DATA;
          }

          break;
        }

        case TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_ADF_SELECTION:
        {
          /* start sim clock signal */
          clock_sim_start_4m(PIN_CLOCK_SIM);

          sim_command_select(0x3f00, ETU_TICKS_SIM);

          if (sim_file_7fff_selected_check( ))
          {
            set_event_phone_command( );

            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
          }
          else
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_SERVER_SIM_COMMAND_ADF_SELECTION;
          }
          /* stop SIM clock */
          clock_sim_stop_4m(PIN_CLOCK_SIM);

          SELECTED_FILE_SIM = 0x7fff;
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

#if (IF_USIM_BINARY_UPDATE)
        case TIME_SLOT_SIGNAL_USIM_CARD_BINARY_UPDATE:
        {
          if (BINARY_UPDATE_FILE_ID == 1)
          {
            TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
            break;
          }

          RETURN_USIM_FILE_BINAREY_UPDATE = sim_command_usim_card_update_binary_0xd6_ble(BINARY_UPDATE_FILE_ID, start_time_stamp_timer0,
              TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US - TIMER_SLOT_SIGNAL_CALLBACK_MARGIN_US);

          TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_USIM_CARD_BINARY_UPDATE;

          break;
        }
#endif
        default:
        {
          break;
        }
      }

      break;
    }

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
    {
      signal_callback_return_param.params.request.p_next = NULL;
      signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
      break;
    }

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
    {
      signal_callback_return_param.params.request.p_next = NULL;
      signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_END;
      break;
    }

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED:
    {
      break;
    }

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_FAILED:
    {
      break;
    }

    default:
      //No implementation needed
      break;
  }
  return (&signal_callback_return_param);
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_client_random_access_connection_request(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM client connection request for random access
|
--------------------------------------------------------------------------------*/
{
  if (FLAG_PHONE_COMMAND_READ)
  {
    return;
  }

  if (RETURN_PHONE_CONNECTION_REQUEST == 2)
  {
#if (IF_LOG_OUTPUT)
    printf("++++++++++++++++++++ BLE on, Wireless SIM Off, no request transmitted ++++++++++++++++++++ \r\n");
#endif
    phone_connection_state_check(1);

    /* update RTC2 CC register value, to set RTC2 new interrupt event time */
    rtc2_compare0_event_postpone(RTC2_COMPARE0_EVENT_INTERVAL_MS);
  }
  else
  {
    if ((!RETURN_PHONE_CONNECTION_REQUEST) && (!connection_request_phone_post_decoding(PHONE_CONNECTION_REQUEST_TYPE, PHONE_CONNECTION_REQUEST_START_TIME)))
    {
      phone_connection_state_check(0);
    }
    else
    {
      if (((NRF_RTC2->COUNTER - PHONE_CONNECTION_REQUEST_START_TIME) < DATA_TX_TIME_MS) &&
        (NRF_RTC2->COUNTER > PHONE_CONNECTION_REQUEST_START_TIME) &&
        (!FLAG_PHONE_COMMAND_READ) &&
        (!((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) &&
        ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS))))
      {
        /* update RTC2 CC register value, to set RTC2 new interrupt event time */
        rtc2_compare0_event_postpone(RTC2_COMPARE0_EVENT_INTERVAL_MS);

        nrf_delay_us(150);
        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;

        time_slot_request(TIMER_SLOT_PHONE_CONNECTION_REQUEST_US);
      }
      else
      {
#if (IF_LOG_OUTPUT)
        (PHONE_CONNECTION_REQUEST_TIME[6])++;
        printf("\r\n++++++++++++++++++++++++++++++ BLE on, Time out for phone connection request, Rx time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - PHONE_CONNECTION_REQUEST_START_TIME);
        printf("\r\n---------------------------- PHONE_CONNECTION_REQUEST_TIME = %ld, %ld, %ld, %ld, %ld, %ld, %ld ----------------------------\r\n", PHONE_CONNECTION_REQUEST_TIME[0], PHONE_CONNECTION_REQUEST_TIME[1], PHONE_CONNECTION_REQUEST_TIME[2], PHONE_CONNECTION_REQUEST_TIME[3], PHONE_CONNECTION_REQUEST_TIME[4], PHONE_CONNECTION_REQUEST_TIME[5], PHONE_CONNECTION_REQUEST_TIME[6]);
#endif
        phone_connection_state_check(1);

        /* update RTC2 CC register value, to set RTC2 new interrupt event time */
        rtc2_compare0_event_postpone(RTC2_COMPARE0_EVENT_INTERVAL_MS);
      }
    }
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_client_wireless_authentication(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM client wireless connection authentication
|
--------------------------------------------------------------------------------*/
{
  /* stage 0 */
  if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage0)
  {
    if (!RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage1;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
      time_slot_request(TIMER_SLOT_PHONE_CONNECTION_REQUEST_US);
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++ Time out for phone command confirm byte read, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME);
#endif
      set_event_phone_command( );
    }
  }
  /* stage 1 */
  else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage1)
  {
    if ((!RETURN_PHONE_AUTHENTICATION_WIRELESS) && (!connection_request_phone_post_decoding(PHONE_CONNECTION_REQUEST_TYPE, PHONE_CONNECTION_REQUEST_START_TIME)))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage2;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
    }
    else
    {
      if (((NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME) < DATA_TX_TIME_MS) &&
        (NRF_RTC2->COUNTER > PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;
        time_slot_request(TIMER_SLOT_PHONE_CONNECTION_REQUEST_US);
      }
      else
      {
#if (IF_LOG_OUTPUT)
        printf("\r\n++++++++++++++++++++++++++++++ Time out for phone command authentication connection request, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME);
#endif
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage2;

        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;
        time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
      }
    }
  }
  /* stage 2 */
  else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage2)
  {
    if (!RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage3;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US + TIMER_SLOT_CC_ENCODING_TIME_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++ Time out for phone connection request, no connection, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME);
#endif
      set_event_phone_command( );
    }
  }
  /* stage 3 */
  else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage3)
  {
    if (!RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage4;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
    }
    else
    {
      if (((NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME) < DATA_TX_TIME_MS) &&
      (NRF_RTC2->COUNTER > PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
        time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US + TIMER_SLOT_CC_ENCODING_TIME_US);
#else
        time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
      }
      else
      {
#if (IF_LOG_OUTPUT)
        printf("\r\n++++++++++++++++++++++++++++++ Time out for phone command authentication transmission, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME);
#endif
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage4;

        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;
        time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
      }
    }
  }
  /* stage 4 */
  else   if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage4)
  {
    if (!RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage5;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
    }
    else
    {
      set_event_phone_command( );
    }
  }
  /* stage 5 */
  else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage5)
  {
    if (!RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage6;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US + TIMER_SLOT_CC_DECODING_TIME_US);
#else
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
#endif
    }
    else
    {
      if (((NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME) < DATA_TX_TIME_MS) &&
        (NRF_RTC2->COUNTER > PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
        time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
        time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
      }
      else
      {
#if (IF_LOG_OUTPUT)
        printf("\r\n++++++++++++++++++++++++++++++ Time out for phone command authentication SIM data receive, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME);
#endif
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage6;

        /* free the time slot request semaphore */
        SEMAPHORE_TIME_SLOT_REQUEST = 0;

#if (IF_DATA_CC_CODED)
        time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US + TIMER_SLOT_CC_DECODING_TIME_US);
#else
        time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
#endif
      }
    }
  }
  /* stage 6 */
  else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage6)
  {
    if (!RETURN_PHONE_AUTHENTICATION_WIRELESS)
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n-------------------------- phone command authentication stage 6 --------------------------------------------------------\r\n");
      printf_log_rx(*READ_BYTE_UICC_TERMINAL, READ_BYTE_UICC_TERMINAL + 1);
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage7;

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
    }
    else
    {
      set_event_phone_command( );
    }
  }
  /* stage 7 */
  else if (PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE == PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage7)
  {
    TIME_SLOT_EVENT_TYPE = TIME_SLOT_EVENT_DEFAULT_VALUE;
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_radio_link_listening(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server wireless signal listening results process
|
--------------------------------------------------------------------------------*/
{
  sim_connection_state_check(RETURN_WATCH_SIM_LISTENING);

#if (IF_LOG_OUTPUT)
  connection_listening_sim_return_message(RETURN_WATCH_SIM_LISTENING);
#endif
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_wireless_sim_command_receive(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server receiving data of wireless SIM command
|
--------------------------------------------------------------------------------*/
{
#if (IF_DATA_CC_CODED)
  uint32_t i;
#endif

  if (!RETURN_WATCH_SIM_DATA_RECEIVE) /* phone command received correct */
  {
#if (IF_DATA_CC_CODED)
    ccdecoder_byte_sequence_64(USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 1, ENCODE_BLOCK_INDEX, GLOBAL_TMP_RAM);
    for (i=0; i<(*GLOBAL_TMP_RAM + 1); i++)
    {
      *(USIM_SERVER_COMMAND_WIRELESS_RECEIVE + i) = *(GLOBAL_TMP_RAM + i);
    }
#endif

#if (IF_LOG_OUTPUT)
    printf("\r\n-------------------------- Phone command received --------------------------------------------------------\r\n");
    printf_log_rx(*USIM_SERVER_COMMAND_WIRELESS_RECEIVE, USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 1);
#endif
    /* wireless UICC-terminal interface authentication check */
    if (byte_string_comparison(KEY_LENGTH, USIM_SERVER_COMMAND_WIRELESS_RECEIVE + 6, AUTHENTICATE_RESULT) && (!((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1)))
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n------------------------- authentication error in handler_time_slot_event_signal_usim_server_wireless_sim_command_receive -------------------------\r\n");
      printf("transmitted: ");
      printf_log_tx(KEY_LENGTH, AUTHENTICATE_RESULT);
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
    else
    {
      if (main_usim_server_wireless_sim_ble_stage1( ))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
    }
  }
  else
  {
    if (((NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME) < DATA_TX_TIME_MS) &&
      (NRF_RTC2->COUNTER > SIM_DATA_RECEIVE_COMMAND_START_TIME) && (!FLAG_PHONE_COMMAND_READ))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++ Time out for SIM data receive phone command, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME);
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_send_file_data(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server send wireless data of EF files in USIM card
|
--------------------------------------------------------------------------------*/
{
  if (!RETURN_WATCH_SIM_DATA_SEND)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n----------------------------------- USIM data send complete --------------------------------------------\r\n");
    /* wireless USIM0 */
    if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
    {
      printf_log_tx(*USIM0_EF_DATA_RAM, USIM0_EF_DATA_RAM + 1);
    }
    /* wireless USIM1 */
    else
    {
      printf_log_tx(*USIM1_EF_DATA_RAM, USIM1_EF_DATA_RAM + 1);
    }
#endif
    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION;

    /* free the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 0;
    time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
  }
  else
  {
    if (((NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME) < DATA_TX_TIME_MS) &&
      (NRF_RTC2->COUNTER > SIM_DATA_RECEIVE_COMMAND_START_TIME) && (!FLAG_PHONE_COMMAND_READ))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++ Time out for SIM data send, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME);
#endif
      /* renew RTC2 CC event interrupt and watch-dog */
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_sim_config_data_receive(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server receiving config data synch
|
--------------------------------------------------------------------------------*/
{
  if (!RETURN_PHONE_CONFIG_DATA_SEND)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n-------------------------- SIM data receive confirm message send complete -----------------------------------\r\n");
#endif
    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION;

    /* free the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 0;
    time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
  }
  else
  {
    if (((NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME) < DATA_TX_TIME_MS) &&
      (NRF_RTC2->COUNTER > SIM_DATA_RECEIVE_COMMAND_START_TIME) && (!FLAG_PHONE_COMMAND_READ))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++ Time out for SIM data send, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME);
#endif
      /* renew RTC2 CC event interrupt and watch-dog */
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_sim_command_authentication_0x88(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server side local SIM command authentication
|
--------------------------------------------------------------------------------*/
{
  /* watch command is completed */
  if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage4)
  {
    set_event_phone_command( );
  }
  else
  {
    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

    /* free the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 0;
    time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_sim_command_get_data_0xc0(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server side get data of phone command
|
--------------------------------------------------------------------------------*/
{
  if (USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE == USIM_SERVER_COMMAND_GET_DATA_BLE_stage2)
  {
    set_event_phone_command( );
  }
  else
  {
    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

    /* free the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 0;
    time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0xC0_US);
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_wireless_authentication_sim_command(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server side wireless authentication SIM command
|
--------------------------------------------------------------------------------*/
{
  if (USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE == USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage2)
  {
    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_DATA_SEND;

    /* free the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
    time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
    time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_wireless_authentication_data_send(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server side wireless authentication results data send
|
--------------------------------------------------------------------------------*/
{
  if (!RETURN_AUTHENTICATION_WIRELESS_DATA_SEND)
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n-------------------------- SIM data send complete --------------------------------------------------------\r\n");
    printf_log_tx(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1);
#endif
    /* if the wireless USIM logical address is 7 (reserved for iPAD), let re-access network by local USIM */
    if ((USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS == 7) &&
      ((((PHONE_WIRELESS_USED_USIM & 1) == 0) && (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)) ||
      (((PHONE_WIRELESS_USED_USIM & 1) == 1) && (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1))))
    {
      CONNECTION_STATE |= 0x4;
#if (IF_LOG_OUTPUT)
      printf("\r\n-------------------------- Set phone re-access network, USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS = %d --------------------------------------------------------\r\n", USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS);
#endif
    }

    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_RANDOM_NUMBER_GENERATION;

    /* free the time slot request semaphore */
    SEMAPHORE_TIME_SLOT_REQUEST = 0;
    time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
  }
  else
  {
    if (((NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME) < DATA_TX_TIME_MS) &&
      (NRF_RTC2->COUNTER > SIM_DATA_RECEIVE_COMMAND_START_TIME) && (!FLAG_PHONE_COMMAND_READ))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
#if (IF_DATA_CC_CODED)
      time_slot_request(2 * TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#else
      time_slot_request(TIMER_SLOT_SIM_DATA_WIRELESS_TX_RX_US);
#endif
    }
    else
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n++++++++++++++++++++++++++++++ Time out for SIM data send, total time used = %ld ++++++++++++++++++++++++++++++++\r\n", NRF_RTC2->COUNTER - SIM_DATA_RECEIVE_COMMAND_START_TIME);
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
  }
}

/********************************************************************************/
void handler_time_slot_event_signal_usim_server_sim_command_adf_selection(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server side SIM command ADF selection
|
--------------------------------------------------------------------------------*/
{
  WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage0;
  TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION;
  time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);

  /* free the time slot request semaphore */
  SEMAPHORE_TIME_SLOT_REQUEST = 0;
  time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
}

#if (IF_USIM_BINARY_UPDATE)
/********************************************************************************/
void handler_time_slot_event_signal_usim_server_ef_binary_update(void)
/*--------------------------------------------------------------------------------
| time slot event handler for USIM server side EF data binary update of USIM card
|
--------------------------------------------------------------------------------*/
{
  /* stage 0 */
  if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage0)
  {
    if (RETURN_USIM_FILE_BINAREY_UPDATE == 0)
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n--------------------------------- USIM binary update stage 0 --------------------------------------------\r\n");
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
      SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage1;
      time_slot_request(TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US);
    }
    else
    {
      printf("\r\n++++++++++++++++++++++++++++++++ USIM binary update stage 0 ERROR .... ++++++++++++++++++++++++++++++++\r\n");
    }
  }
  /* stage 1 */
  else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage1)
  {
    if (RETURN_USIM_FILE_BINAREY_UPDATE == 0)
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n--------------------------------- USIM binary update stage 1 --------------------------------------------\r\n");
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;
      SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage2;
      time_slot_request(TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US);
    }
    else
    {
      printf("\r\n++++++++++++++++++++++++++++++++ USIM binary update stage 1 ERROR, RETURN_USIM_FILE_BINAREY_UPDATE = %ld, ++++++++++++++++++++++++++++++++\r\n", RETURN_USIM_FILE_BINAREY_UPDATE);
    }
  }
  /* stage 2 */
  else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage2)
  {
    if (RETURN_USIM_FILE_BINAREY_UPDATE == 0)
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n--------------------------------- USIM binary update stage 2 --------------------------------------------\r\n");
#endif

      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      SEMAPHORE_TIME_SLOT_REQUEST = 0;

      if ((SELECTED_FILE_SIM == 0x7fff) || ((SELECTED_FILE_SIM >> 16) == 0x7fff))
      {
        usim_binary_update_file_id_clear(BINARY_UPDATE_FILE_ID);

        if (SIM_FILE_UPDATE_BINARY_TASK_QUEUE == 0)
        {
          /* USIM0 is used by phone locally */
          if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
          {
            /* set the mark bit for flash data write check */
            FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USIM0_DATA_OFFSET_POS);
          }
#if (PIN_VCC_SIM1 != PIN_NULL)
          /* USIM1 is used by phone locally */
          else
          {
            /* set the mark bit for flash data write check */
            FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USIM1_DATA_OFFSET_POS);
          }
#endif
          /* set flash write flag */
          FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
        }
      }
      else
      {
        SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage3;
        time_slot_request(TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US);
      }
    }
    else
    {
      printf("\r\n++++++++++++++++++++++++++++++++ USIM binary update stage 2 ERROR, RETURN_USIM_FILE_BINAREY_UPDATE = %ld .... ++++++++++++++++++++++++++++++++\r\n", RETURN_USIM_FILE_BINAREY_UPDATE);
    }
  }
  /* stage 3 */
  else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage3)
  {
    if (RETURN_USIM_FILE_BINAREY_UPDATE == 0)
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n--------------------------------- USIM binary update stage 3 --------------------------------------------\r\n");
#endif
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

      /* free the time slot request semaphore */
      SEMAPHORE_TIME_SLOT_REQUEST = 0;

      SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage4;
      time_slot_request(TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US);
    }
    else
    {
      printf("\r\n++++++++++++++++++++++++++++++++ USIM binary update stage 3 ERROR, RETURN_USIM_FILE_BINAREY_UPDATE = %ld .... ++++++++++++++++++++++++++++++++\r\n", RETURN_USIM_FILE_BINAREY_UPDATE);
    }
  }
  /* stage 4 */
  else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage4)
  {
    if (RETURN_USIM_FILE_BINAREY_UPDATE == 0)
    {
#if (IF_LOG_OUTPUT)
      printf("\r\n--------------------------------- USIM binary update stage 4 --------------------------------------------\r\n");
#endif

      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      SEMAPHORE_TIME_SLOT_REQUEST = 0;

      usim_binary_update_file_id_clear(BINARY_UPDATE_FILE_ID);

      if (SIM_FILE_UPDATE_BINARY_TASK_QUEUE == 0)
      {
        /* USIM0 is used by phone locally */
        if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
        {
          /* set the mark bit for flash data write check */
          FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USIM0_DATA_OFFSET_POS);
        }
#if (PIN_VCC_SIM1 != PIN_NULL)
        /* USIM1 is used by phone locally */
        else
        {
          /* set the mark bit for flash data write check */
          FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USIM1_DATA_OFFSET_POS);
        }
#endif
        /* set flash write flag */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
      }
    }
    else
    {
      printf("\r\n++++++++++++++++++++++++++++++++ USIM binary update stage 4 ERROR, RETURN_USIM_FILE_BINAREY_UPDATE = %ld .... ++++++++++++++++++++++++++++++++\r\n", RETURN_USIM_FILE_BINAREY_UPDATE);
    }
  }
  else
  {
    /* do nothing */
  }
}
#endif
#endif


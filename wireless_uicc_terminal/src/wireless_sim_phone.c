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
#include "wireless_sim_phone.h"
#include "define_config.h"
#include "comm_phone_command.h"
#include "comm_wireless_interface.h"
#include "comm_uicc_terminal_interface.h"
#include "comm_initial.h"
#include "nrf_nvic.h"
#include "ble_nus.h"
#include "nrf_sdm.h"
#include "time_slot.h"
#include "app_util_platform.h"
#include "phone_command_sim.h"
#include "phone_wireless_interface.h"
#include "usim_server_sim_interface.h"
#include "usim_server_wireless_interface.h"
#include "flash_file_address.h"
#include "sd_ble_ancs_nus.h"
#include "uart_printf.h"
#include "global_data_exchange.h"
#include "nrf_sdh.h"
#include "ble_flash.h"

/* if the USIM EF data updated to flash
|  bit 0: if local USIM0 updated, 1: updated, 0: not updated
|  bit 1: if local USIM1 updated, 1: updated, 0: not updated
|  bit 2: if wireless USIM0 updated, 1: updated, 0: not updated
|  bit 3: if wireless USIM1 updated, 1: updated, 0: not updated */
static volatile uint8_t    IF_USIM_FILE_DATA_UPDATED = 0;

extern uint8_t SIM_SERVER_ON_USIM0_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_OFF_USIM0_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_ON_USIM1_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_OFF_USIM1_DISPLAY_TEXT[];
extern uint8_t CONFIG_RESTORE_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t USIM0_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t USIM1_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t PASSWD_CHANGED_DISPLAY_TEXT[];
extern uint8_t SYATEM_KEY_UPDATED_DISPLAY_TEXT[];
extern uint8_t LED_PATTERN_UPDATED_DISPLAY_TEXT[];
extern uint8_t NAME_CHANGED_DISPLAY_TEXT[];
extern uint8_t BLE_BOND_DELETE_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t LOGICAL_ADDRESS_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t LED_PATTERN_ERROR_DISPLAY_TEXT[];
extern uint8_t ICCID_2FE2_DATA_MODE_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_ON_USIM0_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_OFF_USIM0_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_ON_USIM1_DISPLAY_TEXT[];
extern uint8_t SIM_SERVER_OFF_USIM1_DISPLAY_TEXT[];
extern uint8_t SIM_CLIENT_ON_USIM0_CONNECTED_STRING[];
extern uint8_t SIM_CLIENT_ON_USIM1_CONNECTED_STRING[];
extern uint8_t SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT[];
extern uint8_t SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT[];
extern uint8_t SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT[];
extern uint8_t WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_DISPLAY_TEXT[];
extern uint8_t BLE_ON_OFF_DISPLAY_TEXT[];
extern uint8_t WIRELESS_SIM_ON_OFF_DISPLAY_TEXT[];
extern uint8_t BLE_ON_OFF_0X88_DISPLAY_TEXT[];
extern uint8_t FIXED_ICCID_DATA_DISPLAY_TEXT[];

#if (IF_SOFTDEIVE_USED)
extern volatile uint8_t    ANCS_NOTIF_LED_PATTERN[ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER];
#endif

#define STRING_LENGTH_NUS                  32
extern uint8_t RECEIVED_STRING_NUS[STRING_LENGTH_NUS + 1];

void (*handler_function)(void);

/********************************************************************************/
void ble_tx_data_process(uint8_t *data_tx)
/*--------------------------------------------------------------------------------
| data send by BLE NUS
|
--------------------------------------------------------------------------------*/
{
  uint8_t length_data_tx;
  uint8_t ble_data_bloack_n;
  uint8_t ble_data_tx[20];
  uint32_t i;
  uint8_t j;

  /* the total length of BLE data Tx */
  length_data_tx = *data_tx;
  /* BLE data blocks */
  ble_data_bloack_n = (length_data_tx) / 19;

  /* 20 bytes BLE payload package send */
  for (i=0; i<ble_data_bloack_n; i++)
  {
    ble_data_tx[0] = i;
    for (j=0; j<19; j++)
    {
      ble_data_tx[j + 1] = *(data_tx + 19 * i + j);
    }

    nus_send_bytes(ble_data_tx, 20);
  }

  /* the last BLE package send */
  ble_data_tx[0] = ble_data_bloack_n;
  for (i=0; i<length_data_tx + 1 - 19 * ble_data_bloack_n; i++)
  {
    ble_data_tx[i + 1] = *(data_tx + 19 * ble_data_bloack_n + i);
  }

  nus_send_bytes(ble_data_tx, length_data_tx + 2 - 19 * ble_data_bloack_n);
}

/********************************************************************************/
void smart_usim_initialization(void)
/*--------------------------------------------------------------------------------
| smart USIM connector initialization
|
--------------------------------------------------------------------------------*/
{
  /* UART initialization for printf */
#if (IF_LOG_OUTPUT)
  uart_initialization( );
#endif

  /* Set radio configuration parameters */
  radio_configure();

  /* config and start RTC2 */
  rtc2_radio_link_config( );

  /* update user configuration settings */
  update_user_configuration( );

  /* PIN set config for USIM card reader interface */
  uicc_reader_sim_pin_setup( );

  /* SAADC initialization */
  saadc_init_config( );

  /* timer1 initialization */
  timer1_initialization( );

  /* GPIO pin configure for SIM-phone contactor interface */
  phone_sim_pin_setup( );

  /* GPIOTE event for GPIO in event initialization */
  gpio_event_in_config( );

  /*-----------------------------------------------------------------------------*/
  if((uint32_t)NRF_UICR->APPROTECT != 0)
  {
    /* enable CPU registers and memory mapped address protection */
    ble_flash_word_write((uint32_t *)(&(NRF_UICR->APPROTECT)), 0);
    /* USIM EF data and wireless SIM interface config initialization */
    usim_data_initialization( );
  }

  /* start watch dog timer */
  watch_dog_timer_init(WATCH_DOG_TIME_OUT_S);
  NRF_RTC2->CC[0] = NRF_RTC2->COUNTER + (COMPARE0_EVENT_POSTPONE_USIM_MS);
}

/********************************************************************************/
void RTC2_IRQHandler(void)
/*--------------------------------------------------------------------------------
| RTC2 tick interrupt handler, for radio link connection and other routine tasks
|
--------------------------------------------------------------------------------*/
{
#if (IF_SOFTDEIVE_USED)
  uint32_t i;
  uint8_t tmp_ble_data[20];
#endif
  static volatile uint32_t status_check_couter = 0;
  uint32_t connection_listening_return_code;

  if ((NRF_RTC2->EVENTS_COMPARE[0] != 0) && ((NRF_RTC2->INTENSET & RTC_INTENSET_COMPARE0_Msk) != 0))
  {
    /* clear CC[0] event */
    NRF_RTC2->EVENTS_COMPARE[0] = 0;

    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;
    NRF_RTC2->CC[0] = NRF_RTC2->COUNTER + RTC2_COMPARE0_EVENT_INTERVAL_MS;

    /* increase the counter */
    status_check_couter++;

    /* wireless USIM */
    if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
    {
      if ((status_check_couter % CONNECT_STATE_TIME_PHONE) == 0)
      {
#if (IF_SOFTDEIVE_USED)
        if (IF_SOFTDEVICE_RUNNING)
        {
          usim_client_connection_request_time_slot_request( );
        }
        else
#endif
        {
          phone_connection_state_check(connection_request_phone(0x11));
        }
      }

      /* phone data send to watch */
      phone_data_send_check( );
    }
    /* local USIM */
    else
    {
#if (IF_SOFTDEIVE_USED)
      if (IF_SOFTDEVICE_RUNNING)
      {
        usim_server_listening_time_slot_request( );
      }
      else
#endif
      {
        connection_listening_return_code = connection_listening_sim(0, 0);
        sim_connection_state_check(connection_listening_return_code);
#if (IF_LOG_OUTPUT)
        connection_listening_sim_return_message(connection_listening_return_code);
#endif
      }
    }

#if (IF_LOG_OUTPUT)
    if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
    {
      printf("Wireless SIM client side, CONNECTION_SLOT_TIME_MS = %ld, NRF_RTC2->CC[0] = %ld, -------- \r\n",  CONNECTION_SLOT_TIME_MS, NRF_RTC2->CC[0]);
    }
    else
    {
      printf("Wireless SIM server side, CONNECTION_SLOT_TIME_MS = %ld, NRF_RTC2->CC[0] = %ld, -------- \r\n",  CONNECTION_SLOT_TIME_MS, NRF_RTC2->CC[0]);
    }
#endif

#if (IF_SOFTDEIVE_USED)
    /* BLE NUS text send */
    if (status_check_couter % 100 == 0)
    {
      /* test text send */
      if (IF_SOFTDEVICE_RUNNING)
      {
        for (i=0; i<20; i++)
        {
          tmp_ble_data[i] = 0;
        }
        sprintf((char *)tmp_ble_data, "%ld", NRF_RTC2->COUNTER);

        nus_send_bytes(tmp_ble_data, 20);
      }
    }
#endif

    /* USIM power status check */
    if ((status_check_couter % 100) == 10)
    {
      usim_power_on_status_check( );
    }
    /* USIM EF data in the flash check */
    if ((status_check_couter % 100) == 20)
    {
      usim_file_update_status_check( );
    }
    /* flash data write check */
    if ((status_check_couter % 100) == 30)
    {
      flash_data_write_check( );
    }

#if (IF_USIM_BINARY_UPDATE)
    /* USIM file binary update check for local USIM used */
    if (((status_check_couter % 100) == 40) && SIM_FILE_UPDATE_BINARY_TASK_QUEUE &&
       ((PHONE_LOCAL_USED_USIM & 0xF0) == 0))
    {
#if (IF_LOG_OUTPUT)
      printf("====================================== binary update ======================================\r\n");
#endif

      usim_binary_update_check( );
    }
#endif /* end of #if (IF_USIM_BINARY_UPDATE) */

#if (IF_SOFTDEIVE_USED)
    if ((status_check_couter % 100) == 50)
    {
      /* BLE statck running check */
      ble_stack_status_check( );
    }

    if ((status_check_couter % 100) == 60)
    {
      ble_stop_task_queue_check( );
    }
#endif /* end of #if (IF_SOFTDEIVE_USED) */

    /* clear the status check counter */
    if ((status_check_couter % 100) == 70)
    {
      config_sych_setting_check( );
      idle_mode_text_diaplay_status_check( );
    }

    /* NAA reset */
    if (((CONNECTION_STATE & 0x4) >> 2) && (USAT_BYTE_LENGTH_BACK == 0))
    {
      USAT_BYTE_LENGTH_BACK = 11;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_SIM_RESET_FETCH;
    }

    /* if there is flash write task in the queue */
    if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_TASK_OFFSET_POS) & 1)
    {
      flash_data_write_task( );
    }

#if (IF_SOFTDEIVE_USED)
    /* BLE NUS received command */
    if (RECEIVED_STRING_NUS[0])
    {
      nus_command_process(RECEIVED_STRING_NUS);
    }
#endif

#ifdef PIN_LED_BLUE
    /* led check */
    if (status_check_couter % 2 == 0)
    {
      led_status_check( );
    }
#endif

    /* update CC */
    if (NRF_RTC2->CC[0] < NRF_RTC2->COUNTER)
    {
      NRF_RTC2->CC[0] = NRF_RTC2->COUNTER + RTC2_COMPARE0_EVENT_INTERVAL_MS;
    }
  }

#if (IF_SOFTDEIVE_USED)
  /* time slot re-request interrupt */
  if ((NRF_RTC2->EVENTS_COMPARE[1] != 0) && ((NRF_RTC2->INTENSET & RTC_INTENSET_COMPARE1_Msk) != 0))
  {
    NRF_RTC2->EVENTS_COMPARE[1] = 0;

    handler_function( );
  }
#endif

  if ((NRF_RTC2->EVENTS_COMPARE[2] != 0) && ((NRF_RTC2->INTENSET & RTC_INTENSET_COMPARE2_Msk) != 0))
  {
    NRF_RTC2->EVENTS_COMPARE[2] = 0;
  }
}

/********************************************************************************/
void rtc2_radio_link_config(void)
/*--------------------------------------------------------------------------------
| RTC2 initial configuration, for radio link connection and other routine tasks
|
--------------------------------------------------------------------------------*/
{
  /* Start 32 KHz clock */
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;

  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
  {
  }
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;

  /* set the interrupt priority of RTC2 as RADIO_LINK */
  sd_nvic_SetPriority(RTC2_IRQn, RADIO_LINK_IRQ_PRIORITY);
  sd_nvic_ClearPendingIRQ(RTC2_IRQn);
  /* Enable Interrupt for the RTC2 in the core. */
  sd_nvic_EnableIRQ(RTC2_IRQn);

  /* set the real time couter resolution is about 1ms */
  NRF_RTC2->TASKS_STOP = 1;
  NRF_RTC2->PRESCALER  = 32;

  /* Compare0 after 3000ms */
  NRF_RTC2->CC[0] = 3000;
  /* Enable COMPARE0 event and COMPARE0 interrupt */
  NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Msk;
  /* Enable COMPARE1 event and COMPARE1 interrupt */
  NRF_RTC2->CC[1] = 0;
  NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE1_Msk;
  NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE1_Msk;

  /* Enable COMPARE2 event and COMPARE2 interrupt */
  NRF_RTC2->CC[2] = 0;
  NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE2_Msk;
  NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE2_Msk;

  NRF_RTC2->TASKS_START = 1;
}

/********************************************************************************/
void rtc2_compare0_event_postpone(uint32_t postpone_time_ms)
/*--------------------------------------------------------------------------------
| postpone RTC2 compare0 event if some events need more time
|
--------------------------------------------------------------------------------*/
{
  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;
  sd_nvic_ClearPendingIRQ(RTC2_IRQn);

  /* update RTC2 CC register value, to set RTC2 new interrupt event time */
  NRF_RTC2->CC[0] = NRF_RTC2->COUNTER + postpone_time_ms;
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
uint32_t time_slot_request(uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| request time slot for wireless USIM interface
|
--------------------------------------------------------------------------------*/
{
  CRITICAL_REGION_ENTER();

  TIME_SLOT_REQUEST_START_TIME = NRF_RTC2->COUNTER;
  TIME_SLOT_REQUEST_LENGTH_US = time_length_us;
  handler_function = time_slot_request_queue;

  time_slot_request_queue();

  CRITICAL_REGION_EXIT();

  return(0);
}

/********************************************************************************/
void usim_client_connection_request_time_slot_request(void)
/*--------------------------------------------------------------------------------
| request time slot for wireless USIM interface
|
--------------------------------------------------------------------------------*/
{
  if (FLAG_PHONE_COMMAND_READ)
  {
#if (IF_LOG_OUTPUT)
      printf("++++++++++++++++++++++++ Phone command time conllision, return in usim_client_connection_request_time_slot_request ++++++++++++++++++++++++\r\n");
#endif
    return;
  }
  if ((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) && ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS))
  {
    return;
  }

  PHONE_CONNECTION_REQUEST_START_TIME = NRF_RTC2->COUNTER;
  TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_PHONE_CONNECTION_REQUEST_RADIO_LINK;
  time_slot_request(TIMER_SLOT_PHONE_CONNECTION_REQUEST_US);
}

/********************************************************************************/
void usim_server_listening_time_slot_request(void)
/*--------------------------------------------------------------------------------
| request time slot for wireless USIM interface for USIM server
|
--------------------------------------------------------------------------------*/
{
  if (FLAG_PHONE_COMMAND_READ)
  {
#if (IF_LOG_OUTPUT)
      printf("++++++++++++++++++++++++ Phone command time conllision, return in usim_server_listening_time_slot_request ++++++++++++++++++++++++\r\n");
#endif
    return;
  }
  if ((NEXT_PHONE_COMMAND_0XF2_TIME > NRF_RTC2->COUNTER) && ((NEXT_PHONE_COMMAND_0XF2_TIME - NRF_RTC2->COUNTER) < TIME_SLOT_NEXT_COMMAND_0XF2_MARGIN_MS))
  {
    return;
  }

  TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_LISTENING_RADIO_LINK;
  time_slot_request(TIMER_SLOT_SIM_CONNECTION_LISTENING_US);
}

/********************************************************************************/
void ble_stack_status_check(void)
/*--------------------------------------------------------------------------------
| BLE stack softdevice running status check
|
--------------------------------------------------------------------------------*/
{
  static uint8_t ble_check_counter = 0;

  /* BLE stack softdevice stopeed for USIM switch, skip BLE staus check */
  if (FLAG_USIM_SWITCH)
  {
    ble_check_counter++;
    if (ble_check_counter > 10)
    {
      ble_check_counter = 0;

      FLAG_USIM_SWITCH = 0;
    }
    return;
  }

  if (nrf_sdh_is_enabled( ))
  {
    /* BLE stack is started */
    IF_SOFTDEVICE_RUNNING = 1;
#if (IF_LOG_OUTPUT)
    printf("----------------- BLE stack softdevice is running... --------------\r\n");
#endif
    /* BLE is set disable in configuration */
    if (!(*((uint8_t *)BLE_STATUS_ON_OFF_FLASH_ADDR)))
    {
      stop_ble_data( );
#if (IF_LOG_OUTPUT)
      printf("====================== BLE Stack Stopped =======================\r\n");
#endif
    }
  }
  else
  {
    /* BLE stack is started */
    IF_SOFTDEVICE_RUNNING = 0;
#if (IF_LOG_OUTPUT)
    printf("++++++++++++++++++ BLE stack softdevice is not running... ++++++++++++++++++\r\n");
#endif
    /* BLE is set enable in configuration */
    /* if the BLE stack is stopped recently, do not start BLE stack */
    if ((*((uint8_t *)BLE_STATUS_ON_OFF_FLASH_ADDR)) && (((NRF_RTC2->COUNTER - BLE_STOP_TIME) > 5000) || (NRF_RTC2->COUNTER < BLE_STOP_TIME)))
    {
#if (IF_LOG_OUTPUT)
      printf("====================== Start BLE Stack =======================\r\n");
#endif
      start_ble_data( );
    }
  }
}

/********************************************************************************/
void nus_command_process(uint8_t *nus_string)
/*--------------------------------------------------------------------------------
| BLE NUS command
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  /* RST: reset teh device */
  if ((*(nus_string + 0) == 3) &&
    ((*(nus_string + 1) == 'r') || (*(nus_string + 1) == 'R')) &&
    ((*(nus_string + 2) == 's') || (*(nus_string + 2) == 'S')) &&
    ((*(nus_string + 3) == 't') || (*(nus_string + 3) == 'T')))
  {
#if (IF_LOG_OUTPUT)
    printf("---------------------------------- BLE NUS command to reset the device  -------------------------------\r\n");
#endif
    /* system reset */
    sd_nvic_SystemReset( );
  }

  /* LED: LED flash pattern setting */
  else if ((RECEIVED_STRING_NUS[0] > 4) &&
    ((*(nus_string + 1) == 'l') || (*(nus_string + 1) == 'L')) &&
    ((*(nus_string + 2) == 'e') || (*(nus_string + 2) == 'E')) &&
    ((*(nus_string + 3) == 'd') || (*(nus_string + 3) == 'D')))
  {
    /* get the user input cammand line string */
    if (RECEIVED_STRING_NUS[0] == 5)
    {
      *(LED_PATTERN_BLE_FLASH + 1) = *(RECEIVED_STRING_NUS + 5);
    }
    else if (RECEIVED_STRING_NUS[0] > 5)
    {
      *(LED_PATTERN_BLE_FLASH + 0) = (RECEIVED_STRING_NUS[0] - 4 <= 15 ? RECEIVED_STRING_NUS[0] - 4 : 15);
      for (i=0; i<*(LED_PATTERN_BLE_FLASH + 0); i++)
      {
        *(LED_PATTERN_BLE_FLASH + i + 1) = *(RECEIVED_STRING_NUS + 5 + i);
      }
      for (i=*(LED_PATTERN_BLE_FLASH + 0); i<15; i++)
      {
        *(LED_PATTERN_BLE_FLASH + i + 1) = 0;
      }
    }
    else
    {
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_LED_PATTERN_ERROR_TEXT_DISPLAY;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)LED_PATTERN_ERROR_DISPLAY_TEXT);

      RECEIVED_STRING_NUS[0] = 0;
      return;
    }

#if (IF_LOG_OUTPUT)
    printf("====================== LED Pattern Input =======================\r\n");
    printf_log_rx(*(LED_PATTERN_BLE_FLASH + 0), (uint8_t *)LED_PATTERN_BLE_FLASH + 1);
#endif
    if (*(LED_PATTERN_BLE_FLASH + 1) != '8')
    {
      /* copy the LED flash pattern seting */
      memcpy(LED_PATTERN_RAM, (uint8_t *)LED_PATTERN_BLE_FLASH, KEY_LENGTH);
      /* set flash write flag */
      FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
      /* set the mark bit for flash data write check */
      FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_LED_PATTERN_OFFSET_POS);
    }
  }

  /* CRS: user config restore to default value */
  else if ((*(nus_string + 0) == 3) &&
    ((*(nus_string + 1) == 'c') || (*(nus_string + 1) == 'C')) &&
    ((*(nus_string + 2) == 'r') || (*(nus_string + 2) == 'R')) &&
    ((*(nus_string + 3) == 's') || (*(nus_string + 3) == 'S')))
  {
#if (IF_LOG_OUTPUT)
    printf("--------------------------------------- User Config Restore ---------------------------------------\r\n");
#endif
    /* set the mark bit to indicate user data restore */
    BLE_STOPPED_TASK_QUEUE |= (1 << BLE_STOPPED_TASK_CONFIG_RESTORE_POS);
  }

  RECEIVED_STRING_NUS[0] = 0;

  return;
}
#endif

/********************************************************************************/
void flash_data_write_task(void)
/*--------------------------------------------------------------------------------
| flash data write task
|
--------------------------------------------------------------------------------*/
{
  static uint32_t write_try_counter = 0;

  /* anothe flash write task is ongoing, return immediately */
  if (FLASH_UPDATE_WAITING_STAGE)
  {
#if (IF_LOG_OUTPUT)
        printf ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
        printf ("======================= flash write ongoing, stop write =======================\r\n");
        printf ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
#endif
    /* count the flash write try times */
    write_try_counter++;

    /* overtime, clear the flag, try again */
    if (write_try_counter > 100)
    {
      FLASH_UPDATE_WAITING_STAGE = 0;
      write_try_counter = 0;
    }
    return;
  }

  /* save the user config byte to flash */
  start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
  /* clear flash write flag */
  FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_TASK_OFFSET_POS));
}

/********************************************************************************/
void flash_data_write_check(void)
/*--------------------------------------------------------------------------------
| flash data write status check.
|  If the data in the RAM is not the same as in the flash, set the flag to re-write
| the data to the flash again
|  If the data in RAM and flash are the same, clear the flash write flag
|
--------------------------------------------------------------------------------*/
{
  static uint8_t check_try_counter = 0;

  /* flash write is ongoing, return immediately */
  if (FLASH_UPDATE_WAITING_STAGE)
  {
#if (IF_LOG_OUTPUT)
        printf ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
        printf ("======================= flash write ongoing, stop check =======================\r\n");
        printf ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
#endif
    check_try_counter++;
    if (check_try_counter > 10)
    {
      FLASH_UPDATE_WAITING_STAGE = 0;
      check_try_counter = 0;
    }
    return;
  }

  if (FLASH_DATA_WRITE_CHECK_TASK_QUEUE)
  {
    if (byte_string_comparison(FLASH_WRITE_DATA_SIZE_IN_WORD << 2, P_UINT8_FLASH_DATA_RAM_BUFFER, (uint8_t *)USER_CONFIG_FLASH_ADDR))
    {
#if (IF_LOG_OUTPUT)
      printf ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
      printf ("+++++++++++++++ Hi, the different value, flash write try again +++++++++++++++\r\n");
      printf ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
#endif
      /* set mark to falsh datat write try again */
      /* set flash write flag */
      FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
    }
    else
    {
      if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, BEL on/off flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_ON_OFF_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_ON_OFF_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_USER_PASSWORD_OFFSET_POS) & 1)
       {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, user password flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_USER_PASSWORD_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_PASSWORD_INPUT_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)PASSWD_CHANGED_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_SYSTEM_KEY_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, system key flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_SYSTEM_KEY_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_SYSTEM_KEY_INPUT_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SYATEM_KEY_UPDATED_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_LED_PATTERN_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, LED pattern flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_LED_PATTERN_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_LED_PATTERN_UPDATED_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)LED_PATTERN_UPDATED_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_SMART_USIM_DEVICE_NAME_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, smart USIM device name flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_SMART_USIM_DEVICE_NAME_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_SMART_USIM_TRAY_DEVICE_NAME_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)NAME_CHANGED_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_WIRELESS_SIM_ON_OFF_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, wireless SIM of/off setting flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_WIRELESS_SIM_ON_OFF_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_SIM_ON_OFF_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)WIRELESS_SIM_ON_OFF_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_USIM0_DATA_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, USIM0 EF data flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_USIM0_DATA_OFFSET_POS));
      }
#if (PIN_VCC_SIM1 != PIN_NULL)
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_USIM1_DATA_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, USIM1 EF data flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_USIM1_DATA_OFFSET_POS));
      }
#endif
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, default used USIM flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS));
#if (IF_SOFTDEIVE_USED)
        /*disable BLE stack */
        if (IF_SOFTDEVICE_RUNNING)
        {
          stop_ble_data( );
        }
#endif
        /* wireless USIM used */
        if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
        {
          /* mark the USAT bit for reset NAA, let phone re-access the network */
          CONNECTION_STATE |= 0x4;
          FLAG_USIM_SWITCH = 1;
        }
        /* local USIM used */
        else
        {
          /* if both USIM card inside, do switch USIM card */
          if ((((USIM_CARD_PRESENCE >> 0) & 0x01) == 1) && (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1))
          {
            if (DEFAULT_USED_USIM_RAM == 0)
            {
              PHONE_LOCAL_USED_USIM = 0x0b;
              set_default_usim0_etu( );
            }
#if (PIN_VCC_SIM1 != PIN_NULL)
            else if (DEFAULT_USED_USIM_RAM == 1)
            {
              PHONE_LOCAL_USED_USIM = 0x0e;

              set_default_usim1_etu( );
            }
#endif
            /* mark the USAT bit for reset NAA, let phone re-access the network */
            CONNECTION_STATE |= 0x4;
            FLAG_USIM_SWITCH = 1;
          }
        }
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_BLE_BOND_DELETE_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, BLE bond delete flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_BLE_BOND_DELETE_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_BOND_DELETE_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_BOND_DELETE_SUCCESSFULLY_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_LOGICAL_ADDRESS_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, logical address flash write completed =============\r\n");
        printf ("=========================== logical address = %x=======================================\r\n", (*((uint8_t*)DEVICE_LOG_ADD_FLASH_ADDR)) | 0x30);
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_LOGICAL_ADDRESS_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)LOGICAL_ADDRESS_SUCCESSFULLY_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_ICCID_2FE2_DATA_MODE_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, ICCID 2FE2 data mode flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_ICCID_2FE2_DATA_MODE_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)ICCID_2FE2_DATA_MODE_SUCCESSFULLY_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_WIRELESS_USIM_WORK_MODE_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, wireless USIM work mode flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_WIRELESS_USIM_WORK_MODE_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_0X88_COMMAND_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, BLE on/off for 0x88 flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_0X88_COMMAND_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_ON_OFF_0X88_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_ON_OFF_0X88_DISPLAY_TEXT);
      }
      else if ((FLASH_DATA_WRITE_CHECK_TASK_QUEUE >> FLASH_DATA_WRITE_CHECK_ICCID_2FE2_FIXED_DATA_OFFSET_POS) & 1)
      {
#if (IF_LOG_OUTPUT)
        printf ("===============================================================================\r\n");
        printf ("============= Hi, the same value, fixed ICCID data update flash write completed =============\r\n");
        printf ("===============================================================================\r\n");
#endif
        /* clear the mark bit for flash data write check */
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE &= (~(1 << FLASH_DATA_WRITE_CHECK_ICCID_2FE2_FIXED_DATA_OFFSET_POS));

        /* set USAT command mark to display text on phone screen */
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_UPDATE_ICCID_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)FIXED_ICCID_DATA_DISPLAY_TEXT);
      }
      else
      {
        FLASH_DATA_WRITE_CHECK_TASK_QUEUE = 0;
      }
    }
  }
}

/********************************************************************************/
void phone_data_send_check(void)
/*--------------------------------------------------------------------------------
| phone data send task check
|
--------------------------------------------------------------------------------*/
{
  uint8_t flag_bytes[4] = {0, 0, 0, 0};
  static uint8_t send_try_counter = 0;
  uint32_t i;

  if ((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1)
  {
    /* count the data send times */
    send_try_counter++;

    /* send date try overtime, give up to send the data clear the task queue */
    if (send_try_counter > 100)
    {
      send_try_counter = 0;
      USIM_PHONE_CONNECTION_COMMAND &= (~(1 << CONFIG_SYNCH_POS));
      return;
    }

#if (IF_SOFTDEIVE_USED)
    /* stop BLE stack */
    if (IF_SOFTDEVICE_RUNNING)
    {
      stop_ble_data( );
    }
#endif

    if (!phone_command_send_data(flag_bytes, 1, CONFIG_SYCH_REQUEST_DATA_TYPE))
    {
      /* clear the flag for config sync */
      send_try_counter = 0;
      USIM_PHONE_CONNECTION_COMMAND &= (~(1 << CONFIG_SYNCH_POS));
#if (IF_LOG_OUTPUT)
      printf("-------------------- Config synch request send & recive successfully, USIM_PHONE_CONNECTION_COMMAND = 0x%X ----------------------\r\n", (uint8_t)USIM_PHONE_CONNECTION_COMMAND);
#endif

      /* save the received config to data RAM buffer */
      for (i=0; i<*(GLOBAL_TMP_RAM + 0); i++)
      {
        *(P_UINT8_FLASH_DATA_RAM_BUFFER + USER_CONFIG_OFFSET_BYTES + i) = *(GLOBAL_TMP_RAM + 1 + i);
      }
      /* restore the USIM data from flash backup data */
      for (i=USIM0_EF_OFFSET_BYTES; i<(FLASH_WRITE_DATA_SIZE_IN_WORD << 2); i++)
      {
        *(P_UINT8_FLASH_DATA_RAM_BUFFER + USER_CONFIG_OFFSET_BYTES + i) = *((uint8_t *)BACKUP_DATA_USER_CONFIG_FLASH_ADDR + i);
      }

      /* save the user config RAM buffer byte to flash */
      start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);

      /* update user config */
      update_user_configuration( );
    }

    return;
  }

}

#ifdef PIN_LED_BLUE
/********************************************************************************/
void led_wireless_sim_status_indicator(uint32_t led_status_counter)
/*--------------------------------------------------------------------------------
| LED indicator for wireless USIM connection status
|
--------------------------------------------------------------------------------*/
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
    if (WIRELESS_SIM_CONNECTION_STATUS)
    {
      nrf_gpio_pin_write(PIN_LED_RED, 1);
    }
    else
    {
      if (led_status_counter % 8)
      {
        nrf_gpio_pin_write(PIN_LED_RED, 0);
      }
      else
      {
        nrf_gpio_pin_write(PIN_LED_RED, 1);
      }
    }
#endif
}

/********************************************************************************/
void led_flash_pattern_1(uint32_t led_status_counter, uint8_t speed_factor,
     uint8_t flash_pattern)
/*--------------------------------------------------------------------------------
| LED flash pattern
|
--------------------------------------------------------------------------------*/
{
  if ((led_status_counter % (4 * speed_factor)) == (0 * speed_factor))
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if ((led_status_counter % (4 * speed_factor)) == (1 * speed_factor))
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if ((led_status_counter % (4 * speed_factor)) == (2 * speed_factor))
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if ((led_status_counter % (4 * speed_factor)) == (3 * speed_factor))
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 1);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (flash_pattern)
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
}

/********************************************************************************/
void led_flash_pattern_all(uint32_t led_status_counter, uint8_t speed_factor)
/*--------------------------------------------------------------------------------
| LED flash pattern
|
--------------------------------------------------------------------------------*/
{
  if (led_status_counter % (2 * speed_factor))
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 1);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 1);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 1);
#endif
  }
}

/********************************************************************************/
void led_flash_pattern_bar(uint8_t speed_factor)
/*--------------------------------------------------------------------------------
| LED flash pattern
|
--------------------------------------------------------------------------------*/
{
  if (speed_factor == 'a')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'b')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_toggle(PIN_LED_BLUE);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'c')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_toggle(PIN_LED_GREEN);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'd')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_toggle(PIN_LED_WHITE);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'e')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_toggle(PIN_LED_RED);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'f')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_toggle(PIN_LED_GREEN);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'g')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_toggle(PIN_LED_WHITE);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else if (speed_factor == 'h')
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_toggle(PIN_LED_RED);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
  }
  else
  {
#ifdef PIN_LED_BLUE
    nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
    nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_WHITE
    nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_RED
    nrf_gpio_pin_write(PIN_LED_RED, 1);
#endif
#ifdef PIN_LED_YELLOW
    nrf_gpio_pin_write(PIN_LED_YELLOW, 1);
#endif
#ifdef PIN_LED_ORANGE
    nrf_gpio_pin_write(PIN_LED_ORANGE, 1);
#endif
  }
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
void led_ancs_indicator(uint32_t led_status_counter)
/*--------------------------------------------------------------------------------
| LED indicator for Apple ANCS status
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  for (i=0; i<ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER; i++)
  {
    if (ANCS_NOTIF_LED_PATTERN[i])
    {
      if (i == ANCS_NOTIF_LED_PATTERN_TYPE_INCOMING_CALL)
      {
        if ((led_status_counter % 4) == 0)
        {
#ifdef PIN_LED_BLUE
          nrf_gpio_pin_write(PIN_LED_BLUE, 1);
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
        }
        else if ((led_status_counter % 4) == 1)
        {
#ifdef PIN_LED_BLUE
          nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
          nrf_gpio_pin_write(PIN_LED_GREEN, 1);
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
        }
        else if ((led_status_counter % 4) == 2)
        {
#ifdef PIN_LED_BLUE
          nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
          nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
          nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_YELLOW
          nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
          nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
#ifdef PIN_LED_RED
          nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
        }
        else if ((led_status_counter % 4) == 3)
        {
#ifdef PIN_LED_BLUE
          nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_GREEN
          nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_WHITE
          nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
#ifdef PIN_LED_RED
          nrf_gpio_pin_write(PIN_LED_RED, 1);
#endif
#ifdef PIN_LED_YELLOW
          nrf_gpio_pin_write(PIN_LED_YELLOW, 0);
#endif
#ifdef PIN_LED_ORANGE
          nrf_gpio_pin_write(PIN_LED_ORANGE, 0);
#endif
        }

        i = ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER;
      }
      else if (i == ANCS_NOTIF_LED_PATTERN_TYPE_MISSED_CALL)
      {
#ifdef PIN_LED_BLUE
        nrf_gpio_pin_write(PIN_LED_BLUE, 1);
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
        i = ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER;
      }
      else if (i == ANCS_NOTIF_LED_PATTERN_TYPE_SOCIAL)
      {
#ifdef PIN_LED_BLUE
        nrf_gpio_pin_toggle(PIN_LED_BLUE);
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
        i = ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER;
      }
      else if (i == ANCS_NOTIF_LED_PATTERN_TYPE_EMAIL)
      {
#ifdef PIN_LED_BLUE
        if (led_status_counter % 8)
        {
          nrf_gpio_pin_write(PIN_LED_BLUE, 0);
        }
        else
        {
          nrf_gpio_pin_write(PIN_LED_BLUE, 1);
        }
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

        i = ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER;
      }
      else
      {
#ifdef PIN_LED_BLUE
        if (led_status_counter % 16)
        {
          nrf_gpio_pin_write(PIN_LED_BLUE, 0);
        }
        else
        {
          nrf_gpio_pin_write(PIN_LED_BLUE, 1);
        }
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

        i = ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER;
      }
    }
  }
}
#endif

/********************************************************************************/
void led_status_check(void)
/*--------------------------------------------------------------------------------
| LED status check according the LED configuration
|
--------------------------------------------------------------------------------*/
{
  static uint32_t led_status_counter = 0;
#if (IF_SOFTDEIVE_USED)
  uint32_t i;
  uint8_t led_pattern_total_tmp = 0;
#endif

  led_status_counter++;

  if (*(LED_PATTERN_BLE_FLASH + 1) == '0')
  {
    nrf_gpio_pin_write(PIN_LED_BLUE, 0);
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
  }
  else if (*(LED_PATTERN_BLE_FLASH + 1) == '1')
  {
    led_wireless_sim_status_indicator(led_status_counter);
  }
#if (IF_SOFTDEIVE_USED)
  else if (*(LED_PATTERN_BLE_FLASH + 1) == '2')
  {
    for (i=0; i<ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER; i++)
    {
      led_pattern_total_tmp += ANCS_NOTIF_LED_PATTERN[i];
    }

    nrf_gpio_pin_write(PIN_LED_BLUE, 0);

#ifdef PIN_LED_GREEN
    if (led_pattern_total_tmp)
    {
      led_ancs_indicator(led_status_counter);
    }
    else
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
#ifdef PIN_LED_RED
      nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_WHITE
      nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
    }
#endif
  }
  else if (*(LED_PATTERN_BLE_FLASH + 1) == '3')
  {
    for (i=0; i<ANCS_NOTIF_LED_PATTERN_TYPE_TOTAL_NUMBER; i++)
    {
      led_pattern_total_tmp += ANCS_NOTIF_LED_PATTERN[i];
    }

    if (led_pattern_total_tmp)
    {
      led_ancs_indicator(led_status_counter);
    }
    else
    {
      led_wireless_sim_status_indicator(led_status_counter);
    }
  }
#endif
  else if (*(LED_PATTERN_BLE_FLASH + 1) == '8')
  {
    switch (*(LED_PATTERN_BLE_FLASH + 2))
    {
      case '0':
      case '1':
      case '2':
      {
        led_flash_pattern_1(led_status_counter, 1 << (*(LED_PATTERN_BLE_FLASH + 2) & 0xf), 1);

        break;
      }
      case '3':
      case '4':
      case '5':
      {
        led_flash_pattern_1(led_status_counter, 1 << ((*(LED_PATTERN_BLE_FLASH + 2) & 0xf) - 2), 0);

        break;
      }
      case '6':
      case '7':
      case '8':
      case '9':
      {
        led_flash_pattern_all(led_status_counter, 1 << ((*(LED_PATTERN_BLE_FLASH + 2) & 0xf) - 6));

        break;
      }

      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      {
        led_flash_pattern_bar(*(LED_PATTERN_BLE_FLASH + 2));

        break;
      }

      default:
      {
        break;
      }
    }
  }
  else if (*(LED_PATTERN_BLE_FLASH + 1) == '9')
  {
    led_flash_pattern_1(led_status_counter, 2, 1);
  }
  else
  {
    #define led_flash_starting_length  8
    uint8_t char_led;
    uint8_t bit_led_in_total;
    uint8_t bit_led_in_char;
    uint8_t led_flash_cycle;
    uint8_t led_string_length_bits = (*(uint8_t *)LED_PATTERN_FLASH_ADDR - 1) * 8 + led_flash_starting_length;

    if ((*(LED_PATTERN_BLE_FLASH + 1) == 'A') || (*(LED_PATTERN_BLE_FLASH + 1) == 'a'))
    {
      led_flash_cycle = 2;
    }
    else if ((*(LED_PATTERN_BLE_FLASH + 1) == 'B') || (*(LED_PATTERN_BLE_FLASH + 1) == 'b'))
    {
      led_flash_cycle = 4;
    }
    else if ((*(LED_PATTERN_BLE_FLASH + 1) == 'C') || (*(LED_PATTERN_BLE_FLASH + 1) == 'c'))
    {
      led_flash_cycle = 8;
    }
    else if ((*(LED_PATTERN_BLE_FLASH + 1) == 'D') || (*(LED_PATTERN_BLE_FLASH + 1) == 'd'))
    {
      led_flash_cycle = 16;
    }
    else if ((*(LED_PATTERN_BLE_FLASH + 1) == 'E') || (*(LED_PATTERN_BLE_FLASH + 1) == 'e'))
    {
      led_flash_cycle = 32;
    }
    else if ((*(LED_PATTERN_BLE_FLASH + 1) == 'F') || (*(LED_PATTERN_BLE_FLASH + 1) == 'f'))
    {
      led_flash_cycle = 64;
    }
    else if ((*(LED_PATTERN_BLE_FLASH + 1) == 'G') || (*(LED_PATTERN_BLE_FLASH + 1) == 'g'))
    {
      led_flash_cycle = 128;
    }
    else
    {
      led_flash_cycle = 255;
    }

    if (led_status_counter % led_flash_cycle)
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
#ifdef PIN_LED_RED
      nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_WHITE
      nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
    }
    else
    {
      bit_led_in_total = (led_status_counter / led_flash_cycle) % led_string_length_bits;
      /* start LED flash */
      if (bit_led_in_total < led_flash_starting_length)
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
#ifdef PIN_LED_RED
      nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_WHITE
      nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
      }
      else
      {
        char_led = (bit_led_in_total - led_flash_starting_length) / 8;
        bit_led_in_char = (bit_led_in_total - led_flash_starting_length) % 8;

        if (((*(LED_PATTERN_BLE_FLASH + char_led + 2)) << bit_led_in_char) & 0x80)
        {
          if((led_status_counter / led_flash_cycle) % 2)
          {
#ifdef PIN_LED_BLUE
            nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN
            nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_RED
            nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_WHITE
            nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
          }
          else
          {
#ifdef PIN_LED_RED
            nrf_gpio_pin_write(PIN_LED_RED, 1);
#endif
#ifdef PIN_LED_GREEN
            nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
#ifdef PIN_LED_BLUE
            nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_WHITE
            nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
          }
        }
        else
        {
          if((led_status_counter / led_flash_cycle) % 2)
          {
#ifdef PIN_LED_GREEN
            nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_BLUE
            nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_RED
            nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_WHITE
            nrf_gpio_pin_write(PIN_LED_WHITE, 0);
#endif
          }
          else
          {
#ifdef PIN_LED_WHITE
            nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_BLUE
            nrf_gpio_pin_write(PIN_LED_BLUE, 0);
#endif
#ifdef PIN_LED_RED
            nrf_gpio_pin_write(PIN_LED_RED, 0);
#endif
#ifdef PIN_LED_GREEN
            nrf_gpio_pin_write(PIN_LED_GREEN, 0);
#endif
          }
        }
      }
    }
  }
}
#endif

/********************************************************************************/
void usim_power_on_status_check(void)
/*--------------------------------------------------------------------------------
| USIM initial power on and EF 0x7fff slection status check
|
--------------------------------------------------------------------------------*/
{
  /* USIM is not detected */
#if (PIN_VCC_SIM1 != PIN_NULL)
  if ((((PHONE_LOCAL_USED_USIM >> 1) & 1) == 0) || (((PHONE_LOCAL_USED_USIM >> 3) & 1) == 0))
#else
  if (((PHONE_LOCAL_USED_USIM >> 1) & 1) == 0)
#endif
  {
#if (IF_SOFTDEIVE_USED)
    if (IF_SOFTDEVICE_RUNNING)
    {
      /* disable softdevice */
      stop_ble_data();
    }
#endif

    usim_initial_power_on( );
  }
  else
  {
    /* if USIM0 existed */
    if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
    {
      if ((SELECTED_FILE_USIM0 != 0x7fff) && ((SELECTED_FILE_USIM0 >> 16) != 0x7fff))
      {
#if (IF_SOFTDEIVE_USED)
        if (IF_SOFTDEVICE_RUNNING)
        {
          /* disable softdevice */
          stop_ble_data();
        }
#endif

        set_default_usim0_etu( );

        /* start ETU timer */
        NRF_TIMER1->TASKS_START = 1;
        /* start sim clock signal */
        clock_sim_start_4m(PIN_CLOCK_SIM);
        /* EF 0x7FFF selection */
        sim_file_7fff_selected_check( );

        /* stop SIM clock signal and timer 1*/
        stop_sim_clock_timer1( );
      }
#if (IF_LOG_OUTPUT)
      else
      {
        printf("================== USIM0 ");
        printf_selected_file(SELECTED_FILE_USIM0);
      }
#endif
    }
#if (IF_LOG_OUTPUT)
    else
    {
      printf("++++++++++++++++++ USIM0 not existed ++++++++++++++++++\r\n");
    }
#endif
#if (PIN_VCC_SIM1 != PIN_NULL)
    /* if USIM1 existed */
    if (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1)
    {
      if ((SELECTED_FILE_USIM1 != 0x7fff) && ((SELECTED_FILE_USIM1 >> 16) != 0x7fff))
      {
#if (IF_SOFTDEIVE_USED)
        if (IF_SOFTDEVICE_RUNNING)
        {
          /* disable softdevice */
          stop_ble_data();
        }
#endif

        set_default_usim1_etu( );

        /* start ETU timer */
        NRF_TIMER1->TASKS_START = 1;
        /* start sim clock signal */
        clock_sim_start_4m(PIN_CLOCK_SIM);
        /* EF 0x7FFF selection */
        sim_file_7fff_selected_check( );
        /* stop SIM clock signal and timer 1*/
        stop_sim_clock_timer1( );
      }
#if (IF_LOG_OUTPUT)
      else
      {
        printf("================== USIM1 ");
        printf_selected_file(SELECTED_FILE_USIM1);
      }
#endif
    }
#if (IF_LOG_OUTPUT)
    else
    {
      printf("++++++++++++++++++ USIM1 not existed ++++++++++++++++++\r\n");
    }
#endif
#endif
  }
}

/********************************************************************************/
void usim_file_update_status_check(void)
/*--------------------------------------------------------------------------------
| USIM EF data flash write status check
|
--------------------------------------------------------------------------------*/
{
  /* wireless USIM used */
  if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
  {
    if (((IF_USIM_FILE_DATA_UPDATED >> 2) & 1) == 0)
    {
#if (IF_SOFTDEIVE_USED)
      /* stop BLE softdevice */
      if (IF_SOFTDEVICE_RUNNING)
      {
        stop_ble_data( );
      }
#endif
      phone_command_get_file_data(0xffff, USIM0_EF_DATA_RAM);
      IF_USIM_FILE_DATA_UPDATED |= (1 << 2);
    }
    else if (((IF_USIM_FILE_DATA_UPDATED >> 3) & 1) == 0)
    {
#if (IF_SOFTDEIVE_USED)
      /* stop BLE softdevice */
      if (IF_SOFTDEVICE_RUNNING)
      {
        stop_ble_data( );
      }
#endif
      phone_command_get_file_data(0xffee, USIM1_EF_DATA_RAM);
      IF_USIM_FILE_DATA_UPDATED |= (1 << 3);
    }
  }
  /* local USIM used */
  else
  {
    /* USIM0 detectione is already done */
    if (((PHONE_LOCAL_USED_USIM >> 1) & 1) == 1)
    {
      /* if USIM0 is presented */
      if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
      {
        /* USIM0 file data not updated */
        if (((IF_USIM_FILE_DATA_UPDATED >> 0) & 1) == 0)
        {
#if (IF_SOFTDEIVE_USED)
          /* stop BLE softdevice */
          if (IF_SOFTDEVICE_RUNNING)
          {
            stop_ble_data( );
          }
#endif
          /* start ETU timer */
          NRF_TIMER1->TASKS_START = 1;
          clock_sim_start_4m(PIN_CLOCK_SIM0);

          /* update USIM0 file data and save to flash */
          if (!usim_files_data_read_flash_write(0, USIM0_EF_DATA_RAM))
          {
            /* clear the flag bit */
            IF_USIM_FILE_DATA_UPDATED |= (1 << 0);
          }

          /* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
        }
      }
    }
#if (PIN_VCC_SIM1 != PIN_NULL)
    /* USIM1 detectione is already done and USIM1 file data not updated */
    if (((PHONE_LOCAL_USED_USIM >> 3) & 1) == 1)
    {
      /* if USIM1 is presented */
      if (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1)
      {
        /* USIM1 file data not updated */
        if(((IF_USIM_FILE_DATA_UPDATED >> 1) & 1) == 0)
        {
#if (IF_SOFTDEIVE_USED)
          /* stop BLE softdevice */
          if (IF_SOFTDEVICE_RUNNING)
          {
            stop_ble_data( );
          }
#endif
          /* start ETU timer */
          NRF_TIMER1->TASKS_START = 1;
          clock_sim_start_4m(PIN_CLOCK_SIM1);

          /* update USIM1 file data and save to flash */
          if (!usim_files_data_read_flash_write(1, USIM1_EF_DATA_RAM))
          {
            /* clear the flag bit */
            IF_USIM_FILE_DATA_UPDATED |= (1 << 1);
          }

          /* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
        }
      }
    }
#endif
  }
}

/********************************************************************************/
void ble_stop_task_queue_check(void)
/*--------------------------------------------------------------------------------
| task queue to be done require BLE stack stopped
|
--------------------------------------------------------------------------------*/
{
  /* if there is task queue require BLE stop */
  if (BLE_STOPPED_TASK_QUEUE)
  {
#if (IF_SOFTDEIVE_USED)
    /* stop BLE softdevice */
    if (IF_SOFTDEVICE_RUNNING)
    {
      stop_ble_data( );
    }
#endif

    /* restore config data */
    if ((BLE_STOPPED_TASK_QUEUE >> BLE_STOPPED_TASK_CONFIG_RESTORE_POS) & 1)
    {
      if (!usim_data_restore( ))
      {
        /* clear the mark bit */
        BLE_STOPPED_TASK_QUEUE &= (~(1 << BLE_STOPPED_TASK_CONFIG_RESTORE_POS));

        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_CONFIG_RESTORE_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)CONFIG_RESTORE_SUCCESSFULLY_DISPLAY_TEXT);
      }
    }
    /* update USIM0 EF data */
    else if ((BLE_STOPPED_TASK_QUEUE >> BLE_STOPPED_TASK_USIM0_EF_UPDATE_POS) & 1)
    {
      /* start ETU timer */
      NRF_TIMER1->TASKS_START = 1;
      clock_sim_start_4m(PIN_CLOCK_SIM0);

      /* update USIM0 file data and save to flash */
      if (!usim_files_data_read_flash_write(0, USIM0_EF_DATA_RAM))
      {
        /* clear the mark bit */
        BLE_STOPPED_TASK_QUEUE &= (~(1 << BLE_STOPPED_TASK_USIM0_EF_UPDATE_POS));

        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM0_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT);
      }

      /* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
    }
#if (PIN_VCC_SIM1 != PIN_NULL)
    /* update USIM1 EF data */
    else if ((BLE_STOPPED_TASK_QUEUE >> BLE_STOPPED_TASK_USIM1_EF_UPDATE_POS) & 1)
    {
      /* start ETU timer */
      NRF_TIMER1->TASKS_START = 1;
      clock_sim_start_4m(PIN_CLOCK_SIM1);

      /* update USIM1 file data and save to flash */
      if (!usim_files_data_read_flash_write(1, USIM1_EF_DATA_RAM))
      {
        /* clear the mark bit */
        BLE_STOPPED_TASK_QUEUE &= (~(1 << BLE_STOPPED_TASK_USIM1_EF_UPDATE_POS));

        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_SUCCESSFULLY_TEXT_DISPLAY;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM1_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT);
      }

      /* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
    }
#endif
  }
}

/********************************************************************************/
void idle_mode_text_diaplay_status_check(void)
/*--------------------------------------------------------------------------------
| update idle text display to indicate the wireless USIM interface status
|
--------------------------------------------------------------------------------*/
{
  /* return immediately if there are USAT command pending or Config sync in progress */
  if ((USAT_BYTE_LENGTH_BACK) || ((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1))
  {
    return;
  }

  /* text display */
  if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0) /* wireless USIM used */
  {
    /* wireless USIM on */
    if (WIRELESS_SIM_ON_OFF_RAM)
    {
      /* wireless USIM connected */
      if (WIRELESS_SIM_CONNECTION_STATUS)
      {
        /* USIM0 used */
        if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
        {
          if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_SIM_ON_USIM0_CONNECTED_CLIENT)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_CONNECTED_CLIENT;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM0_CONNECTED_STRING);
          }
        }
        /* USIM1 used */
        else
        {
          if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_SIM_ON_USIM1_CONNECTED_CLIENT)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_CONNECTED_CLIENT;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM1_CONNECTED_STRING);
          }
        }
      }
      /* wireless USIM disconnected */
      else
      {
        /* USIM0 used */
        if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
        {
          if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_SIM_ON_USIM0_DISCONNECTED_CLIENT)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_DISCONNECTED_CLIENT;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT);
          }
        }
        /* USIM1 used */
        else
        {
          if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_SIM_ON_USIM1_DISCONNECTED_CLIENT)
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_DISCONNECTED_CLIENT;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT);
          }
        }
      }
    }
    /* woreless USIM off */
    else
    {
      if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_SIM_OFF_ADDRESS_CLIENT)
      {
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_OFF_ADDRESS_CLIENT;
        USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT);
      }
    }
  }
  else /* local USIM used */
  {
    /* set idle mode text to indicate Wireless SIM ON/OFF state */
    /* wireless USIM on */
    if (WIRELESS_SIM_ON_OFF_RAM)
    {
      /* USIM0 used locallly */
      if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
      {
        if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_WIRELESS_USIM0_SIM_ON_SERVER)
        {
          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_ON_SERVER;
          USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM0_DISPLAY_TEXT);
        }
      }
      /* USIM1 used locallly */
      else
      {
        if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_WIRELESS_USIM1_SIM_ON_SERVER)
        {
          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_ON_SERVER;
          USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM1_DISPLAY_TEXT);
        }
      }
    }
    /* wireless USIM off */
    else
    {
      /* USIM0 used locallly */
      if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
      {
        if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_WIRELESS_USIM0_SIM_OFF_SERVER)
        {
          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_OFF_SERVER;
          USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM0_DISPLAY_TEXT);
        }
      }
      /* USIM1 used locallly */
      else
      {
        if (IDLE_MODE_TEXT_TYPE != IDLE_MODE_TEXT_TYPE_WIRELESS_USIM1_SIM_OFF_SERVER)
        {
          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_OFF_SERVER;
          USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM1_DISPLAY_TEXT);
        }
      }
    }
  }
}

/********************************************************************************/
void config_sych_setting_check(void)
/*--------------------------------------------------------------------------------
| user Config sych setting status check
|
--------------------------------------------------------------------------------*/
{
  static uint8_t config_sych_setting_counter = 0;

  if ((USIM_PHONE_CONNECTION_COMMAND >> CONFIG_SYNCH_POS) & 1)
  {
    /* increse the counter */
    config_sych_setting_counter++;
    if (config_sych_setting_counter > 9)
    {
      /* clear the counter and connection command indicator */
      config_sych_setting_counter = 0;
      USIM_PHONE_CONNECTION_COMMAND &= (~(1 << CONFIG_SYNCH_POS));

      /* update idle text for USIM server mode side */
      if ((PHONE_LOCAL_USED_USIM & 0xF0) == 0)
      {
        /* set idle mode text to indicate Wireless SIM ON/OFF state */
        if (WIRELESS_SIM_ON_OFF_RAM) /* wireless on */
        {
          if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1) /* USIM 0 */
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_ON_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM0_DISPLAY_TEXT);
          }
          else if (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1)/* USIM 1 */
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_ON_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM1_DISPLAY_TEXT);
          }
        }
        else /* wireless off */
        {
          if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1) /* USIM 0 */
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_OFF_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM0_DISPLAY_TEXT);
          }
          else if (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1) /* USIM 1 */
          {
            FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_OFF_SERVER;
            USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM1_DISPLAY_TEXT);
          }
        }
      }
    }
  }
}

#if (IF_USIM_BINARY_UPDATE)
/********************************************************************************/
uint32_t usim_binary_update_file_id(uint32_t update_binary_queue)
/*--------------------------------------------------------------------------------
| get the USIM file ID for binary update
|
--------------------------------------------------------------------------------*/
{
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F08_OFFSET_POS) & 1)
  {
    return(0x6f08);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F7E_OFFSET_POS) & 1)
  {
    return(0x6f7e);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F09_OFFSET_POS) & 1)
  {
    return(0x6f09);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F5B_OFFSET_POS) & 1)
  {
    return(0x6f5b);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F73_OFFSET_POS) & 1)
  {
    return(0x6f73);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F7B_OFFSET_POS) & 1)
  {
    return(0x6f7b);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_4F20_OFFSET_POS) & 1)
  {
    return(0x4f20);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_4F52_OFFSET_POS) & 1)
  {
    return(0x4f52);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F31_OFFSET_POS) & 1)
  {
    return(0x6f31);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F78_OFFSET_POS) & 1)
  {
    return(0x6f78);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6FAD_OFFSET_POS) & 1)
  {
    return(0x6fad);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F5C_OFFSET_POS) & 1)
  {
    return(0x6f5c);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6FC4_OFFSET_POS) & 1)
  {
    return(0x6fc4);
  }
  if ((update_binary_queue >> SIM_FILE_UPDATE_BINARY_6F56_OFFSET_POS) & 1)
  {
    return(0x6f56);
  }

  return(1);
}

/********************************************************************************/
uint32_t usim_binary_update_file_id_clear(uint32_t file_id)
/*--------------------------------------------------------------------------------
| clear the USIM file ID after USIM binary update
|
--------------------------------------------------------------------------------*/
{
  switch (file_id)
  {
    case 0x6f08:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F08_OFFSET_POS));
      break;
    }

    case 0x6f7e:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F7E_OFFSET_POS));
      break;
    }

    case 0x6f09:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F09_OFFSET_POS));
      break;
    }

    case 0x6f5b:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F5B_OFFSET_POS));
      break;
    }

    case 0x6f73:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F73_OFFSET_POS));
      break;
    }

    case 0x6f7b:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F7B_OFFSET_POS));
      break;
    }

    case 0x4f20:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_4F20_OFFSET_POS));
      break;
    }

    case 0x4f52:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_4F52_OFFSET_POS));
      break;
    }

    case 0x6f31:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F31_OFFSET_POS));
      break;
    }

    case 0x6f78:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F78_OFFSET_POS));
      break;
    }

    case 0x6fad:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6FAD_OFFSET_POS));
      break;
    }

    case 0x6f5c:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F5C_OFFSET_POS));
      break;
    }

    case 0x6fc4:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6FC4_OFFSET_POS));
      break;
    }

    case 0x6f56:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE &= (~(0x1 << SIM_FILE_UPDATE_BINARY_6F56_OFFSET_POS));
      break;
    }
  }

  /* clear the binary updaet mark */
  BINARY_UPDATE_FILE_ID = 0;

  return(0);
}

/********************************************************************************/
void usim_binary_update_check(void)
/*--------------------------------------------------------------------------------
| USIM file binary update check for local USIM used
|
--------------------------------------------------------------------------------*/
{
  static uint8_t update_try_counter = 0;

  if (BINARY_UPDATE_FILE_ID == 0) /* no task ongoing, start a new binary update task */
  {
    BINARY_UPDATE_FILE_ID = usim_binary_update_file_id(SIM_FILE_UPDATE_BINARY_TASK_QUEUE);
    update_try_counter = 0;
  }
  else /* some binary update task ongoing */
  {
    update_try_counter++;

    /* wait time too long, clear the counter, start a new task */
    if (update_try_counter > 10)
    {
      update_try_counter = 0;
      BINARY_UPDATE_FILE_ID = 0;
    }

    return;
  }

#if (IF_SOFTDEIVE_USED)
  if (IF_SOFTDEVICE_RUNNING)
  {
    /* request time sot for USIM card file update */
    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_CARD_BINARY_UPDATE;
    SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage0;

#if (IF_LOG_OUTPUT)
    printf("\r\n====== time slot request for USIM0 card binary update,  SIM_FILE_UPDATE_BINARY_TASK_QUEUE = 0x%X, SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE = %d, update_file_id = 0x%2X, %2X -----------------\r\n",  (uint8_t)SIM_FILE_UPDATE_BINARY_TASK_QUEUE, SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE, (uint8_t)(BINARY_UPDATE_FILE_ID >> 8), (uint8_t)BINARY_UPDATE_FILE_ID);
#endif

    time_slot_request(TIMER_SLOT_USIM_CARD_BINARY_UPDATE_US);
  }
  else
#endif/* end of #if (IF_SOFTDEIVE_USED) */
  {
#if (IF_LOG_OUTPUT)
    printf("\r\n====== USIM card binary update,  SIM_FILE_UPDATE_BINARY_TASK_QUEUE = 0x%X, update_file_id = 0x%2X, %2X -----------------\r\n",  (uint8_t)SIM_FILE_UPDATE_BINARY_TASK_QUEUE, (uint8_t)(BINARY_UPDATE_FILE_ID >> 8), (uint8_t)BINARY_UPDATE_FILE_ID);
#endif
    if (!sim_command_usim_card_update_binary_0xd6(BINARY_UPDATE_FILE_ID))
    {
      usim_binary_update_file_id_clear(BINARY_UPDATE_FILE_ID);
      /* USIM card EF binary update task queue completed */
      if (SIM_FILE_UPDATE_BINARY_TASK_QUEUE == 0)
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
      }
    }
  }
}

/********************************************************************************/
uint32_t usim_binary_update_piggyback(uint8_t *piggyback_data)
/*--------------------------------------------------------------------------------
| USIM file binary update phone wireless piggyback data prepare at wireless USIM client
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t file_size = 0;
  uint32_t file_offset = 0;
  uint32_t flag_offset = 0;
  uint16_t piggyback_data_file_id = 0;

  BINARY_UPDATE_FILE_ID = usim_binary_update_file_id(SIM_FILE_UPDATE_BINARY_TASK_QUEUE);

  /* piggyback data for command of EF bibary update */
  *(piggyback_data + 0) = PHONE_CONNECTION_PIGGYBACK_UPDATE_BINARY;
  *(piggyback_data + 1) = PHONE_CONNECTION_PIGGYBACK_UPDATE_BINARY;

  /* only some EF data are updated by wireless SIM piggyback back */
  if (BINARY_UPDATE_FILE_ID == 0x6f08)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F08_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f7e)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F7E_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f09)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F09_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f5b)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F5B_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f73)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F73_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f7b)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F7B_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x4f20)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_4F20_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x4f52)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_4F52_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f31)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F31_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f78)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F78_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6fad)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6FAD_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f5c)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F5C_OFFSET_POS);
  }
  else if (BINARY_UPDATE_FILE_ID == 0x6f56)
  {
    piggyback_data_file_id = (0x1 << SIM_FILE_UPDATE_BINARY_6F56_OFFSET_POS);
  }
  else /* USIM EF not updated, the updated file size <= 34 (64 - 4 - 26) */
  {
    *(piggyback_data + 0) = 0;
    *(piggyback_data + 1) = 0;
    *(piggyback_data + 2) = 0;
    *(piggyback_data + 3) = 0;

    usim_binary_update_file_id_clear(BINARY_UPDATE_FILE_ID);

    return (1);
  }

  /* piggyback data for file ID */
  *(piggyback_data + 2) = piggyback_data_file_id & 0xFF;
  *(piggyback_data + 3) = (piggyback_data_file_id >> 8) & 0xFF;

  /* piggyback data for file content */
  get_usim_file_position_index(BINARY_UPDATE_FILE_ID, &file_size, &file_offset, &flag_offset);
  if (PHONE_WIRELESS_USED_USIM == 0) /* wireless USIM0 used */
  {
    for (i=0; i<file_size; i++)
    {
      *(piggyback_data + 4 + i) = *(USIM0_EF_DATA_RAM + file_offset + i);
    }
  }
  else /* wireless USIM1 used */
  {
    for (i=0; i<file_size; i++)
    {
      *(piggyback_data + 4 + i) = *(USIM1_EF_DATA_RAM + file_offset + i);
    }
  }

  return (0);
}

/********************************************************************************/
void usim_binary_update_wireless_connection(uint8_t *piggyback_data)
/*--------------------------------------------------------------------------------
| USIM file binary update phone wireless piggyback data at wireless USIM server
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t update_file_id;
  uint8_t file_size = 0;
  uint32_t file_offset = 0;
  uint32_t flag_offset = 0;

  update_file_id = usim_binary_update_file_id(((*(piggyback_data + 3)) << 8) | (*(piggyback_data + 2)));

  get_usim_file_position_index(update_file_id, &file_size, &file_offset, &flag_offset);

  /* update the received EF data from wireless SIM to RAM data */
  if ((PHONE_WIRELESS_USED_USIM & 1) == 0) /* USIM0 is used by wireless SIM */
  {
    for (i=0; i<file_size; i++)
    {
      *(USIM0_EF_DATA_RAM + file_offset + i) = *(piggyback_data + 4 + i);
    }
  }
  else /* USIM1 is used by wireless SIM */
  {
    for (i=0; i<file_size; i++)
    {
      *(USIM1_EF_DATA_RAM + file_offset + i) = *(piggyback_data + 4 + i);
    }
  }

  switch (update_file_id)
  {
    case 0x6f08:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F08_OFFSET_POS);
      break;
    }

    case 0x6f09:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F09_OFFSET_POS);
      break;
    }

    case 0x6f7e:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F7E_OFFSET_POS);
      break;
    }

    case 0x6f5b:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F5B_OFFSET_POS);
      break;
    }

    case 0x6f73:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F73_OFFSET_POS);
      break;
    }

    case 0x6f7b:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F7B_OFFSET_POS);
      break;
    }

    case 0x4f20:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_4F20_OFFSET_POS);
      break;
    }

    case 0x4f52:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_4F52_OFFSET_POS);
      break;
    }

    case 0x6f31:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F31_OFFSET_POS);
      break;
    }

    case 0x6f78:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F78_OFFSET_POS);
      break;
    }

    case 0x6fad:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6FAD_OFFSET_POS);
      break;
    }

    case 0x6f5c:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F5C_OFFSET_POS);
      break;
    }

    case 0x6f56:
    {
      SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << SIM_FILE_UPDATE_BINARY_6F56_OFFSET_POS);
      break;
    }

    default:
    {
#if (IF_LOG_OUTPUT)
      if (!IF_SOFTDEVICE_RUNNING)
      {
        printf("+++++++++++++++++++++++++++ binary update file ID error, file_ID = 0x%X, %X +++++++++++++++++++++++++++\r\n",
          (uint8_t)(update_file_id >> 8), (uint8_t)update_file_id);
      }
#endif
        break;
    }
  }

  return;
}

#endif


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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comm_initial.h"
#include "app_error.h"
#include "flash_file_address.h"
#include "comm_uicc_terminal_interface.h"
#include "comm_phone_command.h"
#include "nrf_soc.h"
#include "nrf_nvic.h"
#include "wireless_sim_phone.h"
#include "usim_server_sim_interface.h"
#include "ble_flash.h"
#include "global_data_exchange_extern.h"

/********************************************************************************/
uint32_t start_oscillator_16m(void)
/*--------------------------------------------------------------------------------
| initialization of 16M oscillators
|
--------------------------------------------------------------------------------*/
{
  uint32_t initial_timer;
  uint32_t osc_time_out_ms = 2;

  /* Start 16 MHz crystal oscillator */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  /* Wait for the external oscillator to start up */
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
  {
    /*  oscillator start up time out */
    if (((NRF_RTC2->COUNTER - initial_timer) > osc_time_out_ms) || (NRF_RTC2->COUNTER < initial_timer))
    {
#if (IF_LOG_OUTPUT)
      printf("+++++++++++++++++++++++++ 16M oscillator start time out +++++++++++++++++++++++++\r\n" );
#endif
      /* system reset */
      sd_nvic_SystemReset( );

      return (1);
    }
  }

  return(0);
}

/********************************************************************************/
__INLINE void stop_oscillator_16m(void)
/*--------------------------------------------------------------------------------
| Function for initialization 16M oscillators
|
--------------------------------------------------------------------------------*/
{
  /* Stop 16 MHz crystal oscillator */
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}

/********************************************************************************/
uint8_t byte_string_comparison(uint32_t byte_length, uint8_t *byte_1, uint8_t *byte_2)
/*--------------------------------------------------------------------------------
| 1: the 2 string are different
| 0: the 2 string are the same
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  for (i=0; i<byte_length; i++)
  {
    if (*(byte_1 + i) != *(byte_2 + i))
    {
      return(1);
    }
  }

  return(0);
}

/********************************************************************************/
uint8_t random_vector_generate(uint8_t *p_buff, uint8_t size, uint32_t start_time, uint32_t time_length, uint8_t if_bias_correction)
/*--------------------------------------------------------------------------------
| Function for getting vector of random numbers
| p_buff: Pointer to unit8_t buffer for storing the bytes
| length: Number of bytes to take from pool and place in p_buff
| start_time/time_length in us for BLE stack running, TIMER0 used( in time slot),
|         in ms for BLE not running, RTC2 used
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t time_now;

  NRF_RNG->CONFIG = if_bias_correction;
  NRF_RNG->TASKS_START = 1;

  for (i=0; i<size; i++)
  {
    NRF_RNG->EVENTS_VALRDY = 0;
    while(NRF_RNG->EVENTS_VALRDY == 0)
    {
      if (IF_SOFTDEVICE_RUNNING)
      {
        /* get the current timer0 */
        NRF_TIMER0->TASKS_CAPTURE[0] = 1;
        time_now = NRF_TIMER0->CC[0];
      }
      else
      {
        time_now = NRF_RTC2->COUNTER;
      }
      /*  random number generation ready time out */
      if ((time_now - start_time) > time_length)
      {
        return(1);
      }
    }
    *(p_buff + i) = (uint8_t)NRF_RNG->VALUE;
  }

  NRF_RNG->TASKS_STOP = 1;

  return(0);
}

/********************************************************************************/
void usim_data_initialization(void)
/*--------------------------------------------------------------------------------
| USIM EF data initialization, the USIM EF data is just as an example
|
--------------------------------------------------------------------------------*/
{
  uint8_t EF_data_USIM[ALL_USIM_EF_FLAG_SIZE] = {ALL_USIM_EF_SIZE, 0x98,  0x68,  0x10,  0x51,  0x18,  0x1,
             0x91,  0x37,  0x78,  0x76,  0xff,  0xff,  0xff, 0xff,
             0x8,  0x49,  0x6,  0x10,  0x40,  0x6,  0x3,  0x21,  0x87,
             0x2,  0x31,  0x15,  0x26,  0xeb,  0xb7,  0x69,
             0xf3,  0xc8,  0xcf,  0xdf,  0x44,  0xe0,  0x36,  0x45,  0xef,
             0x5e,  0x96,  0xf8,  0xc2,  0x75,  0xcb,  0x32,
             0x55,  0xd5,  0xb7,  0x39,  0x76,  0xc3,  0x48,  0x67,  0x7c,
             0x6b,  0x2,  0x15,  0x68,  0x8,  0x0,  0x77,
             0xec,  0x49,  0xae,  0xe1,  0x71,  0xda,  0xd1,  0x9c,  0xce,
             0x78,  0xb8,  0x28,  0xcb,  0xa8,  0x76,  0x6f,
             0xb3,  0x63,  0x0,  0xf,  0xf0,  0x61,  0xf2,  0x76,  0x32,  0xf0,
             0x32,  0xff,  0x0,  0x40,  0x9c,  0x1a,
             0x8f,  0xb2,  0x64,  0xf0,  0x10,  0x10,  0xd7,  0x0,  0x0,  0xe4,
             0x9,  0xeb,  0xc4,  0x9d,  0xf,  0x39,  0x64,
             0xf0,  0x10,  0xa5,  0x4d,  0x2,  0x0,  0x64,  0xf0,  0x0,  0x64,
             0xf0,  0x20,  0x64,  0xf0,  0x40,  0x64,
             0xf0,  0x70,  0x0,  0x0,  0x0,  0x2,  0xff,  0xff,  0xff,  0xf0,
             0x0,  0x2,  0xf0,  0x0,  0x2,  0xa0,  0x2e,
             0x80,  0x2,  0x23,  0xcd,  0x81,  0x28,  0x23,  0xbe,  0x23,  0xbf,
             0x23,  0xc1,  0x23,  0xc2,  0x23,  0xc4,
             0x23,  0xc6,  0x23,  0xc7,  0x23,  0xc8,  0x23,  0xc9,  0x23,  0xca,
             0x23,  0xce,  0x23,  0xcf,  0x23,  0xec,
             0x12,  0xb1,  0x12,  0xb3,  0x12,  0xb4,  0x12,  0xb7,  0x12,  0xb9,
             0x12,  0xbc,  0x0,  0xdf,  0xb,  0xd6,
             0xbd,  0x2,  0x79,  0x35,  0x28,  0x2,  0xd3,  0x22,  0x1b,  0x55,
             0xf2,  0xa3,  0xa2,  0x24,  0x2,  0xff,
             0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,
             0xff,  0xfd,  0xff,  0xff,  0xff,  0xff,
             0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0x8,  0x91,
             0x68,  0x31,  0x10,  0x10,  0x21,  0x5,
             0xf0,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t user_password[16] = {'0', '0', '0', '0', '0', '0', 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0};
  uint8_t led_pattern_tmp[16] = {15, '9', 'H', 'e', 'l', 'l', 'o', ',', 'W', 'o', 'r', 'l', 'd', '!', ' ', ' '};
  uint8_t device_name_tmp[16] = {0xff, 'w', 'i', 'r', 'e', 'l', 'e', 's', 's', 'S', 'I', 'M', '0', '0', '0', '0'};
  uint32_t i;

  /* Name of the BLE device. Will be included in the advertising data. */
  memcpy(SMART_USIM_DEVICE_NAME_RAM, device_name_tmp, KEY_LENGTH);
  /* password initialization */
  // random_vector_generate(user_password, 16, NRF_RTC2->COUNTER, RANDOM_BYTE_NUMBER_TIME_OUT_MS, 0);
  memcpy(USER_PASSWORD_RAM, user_password, KEY_LENGTH);
  /* system key initialization */
  // random_vector_generate(user_password, 16, NRF_RTC2->COUNTER, RANDOM_BYTE_NUMBER_TIME_OUT_MS, 0);
  memcpy(SYSTEM_KEY_RAM, user_password, KEY_LENGTH);
  /* LED pattern initialization */
  memcpy(LED_PATTERN_RAM, led_pattern_tmp, KEY_LENGTH);

  /* USIM file data initialization */
  memcpy(USIM0_EF_DATA_RAM, EF_data_USIM, ALL_USIM_EF_FLAG_SIZE);
  memcpy(USIM1_EF_DATA_RAM, EF_data_USIM, ALL_USIM_EF_FLAG_SIZE);

  /* wireless SIM device logical address */
  DEVICE_LOGICAL_ADDRESS_RAM = 1;
  /* wireless SIM air interface on/off
  |  0: wireless SIM off
  |  1: wireless SIM on */
  WIRELESS_SIM_ON_OFF_RAM = 1;
  /* USIM server side call On/Off flag: 1 word = 4 bytes */
  /* 0: USIM server side call off */
  /* 1: USIM server side call on */
  USIM_SERVER_CALL_ON_OFF_RAM = 1;
  /* BLE on/off indicator, 1: BLE on, 0: BLE off */
  BLE_STATUS_ON_OFF_RAM = 1;
  /* BLE bond information erase flag, 1: delete, 0: not delete */
  BLE_BOND_DEL_STATUS_RAM = 1;
  /* wireless USIM mode
  0: fixed wireless USIM client mode
  1: wireless USIM auto mode, the USIM card presnece detection is performed,
     if there ther are USIM cards, working at server mode,
     if there is no USIM card, swith to client mode */
  WIRELESS_USIM_WORK_MODE_RAM = 1;
  /* USIM clock frequency flag,
  |  byte 0, 1: 3.25MHz, 2: 3.84MHz, others: random selection between 3.25MHz and 3.84MHz
  |  byte 1, 0: no updated, 1: updated
  |  byte 2, ETU ticks byte, 0x94/0x95/0x96 */
  *(PHONE_USIM_CLK_FREQ_RAM + 0) = 0;
  *(PHONE_USIM_CLK_FREQ_RAM + 1) = 0;
  *(PHONE_USIM_CLK_FREQ_RAM + 2) = 0;
  /* the default used USIM, 0: USIM0, 1: USIM1 */
  DEFAULT_USED_USIM_RAM = 0;
  /* USIM data read mode when phone power on, 4bytes.
  1: read ICCID data and check it with the data in flash,
  0: skip read ICCID, only check if the flach data is available */
  USIM_DATA_MODE_RAM = 1;
  /* ICCID data mode
  0: read the 2FE2 data from USIM card, from USIM0 and USIM1 respectively
  1: both USIM0 and USIM1 use the same 2FE2 data, read from USIM0
  2: both USIM0 and USIM1 use the same fixed 2FE2 data */
  ICCID_2FE2_DATA_MODE_RAM = 2;
  /* BEL stack softdevice on/off when phone command authentication 0x88
  0: BLE off when 0x88 command
  1: BLE on when 0x88 command */
  AUTHEN_0x88_BLE_ON_OFF_RAM = 0;

  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 0) = 0x98;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 1) = 0x10;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 2) = 0x14;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 3) = 0x30;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 4) = 0x72;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 5) = 0x59;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 6) = 0x92;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 7) = 0x51;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 8) = 0x45;
  *(ICCID_2FE2_FIXED_DATA_MODE_RAM + 9) = 0x53;
  /* write the data bytes to flash */
  for (i=0; i<FLASH_WRITE_DATA_SIZE_IN_WORD; i++)
  {
    ble_flash_word_write((uint32_t *)USER_CONFIG_FLASH_ADDR + i, *((uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER + i));
    ble_flash_word_write((uint32_t *)BACKUP_DATA_USER_CONFIG_FLASH_ADDR + i, *((uint32_t *)USER_CONFIG_FLASH_ADDR + i));
  }

}

/********************************************************************************/
uint32_t usim_data_restore(void)
/*--------------------------------------------------------------------------------
| restore initial USIM EF and user config setting data from backup
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  if (ble_flash_page_erase(USER_CONFIG_FLASH_ADDR / FLASH_PAGE_SIZE) != NRF_SUCCESS)
  {
    return(1);
  }

  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
  for (i=0; i<FLASH_WRITE_DATA_SIZE_IN_WORD; i++)
  {
    if (ble_flash_word_write((uint32_t *)USER_CONFIG_FLASH_ADDR + i, *((uint32_t *)BACKUP_DATA_USER_CONFIG_FLASH_ADDR + i)))
    {
      return(1);
    }
  }

  if ((byte_string_comparison(FLASH_WRITE_DATA_SIZE_IN_WORD << 2, (uint8_t *)USER_CONFIG_FLASH_ADDR, (uint8_t *)BACKUP_DATA_USER_CONFIG_FLASH_ADDR)))
  {
#if (IF_LOG_OUTPUT)
    printf("-------------------------------- USER_CONFIG_FLASH restore error --------------------------------\r\n");
#endif
    return(1);
  }
  else
  {
#if (IF_LOG_OUTPUT)
    printf("-------------------------------- USER_CONFIG_FLASH restore successfully --------------------------------\r\n");
#endif
    return(0);
  }
}

/********************************************************************************/
void watch_dog_timer_init(uint32_t time_out_s)
/*--------------------------------------------------------------------------------
| watch dog timer initialization, the time resolution is 1/4 second = 250ms
|
--------------------------------------------------------------------------------*/
{
  NRF_WDT->CRV = time_out_s * 8192 - 1;
  NRF_WDT->CONFIG = 1;
  NRF_WDT->RREN = 1;
  NRF_WDT->TASKS_START = 1;
}

/********************************************************************************/
void update_user_configuration(void)
/*--------------------------------------------------------------------------------
| read the user config parameters stored in flash
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  /* read the config data from flash */
  for (i=0; i<(FLASH_WRITE_DATA_SIZE_IN_WORD << 2); i++)
  {
    *(P_UINT8_FLASH_DATA_RAM_BUFFER + i) = *((uint8_t *)USER_CONFIG_FLASH_ADDR + i);
  }

  /* copy the LED flash pattern seting */
  memcpy((uint8_t *)LED_PATTERN_BLE_FLASH, LED_PATTERN_RAM, KEY_LENGTH);

  /* the configured default USIN card */
  if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 0)
  {
    PHONE_WIRELESS_USED_USIM = 0;
    set_default_usim0( );
  }
  else
  {
#if (PIN_VCC_SIM1 != PIN_NULL)
    PHONE_WIRELESS_USED_USIM = 1;
    set_default_usim1( );
#else
    PHONE_WIRELESS_USED_USIM = 0;
    set_default_usim0( );
#endif
  }

  update_etu_time( );
}

/********************************************************************************/
void update_etu_time(void)
/*--------------------------------------------------------------------------------
| read the ETU time stored in flash
|
--------------------------------------------------------------------------------*/
{
  if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR) == 1)
  {
    ETU_TICKS_PHONE_ATR_SESSION = 1831;

    if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 2) == 0x94)
    {
      /* 1 / 3.25 * 64 * 16 = 315  */
      ETU_TICKS_PHONE = 315;
    }
    else if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 2) == 0x95)
    {
      /* (1 / 3.25 * 64 * 16 = 315) / 2  */
      ETU_TICKS_PHONE = (315 >> 1);
    }
    else if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 2) == 0x96)
    {
      /* (1 / 3.25 * 64 * 16 = 315) / 4  */
      ETU_TICKS_PHONE = (315 >> 2);
    }
  }
  else if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR) == 2)
  {
    ETU_TICKS_PHONE_ATR_SESSION = 1550;

    if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 2) == 0x94)
    {
      /* 1 / 3.84 * 64 * 16 = 266  */
      ETU_TICKS_PHONE = 266;
    }
    else if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 2) == 0x95)
    {
      /* (1 / 3.84 * 64 * 16 = 266) / 2  */
      ETU_TICKS_PHONE = (266 >> 1);
    }
    else if (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 2) == 0x96)
    {
      /* (1 / 3.84 * 64 * 16 = 266) / 4  */
      ETU_TICKS_PHONE = (266 >> 2);
    }
  }
  else
  {
    uint8_t tmp_random_number[2];

    random_vector_generate(tmp_random_number, 2, NRF_RTC2->COUNTER, RANDOM_BYTE_NUMBER_TIME_OUT_MS, 1);
    if (tmp_random_number[0] + tmp_random_number[1] > 255)
    {
      /* phone SIM interface ETU define, in 16M timer ticks
      |  clock signal is 3.25M for SIM interface
      |  1 / 3.25 * 372 * 16 = 1831, initial ETU is 372 clock cycle */
      ETU_TICKS_PHONE_ATR_SESSION = 1831;
    }
    else
    {
      /* phone SIM interface ETU define, in 16M timer ticks
      |  clock signal is 3.84M for SIM interface
      |  1 / 3.84 * 372 * 16 = 1550, initial ETU is 372 clock cycle */
      ETU_TICKS_PHONE_ATR_SESSION = 1550;
    }
  }
}



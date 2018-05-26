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
#include "flash_data_write.h"
#include "global_data_exchange_extern.h"
#include "nrf_fstorage.h"
#include "flash_file_address.h"
#include "nrf_soc.h"
#include "nrf_nvic.h"
#include "ble_flash.h"
#include "comm_phone_command.h"
#include "comm_initial.h"
#include "app_util_platform.h"
#include "wireless_sim_phone.h"

/********************************************************************************/
uint32_t start_flash_page_update(uint32_t update_length_in_word, uint32_t flash_address_update_page, uint32_t *ram_data)
/*--------------------------------------------------------------------------------
| update flash page data, the data length in word is update_length_in_word,
| the data in RAM to be written to flash is started in ram_data
|
--------------------------------------------------------------------------------*/
{
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
  FLASH_UPDATE_WAITING_STAGE = 0;
  flash_page_update(update_length_in_word, flash_address_update_page, ram_data);

  return(0);
}

/********************************************************************************/
uint32_t flash_page_update(uint32_t update_length_in_word, uint32_t flash_address_update_page, uint32_t *ram_data)
/*--------------------------------------------------------------------------------
| update flash page data, the data length in word is update_length_in_word,
| the data in RAM to be written to flash is started in ram_data
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  /* page erase */
  if (FLASH_UPDATE_WAITING_STAGE == 0)
  {
    if (IF_SOFTDEVICE_RUNNING)
    {
      if (sd_flash_page_erase(flash_address_update_page / FLASH_PAGE_SIZE) == NRF_SUCCESS)
      {
        FLASH_UPDATE_WAITING_STAGE = 1;
        return(0);
      }
      else
      {
        FLASH_UPDATE_WAITING_STAGE = 0;
        return(1);
      }
    }
    else
    {
      if (ble_flash_page_erase(flash_address_update_page / FLASH_PAGE_SIZE) == NRF_SUCCESS)
      {
        FLASH_UPDATE_WAITING_STAGE = 1;
      }
      else
      {
        FLASH_UPDATE_WAITING_STAGE = 0;
        return(1);
      }
    }
  }

  /* page data words write */
  if (FLASH_UPDATE_WAITING_STAGE == 1)
  {
    if (IF_SOFTDEVICE_RUNNING)
    {
      if (sd_flash_write((uint32_t *)flash_address_update_page, (uint32_t const *)ram_data, update_length_in_word) != NRF_SUCCESS)
      {
        FLASH_UPDATE_WAITING_STAGE = 0;
        return(1);
      }
    }
    else
    {
      for (i=0; i< update_length_in_word; i++)
      {
        if (ble_flash_word_write((uint32_t *)flash_address_update_page + i, *(ram_data + i)))
        {
          FLASH_UPDATE_WAITING_STAGE = 0;
          return(1);
        }
      }
    }
  }

  FLASH_UPDATE_WAITING_STAGE = 0;
  return(0);
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
void sys_event_flash_erase_write_handler(uint32_t sys_evt, void * p_context)
/*--------------------------------------------------------------------------------
| handler of flash erase/write
|
--------------------------------------------------------------------------------*/
{
  switch (sys_evt)
  {
    case NRF_EVT_FLASH_OPERATION_SUCCESS:
    {
      if (FLASH_UPDATE_WAITING_STAGE == 1)
      {
#if (IF_LOG_OUTPUT)
        printf("-------------------- flash page erase finished, continue to write, FLASH_UPDATE_WAITING_STAGE = %d --------------------\r\n", FLASH_UPDATE_WAITING_STAGE);
#endif
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        /* save the user config byte to flash */
        flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
      }
    }
  }
}
#endif

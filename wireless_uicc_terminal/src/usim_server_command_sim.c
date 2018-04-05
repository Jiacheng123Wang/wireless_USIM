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
#include "phone_command_sim.h"
#include "usim_server_command_sim.h"
#include "define_config.h"
#include "comm_phone_command.h"
#include "comm_uicc_terminal_interface.h"
#include "usim_server_sim_interface.h"
#include "usim_server_wireless_interface.h"
#include "nrf_nvic.h"
#include "wireless_sim_phone.h"
#include "global_data_exchange_extern.h"
#include "flash_file_address.h"
#include "nrf_gpio.h"
#include "define_all_data.h"

/********************************************************************************/
void main_usim_server_phone_command(void)
/*--------------------------------------------------------------------------------
| main function for watch phone to USIM interface
|
--------------------------------------------------------------------------------*/
{
  uint8_t tmp_byes[2];
	
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
	    	  	
  switch(*(PHONE_COMMAND + 2))
  {
    case 0xa4:
		{
      usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;	
		}		
		
    case 0xc0:
		{
      if (*(PHONE_COMMAND + 5))
      {
        usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      }
      else
      {
        tmp_byes[0] = 0x90;
        tmp_byes[1] = 0x0;
        write_bytes(2, tmp_byes, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      }
      break;
		}

    case 0xb0:
		{
		  if ((IF_2FE2_SELECTED) && (*((uint8_t *)ICCID_2FE2_DATA_MODE_FLASH_ADDR) == 2))
			{
			  uint8_t confirm_bytes[1] = {0xb0};
			  uint8_t status_bytes[2] = {0x90, 0x0};

#if (IF_LOG_OUTPUT)
				printf("------------------------------------------------------------------\r\n");
				printf("------------------------ fixed 2FE2 data ---------------------------\r\n");
				printf("------------------------------------------------------------------\r\n");
#endif
				IF_2FE2_SELECTED = 0;

			  write_bytes(1, confirm_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);
			  write_bytes(10, ICCID_2FE2_FIXED_DATA_MODE_RAM, ETU_TICKS_PHONE, PIN_DATA_PHONE);
			  write_bytes(2, status_bytes, ETU_TICKS_PHONE, PIN_DATA_PHONE);
			}
			else
			{
        usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
			}
      break;
		}

		case 0xb2:
		{
		   usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
		   break;
		}

    case 0x12:
		{
      // usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      phone_command_fetch_0x12(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      break;
		}

    case 0xf2:
		{
      phone_command_status_response_0xf2(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
  	  break;
		}

    case 0x10:
		{
      // usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      phone_command_terminal_profile_0x10(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);
      break;
		}

    case 0x14:
		{
      // usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      phone_command_terminal_response_0x14(PHONE_COMMAND, ETU_TICKS_PHONE, 0, 0);
      break;
		}

		case 0x88:
		{
			usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
		  break;
		}

    case 0xd6:
		{
      usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;
		}

    case 0xdc:
		{
      // usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      phone_command_update_record_0xdc(PHONE_COMMAND, ETU_TICKS_PHONE, 0, 0);
      break;
		}

    case 0xa2:
		{
      usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      // phone_command_search_record_0xa2(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;
		}

    case 0xc2:
		{
      // usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      phone_command_envelope_0xc2(PHONE_COMMAND, ETU_TICKS_PHONE, 0, 0);
      break;
		}

    case 0x20:
		{
      /* initial USAT menu setting */
      USAT_BYTE_LENGTH_BACK = USAT_BYTE_LENGTH_BACK_INIT;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_INITIAL_USAT_MENU_SETUP;
      usim_server_command_verify(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;
		}

    case 0x2c:
		{
      usim_server_command_verify(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;
		}

    case 0x70:
		{
      usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;
		}

    case 0xde:
		{
      usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;
		}

	  case 0x24:
	  case 0x26:
	  case 0x28:
		{
      phone_command_pin_0x24(PHONE_COMMAND, ETU_TICKS_PHONE);
      break;
		}

	  case 0x8e:
		{
      usim_server_command_GSM_algorithm(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
      break;
		}

    default:
		{
		  phone_command_default_response(PHONE_COMMAND, ETU_TICKS_PHONE, PIN_DATA_PHONE);				
      break;
		}
  }

  /* stop SIM clock */
  clock_sim_stop_4m(PIN_CLOCK_SIM);		
		
	set_event_phone_command( );
}	

/********************************************************************************/
uint32_t usim_server_command_confirm_sim(uint8_t *phone_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| read USIM feedback to watch phone command, byte *(phone_command + 2))
|
--------------------------------------------------------------------------------*/
{
  uint32_t initial_timer;
	uint8_t check_bit;

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
	
  while (nrf_gpio_pin_read(PIN_DATA_SIM))
  {
    if ((NRF_RTC2->COUNTER - initial_timer) > (DATA_TX_TIME_MS))
    {
      return(1);
    }
  }
  read_byte(READ_BYTE_UICC_TERMINAL + 1, &check_bit, etu_length, PIN_DATA_SIM);
	
  if (*(READ_BYTE_UICC_TERMINAL + 1) != *(phone_command + 2))
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      if ((NRF_RTC2->COUNTER - initial_timer) > (DATA_TX_TIME_MS))
      {
        return(1);
      }
    }	
    read_byte(READ_BYTE_UICC_TERMINAL + 2, &check_bit, etu_length, PIN_DATA_SIM);
		
#if (IF_LOG_OUTPUT)
    printf_log_tx(2, READ_BYTE_UICC_TERMINAL + 1);
#endif  
		
    *(READ_BYTE_UICC_TERMINAL + 0) = 2;
		
    return(0);		
  }
  else
  {
#if (IF_LOG_OUTPUT)
    printf_log_tx(1, READ_BYTE_UICC_TERMINAL + 1);
#endif  
				
    *(READ_BYTE_UICC_TERMINAL + 0) = 1;
		
    return(0);	
  }
}

/********************************************************************************/
uint32_t usim_server_command_GSM_algorithm(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim)
/*--------------------------------------------------------------------------------
| watch phone command session, supported phone command:
| a4: selection
| 10: terminal profile
| 14: terminal response
| 88: run GSM algorithm
| D6: update binary
| DC: update record
| A2: search record
| C2: envelope
|
--------------------------------------------------------------------------------*/
{
  uint32_t tmp_length;
	
  if ((*(bytes_command + 5)) == 0x00)
  {
    tmp_length = 256;
  }
  else
  {
    tmp_length = (*(bytes_command + 5));
  }
	  
  /* write the phone command to SIM, 5 bytes */
  write_bytes(5, bytes_command + 1, etu_length_sim, PIN_DATA_SIM);	

  /* read SIM confirm bytes of the command, 1 or 2 bytes */	
  if (usim_server_command_confirm_sim(bytes_command, etu_length_sim))
  {
    return(1);
  }

  /* write the SIM confirm bytes to phone */
  write_bytes(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);

  if (*(READ_BYTE_UICC_TERMINAL + 0) == 0x2)
  {
    return(1);
  }
	
  /* read phone command data */	
  if (read_bytes_phone(tmp_length, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length_phone))
  {  
    return(1);
  }	
	if ((*(READ_BYTE_UICC_TERMINAL + tmp_length - 1) == 0x2f) && (*(READ_BYTE_UICC_TERMINAL + tmp_length) == 0xe2) && (*(bytes_command + 2) == 0xa4))
	{
#if (IF_LOG_OUTPUT)
		printf("------------------------------------------------------------------\r\n");
		printf("------------------------ 2FE2 selected ---------------------------\r\n");
		printf("------------------------------------------------------------------\r\n");
#endif
    IF_2FE2_SELECTED = 1;		
	}
		
  /* write the phone command data to SIM */
  write_bytes(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1, etu_length_sim, PIN_DATA_SIM);	
  
  /* read SIM returned status bytes, 2 bytes */	
  if (read_bytes_sim(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length_sim, 1))
  {
    return(1);
  }
	
  /* write the SIM returned status bytes to phone */
  write_bytes(2, READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);
  
  return(0);
}

/********************************************************************************/
uint32_t usim_server_command_get_data(uint8_t *bytes_command, uint32_t etu_length_phone, 
         uint32_t etu_length_sim)
/*--------------------------------------------------------------------------------
| watch phone command session, supported phone command:
| B0: read binary
| C0: get response
| 12: fetch
| B2: read redord
| F2: status
|
--------------------------------------------------------------------------------*/
{
  uint32_t tmp_length;
  uint32_t initial_timer;
	uint8_t check_bit;

  if ((*(bytes_command + 5)) == 0x00)
  {
    tmp_length = 256;	
  }	
  else
  {
    tmp_length = (*(bytes_command + 5));
  }

  /* add 1 byte command confirm at the beginning and 2 bytes return status at the end */ 	
  tmp_length += 3;
		
  /* write the phone command to SIM, 5 bytes */
  write_bytes(5, bytes_command + 1, etu_length_sim, PIN_DATA_SIM);	

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;	
  /* read SIM returned phone command confirm byte, *(bytes_command + 2) */	
  while (nrf_gpio_pin_read(PIN_DATA_SIM))
  {
    if ((NRF_RTC2->COUNTER - initial_timer) > (DATA_TX_TIME_MS))
    {
      return(1);
    }
  }
  read_byte(READ_BYTE_UICC_TERMINAL + 1, &check_bit, etu_length_sim, PIN_DATA_SIM);
	
  if ((*(READ_BYTE_UICC_TERMINAL + 1)) == (*(bytes_command + 2)))
  {	
    /* read SIM returned bytes, tmp_length - 1 bytes */	
    if (read_bytes_sim(tmp_length - 1, READ_BYTE_UICC_TERMINAL + 1, PIN_DATA_SIM, etu_length_sim, 0))
    {
      return(1);
    }
    *(READ_BYTE_UICC_TERMINAL + 0) = tmp_length;
    *(READ_BYTE_UICC_TERMINAL + 1) = *(bytes_command + 2);
  }
  else
  {
    /* the phone command is not correct, SIM return 2 status bytes */ 
    /* read the second status byte return from SIM */	
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      if ((NRF_RTC2->COUNTER - initial_timer) > (DATA_TX_TIME_MS))
      {
        return(1);
      }
    }
    read_byte(READ_BYTE_UICC_TERMINAL + 2, &check_bit, etu_length_sim, PIN_DATA_SIM);
	  
    tmp_length = 2;
    *(READ_BYTE_UICC_TERMINAL + 0) = tmp_length;
  }
	
  /* if there have some bytes from USIM to Phone */
  if ((USAT_BYTE_LENGTH_BACK) && (*(READ_BYTE_UICC_TERMINAL + tmp_length - 1) == 0x90) && (*(READ_BYTE_UICC_TERMINAL + tmp_length) == 0x0))
  {
    *(READ_BYTE_UICC_TERMINAL + tmp_length - 1) = 0x91;
    *(READ_BYTE_UICC_TERMINAL + tmp_length) = USAT_BYTE_LENGTH_BACK;
  }
  
  /* write the SIM returned bytes to phone */
  write_bytes(tmp_length, READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);
	
  return(0);
}

/********************************************************************************/
uint32_t usim_server_command_verify(uint8_t *bytes_command, uint32_t etu_length_phone, 
         uint32_t etu_length_sim)
/*--------------------------------------------------------------------------------
| watch phone command session, supported phone command:
| 20: verify
| 2C: unblock pin
|
--------------------------------------------------------------------------------*/
{
  /* write the phone command to SIM, 5 bytes */
  write_bytes(5, bytes_command + 1, etu_length_sim, PIN_DATA_SIM);	
	
  /* read SIM returned status bytes, 2 bytes */	
  if (read_bytes_sim(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length_sim, 1))
  {
    return(1);
  }
	
  /* write the SIM returned status bytes to phone */
  write_bytes(2, READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);
	
  return(0);
}

/********************************************************************************/
uint32_t usim_server_command_status(uint8_t *bytes_command, uint32_t etu_length_phone)
/*--------------------------------------------------------------------------------
| watch phone command status session, command 0xf2
|
--------------------------------------------------------------------------------*/
{
  uint8_t fcp_ADF[0x13 + 1] = {0xff,  0xf2,  0x84,  0x10,  0xa0,  0x0,  0x0,  0x0, 0x87, 
          0x10,  0x2,  0xff,  0x86,  0xff,  0xff,  0x89,  0xff,  0xff,  0xff,  0xff};
  uint8_t tmp_bytes[2];
  
  if (*(bytes_command + 4) == 0xc)
  {
    tmp_bytes[0] = 0x90;
    tmp_bytes[1] = 0;

    /* write SIM response to phone */		
    write_bytes(2, tmp_bytes, etu_length_phone, PIN_DATA_PHONE); 	
  }
  else if ((*(bytes_command + 4) == 0x0) && (*(bytes_command + 5) == 0x0))
  {
    if (USAT_BYTE_LENGTH_BACK)
    {
      tmp_bytes[0] = 0x91;
      tmp_bytes[1] = USAT_BYTE_LENGTH_BACK;
    }
    else
    {
      tmp_bytes[0] = 0x93;
      tmp_bytes[1] = 0;
    }

    /* write SIM response to phone */		
    write_bytes(2, tmp_bytes, etu_length_phone, PIN_DATA_PHONE); 	
  }
  else if (*(bytes_command + 5) == 0x0)
  {
    usim_server_command_verify(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
  }
  else if (*(bytes_command + 5) == 0x12)
  {
    write_bytes(0x13, fcp_ADF + 1, etu_length_phone, PIN_DATA_PHONE);

    if (USAT_BYTE_LENGTH_BACK)
    {
      tmp_bytes[0] = 0x91;
      tmp_bytes[1] = USAT_BYTE_LENGTH_BACK;
    }
    else
    {
      tmp_bytes[0] = 0x90;
      tmp_bytes[1] = 0;
    }
	
    /* write SIM response to phone */		
    write_bytes(2, tmp_bytes, etu_length_phone, PIN_DATA_PHONE); 	
  }
  else
  {
    usim_server_command_get_data(PHONE_COMMAND, ETU_TICKS_PHONE, ETU_TICKS_SIM);
  }
 		
  return(0);
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
uint32_t usim_server_command_confirm_sim_ble(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| read USIM feedback to watch phone command, byte *(phone_command + 2))
|
--------------------------------------------------------------------------------*/
{
  volatile uint32_t time_now; 
  uint8_t tmp_bit; 

  while (nrf_gpio_pin_read(PIN_DATA_SIM))
  {
		/* get the current timer0 */
	  NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	  time_now = NRF_TIMER0->CC[0]; 
    /* wait time out */  
    if ((time_now - start_time_us) > time_length_us)
    {
      return(1);
    }
  }
  read_byte(READ_BYTE_UICC_TERMINAL + 1, &tmp_bit, etu_length, PIN_DATA_SIM);
	
  if (*(READ_BYTE_UICC_TERMINAL + 1) != *(phone_command + 2))
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
			/* get the current timer0 */
		  NRF_TIMER0->TASKS_CAPTURE[0] = 1;
		  time_now = NRF_TIMER0->CC[0]; 
	    /* wait time out */  
	    if ((time_now - start_time_us) > time_length_us)
      {
        return(1);
      }
    }	
    read_byte(READ_BYTE_UICC_TERMINAL + 2, &tmp_bit, etu_length, PIN_DATA_SIM);
				
    *(READ_BYTE_UICC_TERMINAL + 0) = 2;
		
    return(0);		
  }
  else
  {				
    *(READ_BYTE_UICC_TERMINAL + 0) = 1;
		
    return(0);	
  }
}

/********************************************************************************/
uint32_t usim_server_command_GSM_algorithm_ble(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| watch phone command session, supported phone command:
| 88: run GSM algorithm
|
--------------------------------------------------------------------------------*/
{
	/* stage 0 */
  if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage0)
	{
    /* start sim clock signal */
    clock_sim_start_4m(PIN_CLOCK_SIM);
	  
    /* write the phone command to SIM, 5 bytes */
    write_bytes(5, bytes_command + 1, etu_length_sim, PIN_DATA_SIM);	

    /* read SIM confirm bytes of the command, 1 or 2 bytes */	
    if (usim_server_command_confirm_sim_ble(bytes_command, etu_length_sim, start_time_us, time_length_us))
    {
      /* set SWI3_EGU3 to stop BLE softdevice */
      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
		  FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;
		
	    /* stop SIM clock */
	    clock_sim_stop_4m(PIN_CLOCK_SIM);		

      return(1);
    }
	
    WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage1;
    return (0);
  }
	
	/* stage 1 */
  if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage1)
  {
    /* write the SIM confirm bytes to phone */
    write_bytes(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);

    if (*(READ_BYTE_UICC_TERMINAL + 0) == 0x2)
    {
	    /* stop SIM clock */
	    clock_sim_stop_4m(PIN_CLOCK_SIM);		

      return(1);
    }
	
    /* read phone command data */	
    if (read_bytes_ble((*(bytes_command + 5)), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length_phone, 
	  start_time_us, time_length_us))
    {  
      /* set SWI3_EGU3 to stop BLE softdevice */
      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
		  FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;

	    /* stop SIM clock */
	    clock_sim_stop_4m(PIN_CLOCK_SIM);		

      return(1);
    }	
	
    WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage2;
    return (0);	
  }	

	/* stage 2 */
  if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage2)
  {
    /* write the phone command data to SIM */
    write_bytes(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1, etu_length_sim, PIN_DATA_SIM);	
   
	  SIM_READ_BYTES_NUMBER = 2;
	  USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE = 0xFF;
    sd_nvic_SetPendingIRQ(GPIOTE_IRQn);
		
    return (0);	
  }	 	

	/* stage 3 */
  if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage3)
  {
    /* write the SIM returned status bytes to phone */
    write_bytes(2, READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);
	
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );
		
    WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage4;
  }
	
  return (0);	
}

/********************************************************************************/
uint32_t usim_server_command_get_data_ble(uint8_t *bytes_command, uint32_t etu_length_phone, 
         uint32_t etu_length_sim, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| watch phone command session, supported phone command:
| B0: read binary
| C0: get response
| 12: fetch
| B2: read redord
| F2: status
|
--------------------------------------------------------------------------------*/
{
	uint8_t get_data_length = (*(bytes_command + 5));
  volatile uint32_t time_now; 
  uint32_t i;
	uint8_t check_bit;
	
	/* stage 0 */
  if (USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE == USIM_SERVER_COMMAND_GET_DATA_BLE_stage0)
	{
    /* start sim clock signal */
    clock_sim_start_4m(PIN_CLOCK_SIM);

    /* write the phone command to SIM, 5 bytes */
    write_bytes(5, bytes_command + 1, etu_length_sim, PIN_DATA_SIM);	
	
    /* read SIM returned phone command confirm byte, *(bytes_command + 2) */	
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0]; 		
		  /* wait time out */		
      if ((time_now - start_time_us) > time_length_us)
      {
	      /* set SWI3_EGU3 to stop BLE softdevice */
	      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
			  FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;

		    /* stop SIM clock */
		    clock_sim_stop_4m(PIN_CLOCK_SIM);		

        return(1);
      }
    }
    read_byte(READ_BYTE_UICC_TERMINAL + 1, &check_bit, etu_length_sim, PIN_DATA_SIM);
	
	  /* return byte read correct */
    if ((*(READ_BYTE_UICC_TERMINAL + 1)) == (*(bytes_command + 2)))
    {
		  for (i=0; i<get_data_length; i++)
		  {
		    while (nrf_gpio_pin_read(PIN_DATA_SIM))
		    {
				  /* get the current timer0 */
			    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
			    time_now = NRF_TIMER0->CC[0]; 
				  /* wait time out */		
		      if ((time_now - start_time_us) > time_length_us)
		      {
			      /* set SWI3_EGU3 to stop BLE softdevice */
			      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
					  FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;

				    /* stop SIM clock */
				    clock_sim_stop_4m(PIN_CLOCK_SIM);		

		        return(1);
		      }
		    }
		    read_byte(READ_BYTE_UICC_TERMINAL + i + 2, &check_bit, etu_length_sim, PIN_DATA_SIM);
		  }
      *(READ_BYTE_UICC_TERMINAL + 0) = get_data_length + 3;
      *(READ_BYTE_UICC_TERMINAL + 1) = *(bytes_command + 2);
      *(READ_BYTE_UICC_TERMINAL + get_data_length + 2) = 0x90;		
      *(READ_BYTE_UICC_TERMINAL + get_data_length + 3) = 0x00;		
    }
    else /* return byte is different, read status byte */
    {
      /* the phone command is not correct, SIM return 2 status bytes */ 
      /* read the second status byte return from SIM */	
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
			  /* get the current timer0 */
		    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
		    time_now = NRF_TIMER0->CC[0]; 		
			  /* wait time out */		
	      if ((time_now - start_time_us) > time_length_us)
        {
		      /* set SWI3_EGU3 to stop BLE softdevice */
		      sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
				  FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_STOP_BLE_STACK;

			    /* stop SIM clock */
			    clock_sim_stop_4m(PIN_CLOCK_SIM);		

          return(1);
        }
      }
      read_byte(READ_BYTE_UICC_TERMINAL + 2, &check_bit, etu_length_sim, PIN_DATA_SIM);
	  
      *(READ_BYTE_UICC_TERMINAL + 0) = get_data_length;
    }
	
	  USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE = USIM_SERVER_COMMAND_GET_DATA_BLE_stage1;
		
	  return(0);
	}	
	/* stage 1 */
  if (USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE == USIM_SERVER_COMMAND_GET_DATA_BLE_stage1)
	{
    /* write the SIM returned bytes to phone */
    write_bytes(*(READ_BYTE_UICC_TERMINAL + 0), READ_BYTE_UICC_TERMINAL + 1, etu_length_phone, PIN_DATA_PHONE);
	
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );
		
	  USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE = USIM_SERVER_COMMAND_GET_DATA_BLE_stage2;
	}
	
  return(0);
}
#endif

/********************************************************************************/
uint32_t usim_command_authentication_0x88(void)
/*--------------------------------------------------------------------------------
| USIM command authentication 0x88
|
--------------------------------------------------------------------------------*/
{
	uint8_t authentication_command_byte[5] = {0x0,  0x88,  0x0,  0x81,  0x22};
  uint8_t get_response_command_byte[5] = {0x0, 0xc0, 0x0, 0x0, 0x35};
  uint8_t authentication_input_byte[0x22] = {0x10,  0x6e,  0xe,  0x50,  0xc,  0xcf,  0xe2,  
		0xb7,  0x8e,  0x40,  0xdd,  0x60,  0xe4,  0x19,  0xd0,  0xe9,  0xd4,  
		0x10,  0x68,  0x5b,  0xca,  0xab,  0x7a,  0x0,  0x72,  0x4c,  0x38,  0xc3,  0xa7,  0x9c,  
		0x26,  0xd4,  0x7d,  0x2f};
	uint8_t response_length_byte[3];
	
  /* start SIM card interface 4M clock signal */  
  clock_sim_start_4m(PIN_CLOCK_SIM);
	/* start timer1 for UICC-terminal ETU */
  NRF_TIMER1->TASKS_START = 1;
		
  write_bytes(5, authentication_command_byte, ETU_TICKS_SIM, PIN_DATA_SIM);    
	
  if(read_bytes_sim(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
  {
    /* stop SIM interface 4M clock signal */
    clock_sim_stop_4m(PIN_CLOCK_SIM);
    return(1);
  }

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;
  
  write_bytes(0x22, authentication_input_byte, ETU_TICKS_SIM, PIN_DATA_SIM);
  
  if(read_bytes_sim(2, response_length_byte, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
  {
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );
    return(1);
  }
  if (!(*(response_length_byte + 1) == 0x61))
  {
#if (IF_LOG_OUTPUT)    
    printf("Authentication error......\r\n");
#endif  
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );

    return(1);
  }
  
	get_response_command_byte[4]  = *(response_length_byte + 2);  
  write_bytes(5, get_response_command_byte, ETU_TICKS_SIM, PIN_DATA_SIM);
  
  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;
  
  if(read_bytes_sim(get_response_command_byte[4] + 3, READ_BYTE_UICC_TERMINAL + 1, PIN_DATA_SIM, ETU_TICKS_SIM, 0))
  {
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );

    return(1);
  }
	
	/* stop SIM clock signal and timer 1*/
  stop_sim_clock_timer1( );

	return(0);	
}


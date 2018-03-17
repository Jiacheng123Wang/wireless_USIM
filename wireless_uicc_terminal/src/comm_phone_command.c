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
#include <nrf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nrf_gpiote.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "sd_ble_ancs_nus.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdm.h"
#include "define_config.h"
#include "comm_phone_command.h"
#include "comm_uicc_terminal_interface.h"
#include "comm_initial.h"
#include "wireless_sim_phone.h"
#include "flash_file_address.h"
#include "nrf_nvic.h"
#include "nrf_sdm.h"
#include "time_slot.h"
#include "comm_initial.h"
#include "comm_wireless_interface.h"
#include "phone_wireless_interface.h"
#include "usim_server_wireless_interface.h"
#include "usim_server_sim_interface.h"
#include "usim_server_command_sim.h"
#include "phone_command_sim.h"
#include "nrf_library_update.h"
#include "global_data_exchange_extern.h"
#include "define_all_data.h"

uint8_t ABOUT_DISPLAY_TEXT[] = "Wireless USIM interface designed by Jiacheng Wang, +86-13641015183, jiacheng.wang AT icloud.com.";
uint8_t USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT[] = "Send Request to Switch Wireless USIM1";
uint8_t USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT[] = "Send Request to Switch Wireless USIM2";
uint8_t USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT[] = "Can not Switch to Wireless USIM1, Remote USIM1 is not Avaliable";
uint8_t USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT[] = "Can not Switch to Wireless USIM2, Remote USIM2 is not Avaliable";
uint8_t USIM_SWITCH_USIM0_NO_DISPLAY_TEXT[] = "Can not Switch to USIM1, There is no USIM1";
uint8_t USIM_SWITCH_USIM1_NO_DISPLAY_TEXT[] = "Can not Switch to USIM2, There is no USIM2";
uint8_t USIM_SWITCH_USIM0_YES_DISPLAY_TEXT[] = "Switch to USIM1";
uint8_t USIM_SWITCH_USIM1_YES_DISPLAY_TEXT[] = "Switch to USIM2";
uint8_t BLE_STATUS_ON_DISPLAY_TEXT[] = "Set BLE On";
uint8_t BLE_STATUS_OFF_DISPLAY_TEXT[] = "Set BLE Off";
uint8_t WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT[] = "Set wireless USIM On";
uint8_t WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT[] = "Set wireless USIM Off";
uint8_t BLE_BOND_DELETE_DISPLAY_TEXT[] = "Delete Bonded BLE device";
#if (PIN_VCC_SIM1 != PIN_NULL)					
uint8_t SIM_SERVER_ON_USIM0_DISPLAY_TEXT[] = "USIM1, Wireless USIM Server On";  
uint8_t SIM_SERVER_OFF_USIM0_DISPLAY_TEXT[] = "USIM1, Wireless USIM Server Off";  
#else
uint8_t SIM_SERVER_ON_USIM0_DISPLAY_TEXT[] = "Wireless USIM Server On";  
uint8_t SIM_SERVER_OFF_USIM0_DISPLAY_TEXT[] = "Wireless USIM Server Off";  
#endif
uint8_t SIM_SERVER_ON_USIM1_DISPLAY_TEXT[] = "USIM2, Wireless USIM Server On";  
uint8_t SIM_SERVER_OFF_USIM1_DISPLAY_TEXT[] = "USIM2, Wireless USIM Server Off";  
#if (PIN_VCC_SIM1 != PIN_NULL)					
uint8_t SIM_CLIENT_ON_USIM0_CONNECTED_STRING[] = "USIM1, Wireless USIM Client On, Connected";  
uint8_t SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT[] = "USIM1, Wireless USIM Client On, Disconnected";  
#else
uint8_t SIM_CLIENT_ON_USIM0_CONNECTED_STRING[] = "Wireless USIM Client On, Connected";  
uint8_t SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT[] = "Wireless USIM Client On, Disconnected";  
#endif
uint8_t SIM_CLIENT_ON_USIM1_CONNECTED_STRING[] = "USIM2, Wireless USIM Client On, Connected";  
uint8_t SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT[] = "USIM2, Wireless USIM Client On, Disconnected";  
uint8_t SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT[] = "Wireless USIM Client Off, Address: ";  
uint8_t CONFIG_SYCH_USIM_SERVER_DISPLAY_TEXT[] = "Start to Synchronize Config";
uint8_t CONFIG_SYCH_USIM_CLIENT_DISPLAY_TEXT[] = "USIM Client Mode, Can not Start Config Sych";
uint8_t PASSWD_CHANGED_DISPLAY_TEXT[] = "Wireless USIM Password Changed";
uint8_t PASSWD_ERROR_DISPLAY_TEXT[] = "Password String Length is not Enough, Please Try Again";
uint8_t SYATEM_KEY_UPDATED_DISPLAY_TEXT[] = "Wireless USIM System Key Updated";
uint8_t SYATEM_KEY_ERROR_DISPLAY_TEXT[] = "System Key String Length is not Enough, Please Try Again";
uint8_t NAME_CHANGED_DISPLAY_TEXT[] = "Smart SIM Tray Device Name Changed";
uint8_t NAME_ERROR_DISPLAY_TEXT[] = "Device Name String Length is not Enough, Please Try Again";
uint8_t CONFIG_SYCH_IDLE_DISPLAY_TEXT[] = "Config Sychronization in Progress";
uint8_t USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT[] = "Can not Switch Wireless USIM, Wireless USIM is Off or There is no Wireless USIM Link";
uint8_t CONFIG_RESTORE_DISPLAY_TEXT[] = "Restore Config to Default Setting";
uint8_t LED_PATTERN_UPDATED_DISPLAY_TEXT[] = "LED Pattern Updated";
uint8_t LED_PATTERN_ERROR_DISPLAY_TEXT[] = "LED Pattern String Length is not Enough, Please Try Again.";
uint8_t WRONG_CONFIG_SETTING_KEY_WORD_DISPLAY_TEXT[] = "Config setting command input wrong, input \"000\" for command list";
uint8_t CONFIG_SETTINGS_KEY_WORD_LIST_DISPLAY_TEXT[] = "Command: RST, LED, PWD, KEY, NAM, CRS, CSY, DBN, USM, RCR, ADD, ICD, UWM, DUM, AUT";
uint8_t USIM0_EF_UPDATE_DISPLAY_TEXT[] = "Update USIM1 File Data";
uint8_t USIM1_EF_UPDATE_DISPLAY_TEXT[] = "Update USIM2 File Data";
uint8_t USIM_EF_UPDATE_ERROR_DISPLAY_TEXT[] = "Update USIM File Data Error, Wireless USIM Client Mode or Wrong USIM No. (Correct: 1 or 2)";
uint8_t REMOTE_USIM_RESTORE_DISPLAY_TEXT_CLIENT[] = "USIM Client Mode, Please Set Remote USIM Restore at USIM Server Side";
uint8_t REMOTE_USIM_RESTORE_DISPLAY_TEXT[] = "Set Remote USIM Restore";
uint8_t CONFIG_RESTORE_SUCCESSFULLY_DISPLAY_TEXT[] = "User Config and EF Data Restore Successfully";
uint8_t USIM0_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT[] = "USIM1 EF Data Updated Successfully";
uint8_t USIM1_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT[] = "USIM2 EF Data Updated Successfully";
uint8_t BLE_BOND_DELETE_SUCCESSFULLY_DISPLAY_TEXT[] = "BLE Device Bond Deleted Successfully";
uint8_t LOGICAL_ADDRESS_ERROR_DISPLAY_TEXT[] = "Wireless USIM Logical Address Set Error, No Logical Address or Wrong Logical Address Input (Correct: 0-7)";
uint8_t LOGICAL_ADDRESS_SUCCESSFULLY_DISPLAY_TEXT[] = "Wireless USIM Logical Address Set Successfully as:  ";
uint8_t ICCID_2FE2_DATA_MODE_ERROR_DISPLAY_TEXT[] = "ICCID 2FE2 Data Mode Set Error, No Input or Wrong Number Input (Correct: 0-2)";
uint8_t ICCID_2FE2_DATA_MODE_SUCCESSFULLY_DISPLAY_TEXT[] = "ICCID 2FE2 Data Mode Set Successfully as:  ";
uint8_t WIRELESS_USIM_WORK_MODE_ERROR_DISPLAY_TEXT[] = "Wireless USIM Work Mode Set Error, No Input or Wrong Number Input (Correct: 0 or 1)";
uint8_t WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_DISPLAY_TEXT[] = "Wireless USIM Work Mode Set Successfully as:  ";
uint8_t DEFAULT_USED_USIM_ERROR_DISPLAY_TEXT[] = "Default Used USIM Set Error, No Input or Wrong Number Input (Correct: 1 or 2)";
uint8_t BLE_OFF_0X88_COMMAND_DISPLAY_TEXT[] = "Set BLE Off for Phone Command Authentication 0x88";
uint8_t BLE_ON_0X88_COMMAND_DISPLAY_TEXT[] = "Set BLE On for Phone Command Authentication 0x88";
uint8_t BLE_ON_OFF_DISPLAY_TEXT[] = "BLE On/Off Set Successfully";
uint8_t WIRELESS_SIM_ON_OFF_DISPLAY_TEXT[] = "Wireless USIM On/Off Set Successfully";
uint8_t BLE_ON_OFF_0X88_DISPLAY_TEXT[] = "BLE On/Off for 0x88 Command Set Successfully";
	
/********************************************************************************/
void phone_sim_pin_setup(void)
/*--------------------------------------------------------------------------------
| Phone-SIM interface GPIO pin setting
|
--------------------------------------------------------------------------------*/
{
	/* UICC-terminal RST pin */
  nrf_gpio_cfg_input(PIN_RESET_PHONE, NRF_GPIO_PIN_PULLUP);
	/* UICC-terminal CLK pin */
  nrf_gpio_cfg_input(PIN_CLOCK_PHONE, NRF_GPIO_PIN_PULLUP);
	/* UICC-terminal IO pin */
  nrf_gpio_cfg_output_H0D1(PIN_DATA_PHONE);

  /* put IO pin high state */
  nrf_gpio_pin_write(PIN_DATA_PHONE, 1);  
	
	/* LED pin initialization */
#ifdef PIN_LED_BLUE		
  nrf_gpio_cfg_output(PIN_LED_BLUE);
  nrf_gpio_pin_write(PIN_LED_BLUE, 1);
#endif
#ifdef PIN_LED_GREEN		
  nrf_gpio_cfg_output(PIN_LED_GREEN);
  nrf_gpio_pin_write(PIN_LED_GREEN, 1);
#endif
#ifdef PIN_LED_YELLOW		
  nrf_gpio_cfg_output(PIN_LED_YELLOW);
  nrf_gpio_pin_write(PIN_LED_YELLOW, 1);
#endif
#ifdef PIN_LED_ORANGE		
  nrf_gpio_cfg_output(PIN_LED_ORANGE);
  nrf_gpio_pin_write(PIN_LED_ORANGE, 1);
#endif
#ifdef PIN_LED_WHITE		
  nrf_gpio_cfg_output(PIN_LED_WHITE);
  nrf_gpio_pin_write(PIN_LED_WHITE, 1);
#endif
#ifdef PIN_LED_RED		
  nrf_gpio_cfg_output(PIN_LED_RED);
  nrf_gpio_pin_write(PIN_LED_RED, 1);
#endif
}

/********************************************************************************/
void saadc_init_config(void)
/*--------------------------------------------------------------------------------
| SAADC configuration initialization to get DC sample value from phone VDD pin
|
--------------------------------------------------------------------------------*/
{
  uint32_t start_time_ms;
	
  /* Configure SAADC singled-ended channel, Internal reference (0.6V) and 1/6 gain */
  NRF_SAADC->CH[0].CONFIG = (SAADC_CH_CONFIG_GAIN_Gain1_6    << SAADC_CH_CONFIG_GAIN_Pos)   |
                            (SAADC_CH_CONFIG_MODE_SE         << SAADC_CH_CONFIG_MODE_Pos)   |
                            (SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) |
                            (SAADC_CH_CONFIG_RESN_Bypass     << SAADC_CH_CONFIG_RESN_Pos)   |
                            (SAADC_CH_CONFIG_RESP_Bypass     << SAADC_CH_CONFIG_RESP_Pos)   |
                            (SAADC_CH_CONFIG_TACQ_20us       << SAADC_CH_CONFIG_TACQ_Pos);

  /* Configure the SAADC channel with VDD as positive input, no negative input(single ended) */
  NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_VDD << SAADC_CH_PSELP_PSELP_Pos;
  NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos;

  /* Configure the SAADC resolution */
  NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_8bit << SAADC_RESOLUTION_VAL_Pos;

  /* No automatic sampling, will trigger with TASKS_SAMPLE */
  NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;

  /* Enable SAADC (would capture analog pins if they were used in CH[0].PSELP) */
  NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

  /* Calibrate the SAADC (only needs to be done once in a while) */
  NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
	start_time_ms = NRF_RTC2->COUNTER;
  while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0)
	{
		if (NRF_RTC2->COUNTER - start_time_ms > 5)
		{
      /* system reset */
      sd_nvic_SystemReset( );
		}
	}
  NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
	start_time_ms = NRF_RTC2->COUNTER;
  while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy <<SAADC_STATUS_STATUS_Pos))
	{
		if (NRF_RTC2->COUNTER - start_time_ms > 5)
		{
      /* system reset */
      sd_nvic_SystemReset( );
		}
	}
}

/********************************************************************************/
int32_t saadc_sample_value_get(void)
/*--------------------------------------------------------------------------------
| get one ADC sample value from phone VDD pin
|
--------------------------------------------------------------------------------*/
{
  volatile int32_t result = 0;
  uint32_t start_time_ms;

  /* Configure result to be put in RAM at the location of "result" variable */
  NRF_SAADC->RESULT.MAXCNT = 1;
  NRF_SAADC->RESULT.PTR = (uint32_t)&result;
	
  /* Start the SAADC and wait for the started event */
  NRF_SAADC->TASKS_START = 1;
	start_time_ms = NRF_RTC2->COUNTER;
  while (NRF_SAADC->EVENTS_STARTED == 0)
	{
		if (NRF_RTC2->COUNTER - start_time_ms > 5)
		{
      /* system reset */
      sd_nvic_SystemReset( );
		}
	}
  NRF_SAADC->EVENTS_STARTED = 0;

  /* Do a SAADC sample, will put the result in the configured RAM buffer */
  NRF_SAADC->TASKS_SAMPLE = 1;
	start_time_ms = NRF_RTC2->COUNTER;
  while (NRF_SAADC->EVENTS_END == 0)
	{
		if (NRF_RTC2->COUNTER - start_time_ms > 5)
		{
      /* system reset */
      sd_nvic_SystemReset( );
		}
	}
  NRF_SAADC->EVENTS_END = 0;

  /* Stop the SAADC, since it's not used anymore */
  NRF_SAADC->TASKS_STOP = 1;
	start_time_ms = NRF_RTC2->COUNTER;
  while (NRF_SAADC->EVENTS_STOPPED == 0)
	{
		if (NRF_RTC2->COUNTER - start_time_ms > 5)
		{
      /* system reset */
      sd_nvic_SystemReset( );
		}
	}
  NRF_SAADC->EVENTS_STOPPED = 0;

	return (result);
}

/********************************************************************************/
void gpio_event_in_config(void)
/*--------------------------------------------------------------------------------
| GPIO pin input event interrupt configuration
|
--------------------------------------------------------------------------------*/
{
  sd_nvic_SetPriority(GPIOTE_IRQn, GPIOTE_IRQ_PRIORITY);
  sd_nvic_ClearPendingIRQ(GPIOTE_IRQn); 
  sd_nvic_SetPriority(SWI3_EGU3_IRQn, SWI3_EGU3_IRQ_PRIORITY);  
  sd_nvic_ClearPendingIRQ(SWI3_EGU3_IRQn); 
  
	/* GPIOTE channel 1, phone CLOCK pin GPIOTE event */
	NRF_GPIOTE->CONFIG[1] =  (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)
	                           | (PIN_CLOCK_PHONE << GPIOTE_CONFIG_PSEL_Pos)
	                           | (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos);
	NRF_GPIOTE->INTENSET |= GPIOTE_INTENSET_IN1_Set << GPIOTE_INTENSET_IN1_Pos;
  
	/* Enable interrupt */
  sd_nvic_EnableIRQ(GPIOTE_IRQn);
  sd_nvic_EnableIRQ(SWI3_EGU3_IRQn);
}

/********************************************************************************/
void clear_event_phone_command(void)
/*--------------------------------------------------------------------------------
| clear interrupt events for phone command process
|
--------------------------------------------------------------------------------*/
{
  /* clear GPIOTE channel 1, PIN_CLOCK_PHONE */
  gpio_event_in_clear_clock( );
	/* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
		
  /* start timer1, for ETU timing */
  NRF_TIMER1->TASKS_START = 1;
	/* turn on LED to indicate phone command session is ongoing */
#ifdef PIN_LED_BLUE
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
    nrf_gpio_pin_write(PIN_LED_BLUE, 1); 
	}
#endif 
#ifdef PIN_LED_GREEN			
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
    nrf_gpio_pin_write(PIN_LED_GREEN, 1);
	}
#endif 
#ifdef PIN_LED_YELLOW
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
    nrf_gpio_pin_write(PIN_LED_YELLOW, 1); 
	}
#endif 
#ifdef PIN_LED_ORANGE			
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
    nrf_gpio_pin_write(PIN_LED_ORANGE, 1);
	}
#endif 
#ifdef PIN_LED_WHITE
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
    nrf_gpio_pin_write(PIN_LED_WHITE, 1); 
	}
#endif 
#ifdef PIN_LED_RED			
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
    nrf_gpio_pin_write(PIN_LED_RED, 1);
	}
#endif 
}
 
/********************************************************************************/
void set_event_phone_command(void)
/*--------------------------------------------------------------------------------
| re-enable interrupt events after phone command session
|
--------------------------------------------------------------------------------*/
{
  /* enable GPIOTE event from CLK pin */
  gpio_event_in_set_clock( );
	/* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);	
  /* stop ETU timer tick */
  NRF_TIMER1->TASKS_CLEAR = 1;
  NRF_TIMER1->TASKS_SHUTDOWN = 1;
#if (IF_SOFTDEIVE_USED)
	if (IF_SOFTDEVICE_RUNNING)
	{
    FLAG_PHONE_COMMAND_READ = 0;
	}
#endif
}

/********************************************************************************/
void GPIOTE_IRQHandler(void)
/*--------------------------------------------------------------------------------
| phone GPIO input event interrupt handler
|
--------------------------------------------------------------------------------*/
{
	static volatile uint8_t CLOCK_PIN_COUNTER = 0;

#if (IF_SOFTDEIVE_USED)
	/* GPIOTE event for USIM authentication command results data read */
  if (SIM_READ_BYTES_NUMBER)
	{
		/* free the time slot request semaphore */
		SEMAPHORE_TIME_SLOT_REQUEST = 0;
		
	  /* read SIM returned status bytes, 2 bytes */
	  if (read_bytes_sim(SIM_READ_BYTES_NUMBER, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
	  {
	    /* clear SIM data reab byes number */
			SIM_READ_BYTES_NUMBER = 0;
	    /* renew RTC2 CC event interrupt and watch-dog */
			rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
			
	    return;
	  }
#if (IF_LOG_OUTPUT)
		printf_log_tx(2, READ_BYTE_UICC_TERMINAL + 1);
#endif
		
    /* read byte check */		
		if (*(READ_BYTE_UICC_TERMINAL + 1) != 0x61)
		{
	    /* clear SIM data reab byes number */
			SIM_READ_BYTES_NUMBER = 0;
		  if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage2)
			{
				set_event_phone_command( );
			}
			else if (USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE == USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage0)
			{
		    rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);	
				/* stop timer1 */
				NRF_TIMER1->TASKS_SHUTDOWN = 1;
	    }
			
			// /* authentication application error, reset USIM */
			// if ((*(READ_BYTE_UICC_TERMINAL + 1) == 0x98) && (*(READ_BYTE_UICC_TERMINAL + 2) == 0x62))
			// {
			// 	sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
			// 	FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			// }

		  return;				
		}
		if ((*(READ_BYTE_UICC_TERMINAL + 2) != 0x35) && (*(READ_BYTE_UICC_TERMINAL + 2) != 0x10))
		{
      /* clear SIM data reab byes number */
		  SIM_READ_BYTES_NUMBER = 0;
			if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage2)
			{
				set_event_phone_command( );
			}
			else if (USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE == USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage0)
			{
	      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);	
			  /* stop timer1 */
			  NRF_TIMER1->TASKS_SHUTDOWN = 1;
      }
				
			return;			
		}
		
    /* set SWI3_EGU3 for phone command process */
    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
		if (WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE == USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage2)
		{
			FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_USIM_SERVER_SIM_COMMAND_AUTHENTICATION_stage3;
		}
		else if (USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE == USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage0)
		{
			FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_USIM_SERVER_WIRELESS_AUTHENTICATION_stage1;
			/* save the read bytes to other place */
			*(READ_BYTE_UICC_TERMINAL + 251) = *(READ_BYTE_UICC_TERMINAL + 1);
		  *(READ_BYTE_UICC_TERMINAL + 252) = *(READ_BYTE_UICC_TERMINAL + 2);					
		}
		else
		{
#if (IF_LOG_OUTPUT)
	    printf("++++++++++++++++++++++++++++++++++ SIM data byte read error in GPIOTE_IRQHandler() ++++++++++++++++++++++++++++++++++\r\n");
#endif  			
		}
				
	  /* re-load watch dog request register */
	  NRF_WDT->RR[0] = 0x6E524635;
    /* clear SIM data reab byes number */
		SIM_READ_BYTES_NUMBER = 0;
	}
#endif
			
	/* GPIOTE event from phone CLOCK pin, trigger phone command byte reading */
	if ((NRF_GPIOTE->EVENTS_IN[1] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN1_Msk))
	{
		NRF_GPIOTE->EVENTS_IN[1] = 0;
	  CLOCK_PIN_COUNTER++;

	  if (CLOCK_PIN_COUNTER > 3)
	  {
	    /* clear CLK pin signal counter */
	    CLOCK_PIN_COUNTER = 0;
			/* clear interrupt events */
      clear_event_phone_command( );
			/* mark the phone command read flag */
#if (IF_SOFTDEIVE_USED)
			if (IF_SOFTDEVICE_RUNNING)
			{
			  FLAG_PHONE_COMMAND_READ = 1;
			}
#endif
								
			/* phone commnad read error */
			if (read_phone_command(PHONE_COMMAND, PIN_DATA_PHONE, ETU_TICKS_PHONE))
			{
				set_event_phone_command( );
#if (IF_SOFTDEIVE_USED)
				if (IF_SOFTDEVICE_RUNNING)
				{
			    FLAG_PHONE_COMMAND_READ = 0;
				}
#endif				
				return;
			}
					
#if (IF_LOG_OUTPUT)
			if (!IF_SOFTDEVICE_RUNNING)
			{
				/* wireless USIM used */
	      if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				{
	        printf("*********************** BLE off, NRF_RTC2->COUNTER = %ld, PHONE_WIRELESS_USED_USIM = 0x%x **********************\r\n", NRF_RTC2->COUNTER, PHONE_WIRELESS_USED_USIM);
				}
				/* local USIM used */
				else
				{
	        printf("*********************** BLE off, NRF_RTC2->COUNTER = %ld, local USIM = %d **********************\r\n", NRF_RTC2->COUNTER, (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1));
	      }
				printf_log_tx(5, PHONE_COMMAND + 1);
			}
#endif  
	    nrf_delay_ms(3);
				
#if (IF_SOFTDEIVE_USED)
			if (IF_SOFTDEVICE_RUNNING)
			{
		    /* set SWI3_EGU3 for phone command process */
		    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
				FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_PHONE_COMMAND;
			}
			else
#endif				
			{
				/* local USIM is used */
				if ((PHONE_LOCAL_USED_USIM & 0xF0) == 0)
				{
		      // main_smart_phone_sim_local_wireless(0, 0);
					main_usim_server_phone_command( );
				}
				/* wireless USIM is used */
				else
				{
		      /* all phone command */
		      main_smart_phone_sim_local_wireless(0, 0);
				}
		  }
		}
	}
}

/********************************************************************************/
void gpio_event_in_set_clock(void)
/*--------------------------------------------------------------------------------
| GPIOTE channel 1 set, GPIOTE from PIN_CLOCK_PHONE
|
--------------------------------------------------------------------------------*/
{
  /* GPIOTE channel 1, GPIOTE from PIN_CLOCK_PHONE */
  NRF_GPIOTE->CONFIG[1] =  (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)
                           | (PIN_CLOCK_PHONE << GPIOTE_CONFIG_PSEL_Pos)
                           | (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos);
}

/********************************************************************************/
void gpio_event_in_clear_clock(void)
/*--------------------------------------------------------------------------------
| GPIOTE channel 1 clear, GPIOTE from PIN_CLOCK_PHONE
|
--------------------------------------------------------------------------------*/
{
  /* clear GPIOTE channel 1, PIN_CLOCK_PHONE */
  NRF_GPIOTE->CONFIG[1] =  0;
	NRF_GPIOTE->EVENTS_IN[1] = 0;
}

#if (IF_SOFTDEIVE_USED)
/********************************************************************************/
void start_ble_data(void)
/*--------------------------------------------------------------------------------
| start softdevice BLE stack and related BLE service
|
--------------------------------------------------------------------------------*/
{
  uint32_t time_now_rtc2;

  /* start softdevice and BLE service stack */
	start_ble_ancs_c( ); 		
	rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
	
	/* check if softdevice is started */
	if (nrf_sdh_is_enabled( ))
	{
		/* BLE stack is started */
		IF_SOFTDEVICE_RUNNING = 1;		
	}
	else
	{
#if (IF_LOG_OUTPUT)
	  printf("----------------- start BLE stack error, reset --------------\r\n");
#endif
		/* system reset */
		sd_nvic_SystemReset( );							
	}
	
	time_now_rtc2 = NRF_RTC2->COUNTER; 
	/* open time slot session */
	while (sd_radio_session_open(time_slot_callback) != NRF_SUCCESS)
	{
		/* phone command time out */
		if (((NRF_RTC2->COUNTER - time_now_rtc2) > DATA_TX_TIME_MS) ||
		  (NRF_RTC2->COUNTER < time_now_rtc2))
		{
#if (IF_LOG_OUTPUT)
			printf("----------------- sd_radio_session_open() error, reset --------------\r\n");
#endif
			/* system reset */
			sd_nvic_SystemReset( );							
		}
	}	
	
	/* save the time of BLE stack started */
	BLE_START_TIME = NRF_RTC2->COUNTER;
	/* clear the semaphore for time slot request */			
  SEMAPHORE_TIME_SLOT_REQUEST = 0;
}

/********************************************************************************/
void stop_ble_data(void)
/*--------------------------------------------------------------------------------
| stop softdevice BLE stack and related BLE service
|
--------------------------------------------------------------------------------*/
{
	/* stop BLE stack softdevice */
	nrf_sdh_disable_request();
	
	/* check the softdevice running status */
	if (nrf_sdh_is_enabled( ))
	{
#if (IF_LOG_OUTPUT)
	  printf("----------------- stop BLE stack error, reset --------------\r\n");
#endif
		/* system reset */
		sd_nvic_SystemReset( );							
	}
	else
	{
		IF_SOFTDEVICE_RUNNING = 0;	
	}
		
  /* Set radio configuration parameters */
  radio_configure();
	
	/* save the time of BLE stack stopped */
	BLE_STOP_TIME = NRF_RTC2->COUNTER;
	/* clear the semaphore for time slot request */			
  SEMAPHORE_TIME_SLOT_REQUEST = 0;
}

/********************************************************************************/
void SWI3_EGU3_IRQHandler(void)
/*--------------------------------------------------------------------------------
| software interrupt 3 handler, for phone command process in time slot, 
| and other process in time slot
|
--------------------------------------------------------------------------------*/
{
  uint32_t request_time_slot_length_us;
 		
  switch (FLAG_SWI3_EGU3_IRQn)
	{
		case SWI3_EGU3_ISR_PHONE_COMMAND:
		{
#if (IF_LOG_OUTPUT)
	    printf("*********************** BLE on, NRF_RTC2->COUNTER = %ld, **********************\r\n", NRF_RTC2->COUNTER);
	    printf_log_tx(5, PHONE_COMMAND + 1);
			if (*(PHONE_COMMAND + 2) == 0xf2)
			{
	      printf("next command 0xf2 time = %ld\r\n", NEXT_PHONE_COMMAND_0XF2_TIME);	
			}		
#endif  
					
			/* phone command session for authentication 0x88/0xc0 */
		  if ((*(PHONE_COMMAND + 2) == 0x88) || (*(PHONE_COMMAND + 2) == 0xc0))
		  {
				/* stop BLE when 0x88 command */
				if (!AUTHEN_0x88_BLE_ON_OFF_RAM)
				{
				  /* stop BLE stack */
				  stop_ble_data( );
		      
					/* all phone command */
		      main_smart_phone_sim_local_wireless(0, 0);
				
			    /* re-start BLE softdevice */
				  start_ble_data( );
				}
				/* time slot request when 0x88 command */
				else
				{
				  /* wireless USIM authentication*/
				  if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				  {
				    PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_STAGE = PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_stage0;
				    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_PHONE_COMMAND_WIRELESS_AUTHENTICATION;
				    PHONE_COMMAND_AUTHENTICATION_WIRELESS_BLE_START_TIME = NRF_RTC2->COUNTER;					
            time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);
				  }
				  /* local USIM authentication */
				  else
				  {
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
				      printf("++++++++++++++++++ PHONE_LOCAL_USED_USIM value error, PHONE_LOCAL_USED_USIM = %d ++++++++++++++++++\r\n", PHONE_LOCAL_USED_USIM);
#endif
							PHONE_LOCAL_USED_USIM |= (1 << 0);
							set_default_usim0_etu( );		
					  }

					  if (*(PHONE_COMMAND + 2) == 0x88)
					  {
						  if ((SELECTED_FILE_SIM != 0x7fff) && ((SELECTED_FILE_SIM >> 16) != 0x7fff))
						  {
#if (IF_LOG_OUTPUT)
					      printf("----------------- SIM file 0x7FFF selection for autherntication --------------\r\n");
				        printf_selected_file(SELECTED_FILE_SIM);
#endif
							  /* stop BLE stack */
							  stop_ble_data( );
							
							  /* start sim clock signal */
							  clock_sim_start_4m(PIN_CLOCK_SIM);
					      sim_file_7fff_selected_check( );
						    /* stop SIM clock */
						    clock_sim_stop_4m(PIN_CLOCK_SIM);
							
						    /* re-start BLE softdevice */
							  start_ble_data( );
						  }							
							
						  WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage0;
					    TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION;
              time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);		
				    }
				    else if (*(PHONE_COMMAND + 2) == 0xc0)
				    {
	 			      USIM_SERVER_COMMAND_GET_DATA_BLE_STAGE = USIM_SERVER_COMMAND_GET_DATA_BLE_stage0;
	 			      TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_GET_DATA;
		          time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0xC0_US);
						}
				  }
			  }		
		  }			
			/* phone command session for USIM switch */
			else if ((*(PHONE_COMMAND + 2) == 0x14) && 
				((FETCH_COMMAND_TYPE == FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT) ||
				 (FETCH_COMMAND_TYPE == FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT) ||
				 (FETCH_COMMAND_TYPE == FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_DISPLAY_TEXT) ||
				 (FETCH_COMMAND_TYPE == FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_DISPLAY_TEXT)))
			{
			  stop_ble_data( );	
		    main_smart_phone_sim_local_wireless(0, 0);		
			}
			/* other phone command session */
			else
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
				
			  /* set the time slot signal callback flag */
			  TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_PHONE_COMMAND;
				time_slot_request(request_time_slot_length_us);
			}	
			break;
		}
			
		case SWI3_EGU3_ISR_USIM_SERVER_SIM_COMMAND_AUTHENTICATION_stage3:
		{
			WATCH_COMMAND_GSM_ALGORITHM_BLE_STAGE = USIM_SERVER_COMMAND_GSM_ALGORITHM_BLE_stage3;
			TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_SIM_COMMAND_AUTHENTICATION;
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_US);

			break;
		}
						
		case SWI3_EGU3_ISR_USIM_SERVER_WIRELESS_AUTHENTICATION_stage1:
		{
      TIME_SLOT_SIGNAL_TYPE_SET = TIME_SLOT_SIGNAL_USIM_SERVER_WIRELESS_AUTHENTICATION_SIM_COMMAND;	
		  USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_STAGE = USIM_SERVER_AUTHENTICATION_WIRELESS_BLE_stage1;					
      time_slot_request(TIMER_SLOT_PHONE_COMMAND_SIM_0x88_WIRELESS_US);

			break;
		}
		
		case SWI3_EGU3_ISR_SIM_CARD_STATE_RESET:
		{
#if (IF_LOG_OUTPUT)
			if (PIN_DATA_SIM == PIN_DATA_SIM0)
			{
        printf("\r\n---------------------------- Reset USIM0 ----------------------------\r\n"); 
			}
#if (PIN_VCC_SIM1 != PIN_NULL)					
			if (PIN_DATA_SIM == PIN_DATA_SIM1)
			{
        printf("\r\n---------------------------- Reset USIM1 ----------------------------\r\n"); 
			}
#endif			
#endif
			SELECTED_FILE_SIM = 0x0;
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
			      
		  NRF_TIMER1->TASKS_START = 1;
			/* stop BLE stack */
      if (IF_SOFTDEVICE_RUNNING)
			{
				stop_ble_data( );
			}
						
	    sim_file_3f00_selected_check( );
			
			/* renew RTC2 CC event interrupt and watch-dog */
		  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
						
			/* start sim clock signal */
			clock_sim_start_4m(PIN_CLOCK_SIM);
			sim_file_7fff_selected_check( );
			
			/* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
						
			/* re-start softdevice */
			start_ble_data( );

			break;
		}
		
		case SWI3_EGU3_ISR_STOP_BLE_STACK:
		{
#if (IF_LOG_OUTPUT)
      printf("\r\n+++++++++++++++++++++++++++++ Stop BLE Stack Soft-device +++++++++++++++++++++++++++++\r\n"); 
#endif
			if (IF_SOFTDEVICE_RUNNING)
			{
			  stop_ble_data( );
			}
			
			break;
		}
		
		default:
		{
#if (IF_LOG_OUTPUT)  
      printf("====================== default SWI3_EGU3_IRQHandler(), FLAG_SWI3_EGU3_IRQn = %d, =======================\r\n", FLAG_SWI3_EGU3_IRQn);
#endif			
			break;
		}
  }	
}
#endif

/********************************************************************************/
uint32_t read_phone_command(uint8_t *bytes_command, uint32_t pin_number, 
     uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command read, 5 bytes
|
--------------------------------------------------------------------------------*/
{
  uint8_t i;
	uint8_t parity_bit[1];
  uint32_t time_now_rtc2;
  uint32_t time_diff_0xf2;
  static uint32_t time_diff_0xf2_mean = 0;
  static uint32_t time_phone_command_0xf2;
	
  time_now_rtc2 = NRF_RTC2->COUNTER;
  
  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;
  
  /* read command from phone, 5 bytes */  
  for (i=1; i<6; i++)
  {
    while (nrf_gpio_pin_read(pin_number))
    {
      /* reset signal from phone is read */
      if (nrf_gpio_pin_read(PIN_RESET_PHONE) == 0)
      {
        nrf_delay_us(10);
        if (nrf_gpio_pin_read(PIN_RESET_PHONE) == 0)
        { 
					nrf_delay_us(50);
					
	        if (nrf_gpio_pin_read(PIN_RESET_PHONE) == 0)
	        { 
#ifdef PIN_LED_BLUE			
            nrf_gpio_pin_write(PIN_LED_BLUE, 0); 
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
#ifdef PIN_LED_YELLOW			
            nrf_gpio_pin_write(PIN_LED_YELLOW, 0); 
#endif 
#ifdef PIN_LED_ORANGE			
            nrf_gpio_pin_write(PIN_LED_ORANGE, 0); 
#endif
						/* stop BLE softdevice */
						if (IF_SOFTDEVICE_RUNNING)
						{
							/* system reset */
							sd_nvic_SystemReset( );														
						}
    
						/* clear the flag that phone initial USAT menu setup */
						FLAG_INITIAL_USAT_MENU_SETUP = 0;
	          /* reset session */
	          read_soft_warm_reset_phone(ETU_TICKS_PHONE_ATR_SESSION, pin_number);
					}
          
				  return(1);
        }
      }
    
      /* read phone command time out */
      if (((NRF_RTC2->COUNTER - time_now_rtc2) > PHONE_COMMAND_TIME_OUT_MS) ||
				(NRF_RTC2->COUNTER < time_now_rtc2))
      {
				if (i == 1)
				{
					return(1);
				}
				else
				{
          *(PHONE_COMMAND + 2) = 0;
					
          return(0);
				}
      }
    }
		
    read_byte(bytes_command + i, parity_bit, etu_length, pin_number);
  
    /* data GPIOTE signal, false alarm read, discard the byte */  
    if (*(bytes_command + 1) == 0xff && (*parity_bit == 0x1) && (i == 1))
    {
			return(1);
    }
  }
#if (IF_SOFTDEIVE_USED)
	if (IF_SOFTDEVICE_RUNNING)
	{
    FLAG_PHONE_COMMAND_READ = 0;
	}
#endif
	
	if (*(bytes_command + 2) == 0xf2)
	{
		time_diff_0xf2 = NRF_RTC2->COUNTER - time_phone_command_0xf2;
    time_phone_command_0xf2 = NRF_RTC2->COUNTER;
		if (abs(time_diff_0xf2 - 27500) < 2500)
		{
			if (abs(time_diff_0xf2_mean - time_diff_0xf2) > 1500)
		  {
				time_diff_0xf2_mean = time_diff_0xf2;
			}
			else
			{
				time_diff_0xf2_mean = (time_diff_0xf2_mean + time_diff_0xf2) / 2;
			}
		}
		NEXT_PHONE_COMMAND_0XF2_TIME = time_phone_command_0xf2 + time_diff_0xf2_mean;
	}
					
  return(0);
}

uint32_t read_soft_warm_reset_phone(uint32_t etu_ticks_initial, uint32_t pin_number)
/*--------------------------------------------------------------------------------
| warm reset phone-SIM session
|
--------------------------------------------------------------------------------*/
{
  uint32_t initial_timer;
  int32_t saadc_value;
  uint32_t return_value_reset;
	
  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  
	/* wait for phone USIM power supply with 3.0V */ 
  while (1)
  {
    /* get ADC sample value from VDD PIN */
    saadc_value = saadc_sample_value_get( );
 
    /* the ADC resolution is 8bit, the ADC value is 256 * (3.0 / 3.6) = 213 if high */
    if (saadc_value > 180)
    {  
#if (IF_LOG_OUTPUT)
      printf("NRF_SAADC->RESULT = %ld, \r\n", saadc_value);
#endif  
			
      break;
    }
		/* VDD is 1.8V */
    else
    {
#if (IF_LOG_OUTPUT)
      printf("NRF_SAADC->RESULT = %ld, \r\n", saadc_value);
#else
      nrf_delay_ms(1);
#endif
    }

    if((NRF_RTC2->COUNTER - initial_timer) > PHONE_COMMAND_TIME_OUT_MS)
    {
      *(PHONE_COMMAND + 2) = 0;
    
#if (IF_LOG_OUTPUT)  
      printf("Reset session read Reset pin time out ......, time used = %ldms\r\n", NRF_RTC2->COUNTER - initial_timer);
#endif
      /* time out return */
      return(1);
    }
  }
	
	/* wait for phone USIM RST pin high */
	while (nrf_gpio_pin_read(PIN_RESET_PHONE) == 0)
  {
    if((NRF_RTC2->COUNTER - initial_timer) > PHONE_COMMAND_TIME_OUT_MS)
    {
      *(PHONE_COMMAND + 2) = 0;
    
#if (IF_LOG_OUTPUT)  
      printf("Reset session read Reset pin time out ......, time used = %ldms\r\n", NRF_RTC2->COUNTER - initial_timer);
#endif
      /* time out return */
      return(1);
    }
  }
	   
  /* reset UICC-terminal session at phone side */
  return_value_reset = soft_warm_reset_phone(etu_ticks_initial, pin_number);  
  
  return (return_value_reset);
}

/********************************************************************************/
uint32_t usim_existence_detection(void)
/*--------------------------------------------------------------------------------
| detect the 2 USIMS present
|
--------------------------------------------------------------------------------*/
{
	uint32_t return_result_3f00_select;
	
  if (*((uint8_t *)WIRELESS_USIM_WORK_MODE_FLASH_ADDR) == 0)
	{
		wireless_usim_client_mode_set( );
#if (IF_LOG_OUTPUT)
	  if (!IF_SOFTDEVICE_RUNNING)	
	  {	
      printf("wireless USIM set to fixed client mode...\r\n");
	  }
#endif 
	} 
	
	/* USIM detection already done */
	if ((((PHONE_LOCAL_USED_USIM >> 1) & 1) == 1) && (((PHONE_LOCAL_USED_USIM >> 3) & 1) == 1))
	{
		return(0);
	}

	return_result_3f00_select = sim_file_3f00_selected_check( ); 
	
	/* configured default USIM card not detected */ 	
  if (return_result_3f00_select == 2)
	{
	  /* default USIM0 */
		if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 0)
		{
			/* set bit1=0, USIM0 is not existed */
			PHONE_LOCAL_USED_USIM &= 0xfe; 
			/* set bit1=1, USIM0 is already detected */
			PHONE_LOCAL_USED_USIM |= 0x02; 
			/* set bit0=0, USIM0 is not existed */
			USIM_CARD_PRESENCE &= 0xfe; 

		  nrf_gpio_pin_write(PIN_RESET_SIM0, 0);
		  nrf_gpio_pin_write(PIN_DATA_SIM0, 0);
		  nrf_gpio_pin_write(PIN_VCC_SIM0, 0);
			
#if (PIN_VCC_SIM1 == PIN_NULL)					
			/* set bit2=0, USIM1 is not existed */
			PHONE_LOCAL_USED_USIM &= 0xfb;
			/* set bit3=1, USIM1 is detected */
			PHONE_LOCAL_USED_USIM |= 0x08;
			/* set bit4=1 */
			PHONE_LOCAL_USED_USIM |= 0x10;
			/* set bit6=0, bit7=0 */
			PHONE_LOCAL_USED_USIM &= 0x3f;
			/* set bit1=0, USIM1 is not existed */
			USIM_CARD_PRESENCE &= 0xfd; 
#endif

#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
#if (PIN_VCC_SIM1 != PIN_NULL)					
			  printf("---SIM card 0 not detected, try SIM card 1 ...... \r\n");
#else
			  printf("---SIM card not detected, switch to wireless SIM client mode...... \r\n");								
#endif
		  }
#endif

#if (PIN_VCC_SIM1 != PIN_NULL)					
			set_default_usim1( );
					
			SELECTED_FILE_SIM = 0; 
	    if (sim_file_3f00_selected_check( ) == 2)
			{
				/* set bit2=0, USIM1 is not existed */
				PHONE_LOCAL_USED_USIM &= 0xfb;
				/* set bit3=1, USIM1 is detected */
				PHONE_LOCAL_USED_USIM |= 0x08;
				/* set bit4=1 */
				PHONE_LOCAL_USED_USIM |= 0x10;
				/* set bit6=0, bit7=0 */
				PHONE_LOCAL_USED_USIM &= 0x3f;
				/* set bit1=0, USIM1 is not existed */
				USIM_CARD_PRESENCE &= 0xfd; 
				
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---SIM card 1 not detected, switch to wireless SIM client mode, default USIM0 ...... \r\n");
		    }
#endif
			  nrf_gpio_pin_write(PIN_RESET_SIM1, 0);
			  nrf_gpio_pin_write(PIN_DATA_SIM1, 0);
			  nrf_gpio_pin_write(PIN_VCC_SIM1, 0);
			}
			else
			{
				/* set bit4=0, bit5=0, bit6=0, bit7=0 */
				PHONE_LOCAL_USED_USIM &= 0x0f;
				/* set bit2=1, bit3=1 */
				PHONE_LOCAL_USED_USIM |= 0x0c; 
				/* set bit1=1, USIM1 is existed */
				USIM_CARD_PRESENCE |= 0x02; 
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---SIM card 1 detected, use SIM card 1 ......\r\n");
		    }
#endif
			} 
#endif			
	  }
#if (PIN_VCC_SIM1 != PIN_NULL)					
		/* default USIM2 */
		else if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 1)
		{
			/* set bit2=0, USIM1 is not existed */
			PHONE_LOCAL_USED_USIM &= 0xfb;
			/* set bit3=1, USIM1 is already detected */
			PHONE_LOCAL_USED_USIM |= 0x08;
		  nrf_gpio_pin_write(PIN_RESET_SIM1, 0);
		  nrf_gpio_pin_write(PIN_DATA_SIM1, 0);
		  nrf_gpio_pin_write(PIN_VCC_SIM1, 0);
			/* set bit1=0, USIM1 is not existed */
			USIM_CARD_PRESENCE &= 0xfd; 
			
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
			  printf("---SIM card 1 not detected, try SIM card 0 ...... \r\n");
		  }
#endif
			
			set_default_usim0( );		
			 
      SELECTED_FILE_SIM = 0; 
	    if (sim_file_3f00_selected_check( ) == 2)
			{
				/* set bit0=0 */
				PHONE_LOCAL_USED_USIM &= 0xfe;
				/* set bit1=1 */
				PHONE_LOCAL_USED_USIM |= 0x02;
				/* set bit6=1 */
				PHONE_LOCAL_USED_USIM |= 0x40;
				/* set bit4=0, bit5=0 */
				PHONE_LOCAL_USED_USIM &= 0xcf;
				/* set bit0=0, USIM1 is not existed */
				USIM_CARD_PRESENCE &= 0xfe; 
							
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---SIM card 0 not detected, switch to wireless SIM client mode, default USIM1 ...... \r\n");
		    }
#endif
			  nrf_gpio_pin_write(PIN_RESET_SIM0, 0);
			  nrf_gpio_pin_write(PIN_DATA_SIM0, 0);
			  nrf_gpio_pin_write(PIN_VCC_SIM0, 0);
			}
			else
			{
				/* set bit4=0, bit5=0, bit6=0, bit7=0 */
				PHONE_LOCAL_USED_USIM &= 0x0f;
				/* set bit0=1, bit1=1 */
				PHONE_LOCAL_USED_USIM |= 0x03; 
				/* set bit0=1, USIM0 is existed */
				USIM_CARD_PRESENCE |= 0x01; 
				
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---SIM card 0 detected, use SIM card 0 ......\r\n");
		    }
#endif
			} 
		} 
#endif		
		else
		{
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
			  printf("+++ Wrong DEFAULT_USED_USIM value, DEFAULT_USED_USIM = %d +++\r\n", *((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR));
		  }
#endif
		  DEFAULT_USED_USIM_RAM = 0;
			
			start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);			
		}		
	}
	/* configured default USIM card detected */ 	
	else if (return_result_3f00_select == 0)
	{
		if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 0)
		{
			/* set bit0=1, bit1=1, bit2=0, bit3=0, bit4=0, bit5=0, bit6=0, bit7=0 */
			PHONE_LOCAL_USED_USIM = 0x03;
			/* set bit0=1, USIM0 existed */
			USIM_CARD_PRESENCE |= 0x01; 
			 
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
#if (PIN_VCC_SIM1 != PIN_NULL)					
			  printf("---USIM0 exsited, use USIM0 ......\r\n");
#else
			  printf("---USIM exsited ......\r\n");
#endif								
		  }
#endif
			
#if (PIN_VCC_SIM1 != PIN_NULL)					
			/* USIM1 detection */
			set_default_usim1( );		
			
			SELECTED_FILE_SIM = 0; 			
	    if (sim_file_3f00_selected_check( ) == 2)
			{
				/* set bit2=0 */
				PHONE_LOCAL_USED_USIM &= 0xfb;
				/* set bit3=1 */
				PHONE_LOCAL_USED_USIM |= 0x08;

				/* set bit1=0, USIM1 not existed */
				USIM_CARD_PRESENCE &= 0xfd; 
				
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---USIM1 not existed, not use USIM1 ......\r\n");
		    }
#endif
			  nrf_gpio_pin_write(PIN_RESET_SIM1, 0);
			  nrf_gpio_pin_write(PIN_DATA_SIM1, 0);
			  nrf_gpio_pin_write(PIN_VCC_SIM1, 0);
			}
			else
			{
				/* set bit2=0 */
				PHONE_LOCAL_USED_USIM &= 0xfb; 
				/* set bit3=1 */
				PHONE_LOCAL_USED_USIM |= 0x08; 
				
				/* set bit1=1, USIM1 existed */
				USIM_CARD_PRESENCE |= 0x02; 
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---USIM1 existed, not use USIM1 ......\r\n");
		    }
#endif
			} 
#endif						
		}
#if (PIN_VCC_SIM1 != PIN_NULL)					
		else if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 1)
		{
			/* set bit0=0, bit1=0, bit2=1, bit3=1, bit4=0, bit5=0, bit6=0, bit7=0 */
			PHONE_LOCAL_USED_USIM = 0x0c; 
			/* set bit1=1, USIM1 existed */
			USIM_CARD_PRESENCE |= 0x02; 
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
			  printf("---SIM card 1 detected, use SIM card 1 ......\r\n");
		  }
#endif
			
			/* USIM0 detection */
			set_default_usim0( );
			
			SELECTED_FILE_SIM = 0; 			 
	    if (sim_file_3f00_selected_check( ) == 2)
			{
				/* set bit0=0 */
				PHONE_LOCAL_USED_USIM &= 0xfe;
				/* set bit1=1 */
				PHONE_LOCAL_USED_USIM |= 0x02;

				/* set bit0=0, USIM1 not existed */
				USIM_CARD_PRESENCE &= 0xfe; 
							
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---USIM0 not existed, not use USIM0 ......\r\n");
		    }
#endif
			  nrf_gpio_pin_write(PIN_RESET_SIM0, 0);
			  nrf_gpio_pin_write(PIN_DATA_SIM0, 0);
			  nrf_gpio_pin_write(PIN_VCC_SIM0, 0);
			}
			else
			{
				/* set bit0=0 */
				PHONE_LOCAL_USED_USIM &= 0xfe; 
				/* set bit1=1 */
				PHONE_LOCAL_USED_USIM |= 0x02; 
				
				/* set bit0=1, USIM0 is existed */
				USIM_CARD_PRESENCE |= 0x01; 
				
#if (IF_LOG_OUTPUT)
		    if (!IF_SOFTDEVICE_RUNNING)
		    {
			    printf("---USIM0 existed, not use USIM0 ......\r\n");
		    }
#endif
			} 			
		}
#endif		
		else
		{
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
			  printf("+++ Wrong DEFAULT_USED_USIM value, DEFAULT_USED_USIM = %d +++\r\n", *((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR));
		  }
#endif			
		  DEFAULT_USED_USIM_RAM = 0;
			
			start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
		}		
	}	
	/* error */
	else
	{
		return(1);
	}
	
	return(0);	
}

/********************************************************************************/
void wireless_usim_client_mode_set(void)
/*--------------------------------------------------------------------------------
| if smart SIM connector is configured as wireless-USIM client mode, set the 
| value of PHONE_LOCAL_USED_USIM
|
--------------------------------------------------------------------------------*/
{
	/* set bit0=0, bit1=0, USIM0 USIM1 and not existed */
	USIM_CARD_PRESENCE = 0; 
	
  /* default USIM0 */
	if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 0)
	{
	  /* set bit0=1, bit2=0, bit3=1, bit4=0, bit4=1, bit5=0, bit6=0, bit7=0 */
	  PHONE_LOCAL_USED_USIM = 0x1a;
	}
	else
	{
	  /* set bit0=1, bit2=0, bit3=1, bit4=0, bit4=0, bit5=0, bit6=1, bit7=0 */
		PHONE_LOCAL_USED_USIM = 0x4a;
	}	
}

/********************************************************************************/
void wireless_usim_server_mode_set(void)
/*--------------------------------------------------------------------------------
| if smart SIM connector is configured as wireless-USIM client mode, set the 
| value of PHONE_LOCAL_USED_USIM
|
--------------------------------------------------------------------------------*/
{
	/* set bit0=1, bit1=1, USIM0 and USIM1 existed */
	USIM_CARD_PRESENCE = 0x3; 
	
  /* default USIM0 */
	if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 0)
	{
	  /* set bit0=1, bit1=1, bit2=0, bit3=1, bit4=0, bit5=0, bit6=0, bit7=0 */
	  PHONE_LOCAL_USED_USIM = 0xb;
	}
  /* default USIM1 */
	else
	{
	  /* set bit0=0, bit1=1, bit2=1, bit3=1, bit4=0, bit5=0, bit6=0, bit7=0 */
	  PHONE_LOCAL_USED_USIM = 0xe;
	}
}

/********************************************************************************/
uint32_t soft_warm_reset_phone(uint32_t etu_length, uint32_t pin_number_io)
/*--------------------------------------------------------------------------------
| read/write initial bytes of phone-SIM reset session
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t initial_timer;
	uint32_t atr_length = 19;
	uint8_t atr_bytes[19];
	uint8_t atr_check_bits[19];
	uint8_t pps_bytes[4];
	uint8_t pps_bits[4];

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  
  nrf_delay_us(200);
  /* write TS byte to phone */  
  write_byte(0x3b, 1, etu_length, pin_number_io);
 
  /* ATR information bytes */
  atr_bytes[0] = 0x9d;
  atr_bytes[1] = 0x96;
  atr_bytes[2] = 0x80;
  atr_bytes[3] = 0x1f;
  atr_bytes[4] = 0xc7;
  atr_bytes[5] = 0x80;
  atr_bytes[6] = 0x31;
  atr_bytes[7] = 0xe0;
  atr_bytes[8] = 0x73;
  atr_bytes[9] = 0xfe;
  atr_bytes[10] = 0x21;
  atr_bytes[11] = 0x13;
  atr_bytes[12] = 0x65;
  atr_bytes[13] = 0xd0;
  atr_bytes[14] = 0x1;
  atr_bytes[15] = 0x87;
  atr_bytes[16] = 0xf;
  atr_bytes[17] = 0x75;
  atr_bytes[18] = 0xf6;

  get_parity_byte(atr_length - 1, atr_bytes, atr_bytes + atr_length - 1);

  for (i=0; i<atr_length; i++)
  {
    get_parity_bit(atr_bytes[i], atr_check_bits + i);
  }

#if (IF_LOG_OUTPUT)
	if (!IF_SOFTDEVICE_RUNNING)
	{
    printf_log_rx(atr_length, atr_bytes);
  }
#endif

  /* write ATR bytes to phone */
  for (i=0; i<atr_length; i++)
  {
	  nrf_delay_us(100);
    write_byte(*(atr_bytes + i), *(atr_check_bits + i), etu_length, pin_number_io);
  } 
	
  /* read PPS bytes from phone */
  for (i=0; i<4; i++)
  {
    while (nrf_gpio_pin_read(pin_number_io))
    {
      if ((NRF_RTC2->COUNTER - initial_timer) > (DATA_TX_TIME_MS << 1))
      {
        return(1);
      }    
    }
    
    nrf_delay_us(10);
    read_byte(pps_bytes + i, pps_bits + i, etu_length, pin_number_io);
  }	
	
#if (IF_LOG_OUTPUT)
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(4, pps_bytes);
	}
#endif  
	
	if ((*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 1) != 1) ||
		((*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 0) != 1) && (*((uint8_t *)PHONE_USIM_CLK_FREQ_FLASH_ADDR + 0) != 2)))
	{
	  /* write the initial ETU value to the flash */
	  if (ETU_TICKS_PHONE_ATR_SESSION == 1550)
	  {
			*(PHONE_USIM_CLK_FREQ_RAM + 0) = 2; 
			*(PHONE_USIM_CLK_FREQ_RAM + 1) = 1; 
	  }
	  else if (ETU_TICKS_PHONE_ATR_SESSION == 1831)
	  {
			*(PHONE_USIM_CLK_FREQ_RAM + 0) = 1; 
			*(PHONE_USIM_CLK_FREQ_RAM + 1) = 1; 
	  }      
		*(PHONE_USIM_CLK_FREQ_RAM + 2) = *(pps_bytes + 2);
			
		if ((*(pps_bytes + 2) == 0x94) || (*(pps_bytes + 2) == 0x95) || (*(pps_bytes + 2) == 0x96))
		{				
			start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
			
#if (IF_LOG_OUTPUT)
	    if (!IF_SOFTDEVICE_RUNNING)	
	    {	
        printf("---Write ETU configuration to flash...\r\n");
	    }
#endif
	  }
		else
		{
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)	
		  {	
	      printf("ETU time not supported by UICC-terminal interface...\r\n");
		  }
#endif  
			return (1);
		}
	}
		
	if (*(pps_bytes + 2) == 0x94) /* data ETU is 64 clock cycle */
	{
    if (ETU_TICKS_PHONE_ATR_SESSION == 1831)
	  {
      /* 1 / 3.25 * 64 * 16 = 315  */  
		  ETU_TICKS_PHONE = 315;
	  }
    else if (ETU_TICKS_PHONE_ATR_SESSION == 1550)
	  {
			/* 1 / 3.84 * 64 * 16 = 266 */
		  ETU_TICKS_PHONE = 266;
	  }
	}
	else if (*(pps_bytes + 2) == 0x95) /* data ETU is 32 clock cycle */
	{
    if (ETU_TICKS_PHONE_ATR_SESSION == 1831)
	  {
		  ETU_TICKS_PHONE = 315 >> 1;
	  }
    else if (ETU_TICKS_PHONE_ATR_SESSION == 1550)
	  {
		  ETU_TICKS_PHONE = 266 >> 1;
	  }
	}
	else if (*(pps_bytes + 2) == 0x96) /* data ETU is 16 clock cycle */
	{
    if (ETU_TICKS_PHONE_ATR_SESSION == 1831)
	  {
		  ETU_TICKS_PHONE = 315 >> 2;
	  }
    else if (ETU_TICKS_PHONE_ATR_SESSION == 1550)
	  {
		  ETU_TICKS_PHONE = 266 >> 2;
	  }
	}
	else
	{
#if (IF_LOG_OUTPUT)
	  if (!IF_SOFTDEVICE_RUNNING)	
	  {	
      printf("ETU time not supported by UICC-terminal interface...\r\n");
	  }
#endif  
		return (1);
	}

	/* USIM card detection */
	usim_existence_detection( );

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;
  
#if (IF_LOG_OUTPUT)  
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_rx(4, pps_bytes);
	}
#endif 
	
	nrf_delay_ms(10); 
	
  /* write PPS bytes to phone */
  for (i=0; i<4; i++)
  {
	  nrf_delay_us(100);
    write_byte(*(pps_bytes + i), *(pps_bits + i), etu_length, pin_number_io);  
  }  
  
  return(0);
}  

/********************************************************************************/
uint32_t write_phone_response(uint32_t bytes_length, uint8_t *bytes_info, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command response feedback session, command 0xc0
|
--------------------------------------------------------------------------------*/
{
  uint8_t confirm_byte[1] = {0xc0};
  uint8_t status_bytes[2] = {0x90, 0x00};
  
  if (USAT_BYTE_LENGTH_BACK)
  {
    status_bytes[0] = 0x91;
    status_bytes[1] = USAT_BYTE_LENGTH_BACK;
  } 
   
  write_bytes (1, confirm_byte, etu_length, PIN_DATA_PHONE);
  write_bytes (bytes_length, bytes_info, etu_length, PIN_DATA_PHONE);
  write_bytes (2, status_bytes, etu_length, PIN_DATA_PHONE);

  return(0);  
}

/********************************************************************************/
uint32_t phone_command_pin_0x24(uint8_t *bytes_command, uint32_t etu_length_phone)
/*--------------------------------------------------------------------------------
| phone command session, supported phone command:
| 24: change PIN
| 26: disable PIN
| 28: enable PIN
|
--------------------------------------------------------------------------------*/
{
  uint32_t tmp_length;
  uint8_t tmp_bytes[2] = {0x90, 0x0};
  
  if ((*(bytes_command + 5)) == 0x00)
  {
    tmp_length = 256;
  }
  else
  {
    tmp_length = (*(bytes_command + 5));
  }
  
  /* write the SIM confirm bytes to phone */
  write_bytes(1, bytes_command + 2, etu_length_phone, PIN_DATA_PHONE);

  /* read phone command data */  
  if (read_bytes_phone(tmp_length, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length_phone))
  {  
    return(1);
  }  
  
  /* write the SIM returned status bytes to phone */
  write_bytes(2, tmp_bytes, etu_length_phone, PIN_DATA_PHONE);
   
  return(0);
}

/********************************************************************************/
uint32_t phone_command_manage_channel_0x70(uint8_t *bytes_command, uint32_t etu_length_phone)
/*--------------------------------------------------------------------------------
| phone command session, supported phone command:
| 70: manage channel 
|
--------------------------------------------------------------------------------*/
{
  uint8_t tmp_bytes[2] = {0x90, 0x0};

  /* write the SIM returned status bytes to phone */
  write_bytes(2, tmp_bytes, etu_length_phone, PIN_DATA_PHONE);
   
  return(0);
}

/********************************************************************************/
uint32_t phone_command_terminal_profile_0x10(uint8_t *phone_command, uint32_t etu_length, 
     uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| phone command terminal profile session, command 0x10, only read the data from 
| phone, not forward the data to SIM 
|
--------------------------------------------------------------------------------*/
{
  uint8_t phone_command_byte[1] = {0x10};
  uint8_t sim_status_byte[2] = {0x90, 0x0};
  
#if (IF_LOG_OUTPUT)  
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(1, phone_command + 2);
	}
#endif  
  
  write_bytes(1, phone_command_byte, etu_length, pin_number_phone);  
   
  if(read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, pin_number_phone, etu_length))
  {
    return(1);
  }
  
  write_bytes(2, sim_status_byte, etu_length, pin_number_phone);  
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_fetch_reset(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| USAT command NAA reset, phone start to access the network again
|
--------------------------------------------------------------------------------*/
{
  uint8_t usat_reset_bytes[14] = {0x12, 0xd0, 0x9, 0x81, 0x3, 0x1, 0x1, 0x0, 0x82, 0x2, 0x81, 0x82, 0x90, 0x0};
  
	if (FLAG_USIM_SWITCH)
	{
		usat_reset_bytes[7] = 0x4;
	}
  /* write SIM response to phone */   
  write_bytes(14, usat_reset_bytes, etu_length, pin_number_phone); 
	  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_fetch_diaplay_text(uint8_t *bytes_command, uint8_t string_length, 
     uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| USAT command text display, string display_string displayed on the phone screen 
|
--------------------------------------------------------------------------------*/
{
  uint8_t start_bytes[15] = {0x12, 0xd0, 0x15, 0x81, 0x3, 0x1, 0x21, 0x81, 0x82, 0x2, 0x81, 0x2, 0x8d, 0x4, 0x4};
  uint8_t end_bytes[2] = {0x90, 0x0};
  
  start_bytes[2] = 0xc + string_length; 
  start_bytes[13] = 0x1 + string_length; 
  
  /* write SIM response to phone */   
  write_bytes(15, start_bytes, etu_length, pin_number_phone);   
  write_bytes(string_length, display_string, etu_length, pin_number_phone);   
  write_bytes(2, end_bytes, etu_length, pin_number_phone);   
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_fetch_diaplay_text_variable(uint8_t *bytes_command, uint8_t string_length, 
     uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone, uint8_t variable_bytes)
/*--------------------------------------------------------------------------------
| USAT command text display, text display_string displayed on the phone screen 
\ with the number of phone SIM logical address
|
--------------------------------------------------------------------------------*/
{
  uint8_t tmp_bytes1[15] = {0x12, 0xd0, 0x15, 0x81, 0x3, 0x1, 0x21, 0x00, 0x82, 0x2, 0x81, 0x2, 0x8d, 0x4, 0x4};
  uint8_t tmp_bytes2[2] = {0x90, 0x0};
  
  tmp_bytes1[2] = 0xc + string_length; 
  tmp_bytes1[13] = 0x1 + string_length; 
  
  /* write SIM response to phone */   
  write_bytes(15, tmp_bytes1, etu_length, pin_number_phone);   
  write_bytes(string_length - 1, display_string, etu_length, pin_number_phone);   
  write_bytes(1, &variable_bytes, etu_length, pin_number_phone);   
  write_bytes(2, tmp_bytes2, etu_length, pin_number_phone);   
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_fetch_get_input_command_line(uint8_t *bytes_command, uint32_t etu_length, 
     uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| USAT command to get phone input command line interface
|
--------------------------------------------------------------------------------*/
{
  uint8_t USAT_bytes[131] = {0x12, 0xd0, 126, 0x81, 0x3, 0x1, 0x23, 0x1, 0x82, 0x2, 0x81, 0x82, 0x8d, 111, 0x4, 
               'C', 'o', 'n', 'f', 'i', 'g', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 
               's', ' ', '(', '3', '-', '3', '2', ' ', 'C', 'h', 'a', 'r', 's', ',', ' ',
							 '3', ' ', 'p', 'a', 'r', 't', 's', ')', ' ', '1', '.', ' ', 'c', 'm', 'd', ' ', '(', '3', ' ', 'C', 
							 'h', 'a', 'r', 's', ')', ' ', '2', '.', ' ', ':', ' ', '(', 'C', 'o', 'l', 'o', 'n', ',', ' ', '1', 
							 ' ', 'C', 'h', 'a', 'r', ')', ' ', '3', '.', ' ', 'c', 'm', 'd', ' ', 'p', 'a', 'r', 'a', 's', '(', 
							 '0', '-', '2', '8', ' ', 'C', 'h', 'a', 'r', 's', ')', '.', ' ', '0', '0', '0', ' ', 'l', 'i', 's',  
							 't',
               0x91, 0x2, 3, 32, 0x90, 0x0};
  
  /* write SIM response to phone */   
  write_bytes(131, USAT_bytes, etu_length, pin_number_phone);   
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_fetch_set_menu(uint8_t *bytes_command, uint32_t etu_length, 
     uint32_t pin_number_phone, uint8_t next_fetch_bytes_length)
/*--------------------------------------------------------------------------------
| USAT command set menu, for configuration parameters setting of wireless 
| SIM interface
|
--------------------------------------------------------------------------------*/
{
  uint8_t bytes_start[29] = {0x12,  0xd0,  0x81,  111,  0x81,  0x3,  0x1,  0x25,  0x0,  0x82,  0x2,  0x81,  0x82,  
              0x85,  14, 'S',  'S', '-',  'T',  'r',  'a',  'y',  ' ',  'C',  'o',  'n',  'f',  'i',  'g'};               
  uint8_t bytes_menu_about[22] = {0x8f,  20,  USAT_MENU_ABOUT,  'A', 'b', 'o',  'u',  't',  ' ',  'W',  'i', 
              'r',  'e',  'l', 'e', 's', 's',  ' ',  'U', 'S', 'I',  'M'};                              
	uint8_t bytes_menu_usim_switch[21] = {0x8f,  19,  USAT_MENU_USIM_SWITCH,  'U',  'S',  'I',  'M', '1',  '/', 'U',  'S',  'I',  'M', '2', ' ',
	            'S',  'w', 'i',  't', 'c', 'h'};
  uint8_t bytes_menu_wireless_sim_on_off[23] = {0x8f,  21,  USAT_MENU_WIRELESS_USIM_ON_OFF,  'W', 'i', 'r',  'e',  'l',  'e',  's',  's', 
              ' ',  'U',  'S',  'I', 'M', ' ', 'O',  'n', '/', 'O',  'f',  'f'}; 
	uint8_t bytes_menu_ble_on_off[14] = {0x8f,  12,  USAT_MENU_BLE_ON_OFF,  'A',  'N',  'C',  'S', ' ',  'O',  'n', '/', 'O', 'f', 'f'};
  uint8_t bytes_menu_command_line[18] = {0x8f,  16,  USAT_MENU_CONFIG_SETTINGS, 'C', 'o', 'n', 'f', 'i',  'g', ' ', 'S',  'e',  't',  't',  'i',  'n',  'g',  's'};
  uint8_t bytes_menu_device_reset[15] = {0x8f,  13,  USAT_MENU_DEVICE_RESET, 'D', 'e', 'v', 'i', 'c',  'e', ' ', 'R',  'e',  's',  'e',  't'};
  
	uint8_t bytes_end[2] = {0x90,  0x0};
  
  bytes_start[3] = 25 + 22 + 21 + 23 + 14 + 18 + 15;
           
  /* write SIM response to phone */  
  /* menu set beginning */   
  write_bytes(29, bytes_start, etu_length, pin_number_phone); 

  write_bytes(22, bytes_menu_about, etu_length, pin_number_phone);   
  write_bytes(21, bytes_menu_usim_switch, etu_length, pin_number_phone); 
  write_bytes(23, bytes_menu_wireless_sim_on_off, etu_length, pin_number_phone); 
  write_bytes(14, bytes_menu_ble_on_off, etu_length, pin_number_phone); 
  write_bytes(18, bytes_menu_command_line, etu_length, pin_number_phone);   
  write_bytes(15, bytes_menu_device_reset, etu_length, pin_number_phone); 
	
	if (next_fetch_bytes_length)
	{
		bytes_end[0] = 0x91; 
		bytes_end[1] = next_fetch_bytes_length; 
	}
   
	write_bytes(2, bytes_end, etu_length, pin_number_phone);   
  
  return(0);
}
              
/********************************************************************************/
uint32_t phone_command_fetch_idle_mode_text(uint8_t *bytes_command, uint8_t string_length, 
     uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| USAT command text display in idle mode, text display_string displayed on the 
| phone screen
|
--------------------------------------------------------------------------------*/
{
  uint8_t tmp_bytes1[15] = {0x12, 0xd0, 0x15, 0x81, 0x3, 0x1, 0x28, 0x00, 0x82, 
              0x2, 0x81, 0x2, 0x8d, 0x4, 0x4};
  uint8_t tmp_bytes2[2] = {0x90, 0x0};
  
  tmp_bytes1[2] = 0xc + string_length; 
  tmp_bytes1[13] = 0x1 + string_length; 
  
  /* write SIM response to phone */   
  write_bytes(15, tmp_bytes1, etu_length, pin_number_phone);   
  write_bytes(string_length, display_string, etu_length, pin_number_phone);   
  write_bytes(2, tmp_bytes2, etu_length, pin_number_phone);   
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_fetch_idle_mode_text_variable(uint8_t *bytes_command, uint8_t string_length, 
     uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| USAT command text display in idle mode, string display_string displayed on the 
| phone screen with number
|
--------------------------------------------------------------------------------*/
{
  uint8_t tmp_bytes1[15] = {0x12, 0xd0, 0x15, 0x81, 0x3, 0x1, 0x28, 0x00, 0x82, 0x2, 
              0x81, 0x2, 0x8d, 0x4, 0x4};
  uint8_t tmp_bytes2[2] = {0x90, 0x0};
  uint8_t variable_bytes = ((*((uint8_t*)DEVICE_LOG_ADD_FLASH_ADDR)) & 7) | 0x30;
  
  tmp_bytes1[2] = 0xc + string_length; 
  tmp_bytes1[13] = 0x1 + string_length; 
  
  /* write SIM response to phone */   
  write_bytes(15, tmp_bytes1, etu_length, pin_number_phone);   
  write_bytes(string_length - 1, display_string, etu_length, pin_number_phone);   
  write_bytes(1, &variable_bytes, etu_length, pin_number_phone);   
  write_bytes(2, tmp_bytes2, etu_length, pin_number_phone);   
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_search_record_0xa2(uint8_t *phone_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command search record session, command 0xa2
|
--------------------------------------------------------------------------------*/
{
  uint8_t record_confirm[1] = {0xa2};
  uint8_t record_status[2] = {0x6a, 0x83};
  
  write_bytes(1, record_confirm, etu_length, PIN_DATA_PHONE);   

  if(read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
  {  
    return(1);
  }
  
#if (IF_LOG_OUTPUT)
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf("++++++++++++++++++++++ search record, to be updated......\r\n");
	}
#endif  
  
  write_bytes(2, record_status, etu_length, PIN_DATA_PHONE);   

  return(0);  
}

/********************************************************************************/
uint32_t phone_command_fetch_0x12(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| phone command terminal fetch session, command 0x12
|
--------------------------------------------------------------------------------*/
{
	if (*(bytes_command + 5) == 0)
	{
		uint8_t tmp_response_byte[2] = {0x93, 0x0};
		
	  write_bytes(2, tmp_response_byte, etu_length, pin_number_phone);
		
		return(0);		
	}
	
	/* clear the phone commmand envelope 0xc2 counter */
	COMMOD_ENVELOPE_0XC2_COUNTER = 0;
	
  /* USAT menu switch */  
  switch (FETCH_COMMAND_TYPE)
  {
    /* set up USAT inital menu */
    case FETCH_COMMAND_TYPE_INITIAL_USAT_MENU_SETUP:
		{
		  /* update idle text */
			/* wireless USIM used */
			if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
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
		          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_CONNECTED_CLIENT;  
		  	      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM0_CONNECTED_STRING);
						}
					  /* USIM1 used */
						else
						{
		          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_CONNECTED_CLIENT;  
		  	      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM1_CONNECTED_STRING);
						}	
				  } 
					/* wireless USIM disconnected */
				  else 
				  {
					  /* USIM0 used */
						if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
						{  
		          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_DISCONNECTED_CLIENT;  
		  	      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT);
						}
					  /* USIM1 used */
						else
						{
		          FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_DISCONNECTED_CLIENT;  
		  	      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT);
						} 	
				  }
				}
				/* woreless USIM off */
				else
				{
	        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_OFF_ADDRESS_CLIENT;  
	  	    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT);  	
				}
			}
			/* local USIM used */
			else
			{
        /* set idle mode text to indicate Wireless SIM ON/OFF state */  
				/* wireless USIM on */
        if (WIRELESS_SIM_ON_OFF_RAM)
        {
					/* USIM0 used locallly */
					if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
					{
						FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_ON_SERVER;
						USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM0_DISPLAY_TEXT); 	
					}
					/* USIM1 used locallly */
					else
					{
						FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_ON_SERVER;
						USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_ON_USIM1_DISPLAY_TEXT); 	
					}
        }
				/* wireless USIM off */
        else
        {
					/* USIM0 used locallly */
					if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
					{
						FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_OFF_SERVER;
						USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM0_DISPLAY_TEXT); 	
					}
					/* USIM1 used locallly */
					else
					{
						FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_OFF_SERVER;
						USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_SERVER_OFF_USIM1_DISPLAY_TEXT); 	
					}
				}
      } 
			
			/* set phone USAT menu */
      phone_command_fetch_set_menu(bytes_command, etu_length, pin_number_phone, USAT_BYTE_LENGTH_BACK); 
			
			/* set the flag that phone initial USAT menu setup is finished */
			FLAG_INITIAL_USAT_MENU_SETUP = 1;
      
      break;
		}
		
    case FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_ON_SERVER:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_SERVER_ON_USIM0_DISPLAY_TEXT), SIM_SERVER_ON_USIM0_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;			
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_WIRELESS_USIM0_SIM_ON_SERVER; 
			 
      break;
		}
	  
    case FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_ON_SERVER:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_SERVER_ON_USIM1_DISPLAY_TEXT), SIM_SERVER_ON_USIM1_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;			
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_WIRELESS_USIM1_SIM_ON_SERVER;
			 
      break;
		}
	  
    case FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM0_SIM_OFF_SERVER:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_SERVER_OFF_USIM0_DISPLAY_TEXT), SIM_SERVER_OFF_USIM0_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_WIRELESS_USIM0_SIM_OFF_SERVER; 
			 
      break;
		}
	  
    case FETCH_COMMAND_TYPE_IDLE_TEXT_WIRELESS_USIM1_SIM_OFF_SERVER:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_SERVER_OFF_USIM1_DISPLAY_TEXT), SIM_SERVER_OFF_USIM1_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_WIRELESS_USIM1_SIM_OFF_SERVER; 
			 
      break;
		}
	  
    case FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_CONNECTED_CLIENT:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_CLIENT_ON_USIM0_CONNECTED_STRING), SIM_CLIENT_ON_USIM0_CONNECTED_STRING, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_SIM_ON_USIM0_CONNECTED_CLIENT;
			 
      break;
		}
			
    case FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_CONNECTED_CLIENT:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_CLIENT_ON_USIM1_CONNECTED_STRING), SIM_CLIENT_ON_USIM1_CONNECTED_STRING, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_SIM_ON_USIM1_CONNECTED_CLIENT;
			 
      break;
		}
			
    case FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM0_DISCONNECTED_CLIENT:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT), SIM_CLIENT_ON_USIM0_DISCONNECTED_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_SIM_ON_USIM0_DISCONNECTED_CLIENT;
			 
      break;
		}
			
    case FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_ON_USIM1_DISCONNECTED_CLIENT:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT), SIM_CLIENT_ON_USIM1_DISCONNECTED_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_SIM_ON_USIM1_DISCONNECTED_CLIENT;
			 
      break;
		}
			
    case FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_OFF_ADDRESS_CLIENT:
		{
      phone_command_fetch_idle_mode_text_variable(bytes_command, strlen((char *)SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT), SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
			IDLE_MODE_TEXT_TYPE = IDLE_MODE_TEXT_TYPE_SIM_OFF_ADDRESS_CLIENT;
			 
      break;
		}
	  	  	             	  	  	             				
    case FETCH_COMMAND_TYPE_DISPLAY_TEXT_ABOUT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)ABOUT_DISPLAY_TEXT), ABOUT_DISPLAY_TEXT, etu_length, pin_number_phone);  
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
      USAT_BYTE_LENGTH_BACK = 0;
      break;
		}		
      
    case FETCH_COMMAND_TYPE_WIRELESS_SIM_RESET_FETCH:
		{
      /* NAA reset */
      phone_command_fetch_reset(bytes_command, etu_length, pin_number_phone);
      /* clear the USAT NAA reset mark bit */
      CONNECTION_STATE &= 0xFFFFFFFB;
			  
      /* set idle mode text to indicate Wireless SIM ON/OFF state */
			/* wireless USIM used */ 
			if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
			{
		  	/* update idle text */
				if (WIRELESS_SIM_ON_OFF_RAM)
				{
				  if (WIRELESS_SIM_CONNECTION_STATUS) 
				  {
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
				  else 
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
				}
				else
				{
	        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_OFF_ADDRESS_CLIENT;  
	  	    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT);  	
				}
			}
			/* local USIM used */
			else
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
			  	  	             	  	  	             
    case FETCH_COMMAND_TYPE_BLE_STATUS_ON_DISPLAY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_STATUS_ON_DISPLAY_TEXT), BLE_STATUS_ON_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}		

    case FETCH_COMMAND_TYPE_BLE_STATUS_OFF_DISPLAY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_STATUS_OFF_DISPLAY_TEXT), BLE_STATUS_OFF_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;

      break;
		}	
		
		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT, etu_length, pin_number_phone);
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}

		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT, etu_length, pin_number_phone); 
		  USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}
		
		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT, etu_length, pin_number_phone);
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}

		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT, etu_length, pin_number_phone); 
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}
		
		case FETCH_COMMAND_TYPE_USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT), 
			    USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT, etu_length, pin_number_phone); 
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}
		
		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM0_YES_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM0_YES_DISPLAY_TEXT, etu_length, pin_number_phone);
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}

		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM1_YES_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM1_YES_DISPLAY_TEXT, etu_length, pin_number_phone);
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}
				
		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_NO_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM0_NO_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM0_NO_DISPLAY_TEXT, etu_length, pin_number_phone);
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}

		case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_NO_DISPLAY_TEXT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_SWITCH_USIM1_NO_DISPLAY_TEXT), 
			    USIM_SWITCH_USIM1_NO_DISPLAY_TEXT, etu_length, pin_number_phone);
			USAT_BYTE_LENGTH_BACK = 0;
					 
			break;
		}
		
    case FETCH_COMMAND_TYPE_SMART_USIM_TRAY_DEVICE_NAME_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)NAME_CHANGED_DISPLAY_TEXT), NAME_CHANGED_DISPLAY_TEXT, etu_length, pin_number_phone);  
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
      USAT_BYTE_LENGTH_BACK = 0;
      break;
		}
		      
    case FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT), WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}	
			
    case FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT), WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}	
		
		case FETCH_COMMAND_TYPE_BLE_BOND_DELETE_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_BOND_DELETE_DISPLAY_TEXT), BLE_BOND_DELETE_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}	

    case FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)CONFIG_SYCH_USIM_SERVER_DISPLAY_TEXT), CONFIG_SYCH_USIM_SERVER_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}	
					
    case FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY_CLIENT:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)CONFIG_SYCH_USIM_CLIENT_DISPLAY_TEXT), CONFIG_SYCH_USIM_CLIENT_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}	
					
	  case FETCH_COMMAND_TYPE_SYSTEM_KEY_INPUT_TEXT_DISPLAY:
	  {
	    phone_command_fetch_diaplay_text(bytes_command, strlen((char *)SYATEM_KEY_UPDATED_DISPLAY_TEXT), 
		       SYATEM_KEY_UPDATED_DISPLAY_TEXT, etu_length, pin_number_phone);  
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    USAT_BYTE_LENGTH_BACK = 0;
	    break;
		}
           
    case FETCH_COMMAND_TYPE_PASSWORD_INPUT_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)PASSWD_CHANGED_DISPLAY_TEXT), PASSWD_CHANGED_DISPLAY_TEXT, etu_length, pin_number_phone);  
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
      USAT_BYTE_LENGTH_BACK = 0;
      break;
		}
      
    case FETCH_COMMAND_TYPE_IDLE_TEXT_CONFIG_SYCH_SERVER:
		{
      phone_command_fetch_idle_mode_text(bytes_command, strlen((char *)CONFIG_SYCH_IDLE_DISPLAY_TEXT), CONFIG_SYCH_IDLE_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
      break;
		}
		
	  case FETCH_COMMAND_TYPE_CONFIG_RESTORE_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)CONFIG_RESTORE_DISPLAY_TEXT), CONFIG_RESTORE_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_LED_PATTERN_UPDATED_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)LED_PATTERN_UPDATED_DISPLAY_TEXT), LED_PATTERN_UPDATED_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
           	  	  	             	  	  	             				
    case FETCH_COMMAND_TYPE_COMMAND_LINE_INPUT:
		{
      phone_command_fetch_get_input_command_line(bytes_command, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      break;
		}
		
		case FETCH_COMMAND_TYPE_CONFIG_SETTING_KEY_WORD_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)WRONG_CONFIG_SETTING_KEY_WORD_DISPLAY_TEXT), WRONG_CONFIG_SETTING_KEY_WORD_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		          	  	  	             	  	  	             				
		case FETCH_COMMAND_TYPE_CONFIG_SETTINGS_KEY_WORD_LIST_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)CONFIG_SETTINGS_KEY_WORD_LIST_DISPLAY_TEXT), CONFIG_SETTINGS_KEY_WORD_LIST_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_LED_PATTERN_ERROR_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)LED_PATTERN_ERROR_DISPLAY_TEXT), LED_PATTERN_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_PASSWORD_ERROR_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)PASSWD_ERROR_DISPLAY_TEXT), PASSWD_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_SYSTEM_KEY_ERROR_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)SYATEM_KEY_ERROR_DISPLAY_TEXT), SYATEM_KEY_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_SMART_USIM_TRAY_DEVICE_NAME_ERROR_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)SYATEM_KEY_ERROR_DISPLAY_TEXT), SYATEM_KEY_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
	      		
		case FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM0_EF_UPDATE_DISPLAY_TEXT), USIM0_EF_UPDATE_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
	    
			break;
		}
	      		
		case FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM1_EF_UPDATE_DISPLAY_TEXT), USIM1_EF_UPDATE_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
	    
			break;
		}
	      		
		case FETCH_COMMAND_TYPE_USIM_EF_UPDATE_ERROR_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM_EF_UPDATE_ERROR_DISPLAY_TEXT), USIM_EF_UPDATE_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
	      		
		case FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY_CLIENT:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)REMOTE_USIM_RESTORE_DISPLAY_TEXT_CLIENT), REMOTE_USIM_RESTORE_DISPLAY_TEXT_CLIENT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
	      		
		case FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY:
	  {
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)REMOTE_USIM_RESTORE_DISPLAY_TEXT), REMOTE_USIM_RESTORE_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_CONFIG_RESTORE_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)CONFIG_RESTORE_SUCCESSFULLY_DISPLAY_TEXT), CONFIG_RESTORE_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM0_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT), USIM0_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)USIM1_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT), USIM1_EF_UPDATE_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
			      		
		case FETCH_COMMAND_TYPE_BLE_BOND_DELETE_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_BOND_DELETE_SUCCESSFULLY_DISPLAY_TEXT), BLE_BOND_DELETE_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
    case FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_ERROR_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)LOGICAL_ADDRESS_ERROR_DISPLAY_TEXT), LOGICAL_ADDRESS_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
					
    case FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text_variable(bytes_command, strlen((char *)LOGICAL_ADDRESS_SUCCESSFULLY_DISPLAY_TEXT), LOGICAL_ADDRESS_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone, (*((uint8_t*)DEVICE_LOG_ADD_FLASH_ADDR)) | 0x30);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
					      		
		case FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_ERROR_TEXT_DISPLAY:			      		
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)ICCID_2FE2_DATA_MODE_ERROR_DISPLAY_TEXT), ICCID_2FE2_DATA_MODE_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text_variable(bytes_command, strlen((char *)ICCID_2FE2_DATA_MODE_SUCCESSFULLY_DISPLAY_TEXT), ICCID_2FE2_DATA_MODE_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone, (*((uint8_t*)ICCID_2FE2_DATA_MODE_FLASH_ADDR)) | 0x30);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
					
		case FETCH_COMMAND_TYPE_WIRELESS_USIM_WORK_MODE_ERROR_TEXT_DISPLAY:			      		
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)WIRELESS_USIM_WORK_MODE_ERROR_DISPLAY_TEXT), WIRELESS_USIM_WORK_MODE_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
		case FETCH_COMMAND_TYPE_WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text_variable(bytes_command, strlen((char *)WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_DISPLAY_TEXT), WIRELESS_USIM_WORK_MODE_SUCCESSFULLY_DISPLAY_TEXT, etu_length, pin_number_phone, (*((uint8_t*)WIRELESS_USIM_WORK_MODE_FLASH_ADDR)) | 0x30);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
					
		case FETCH_COMMAND_TYPE_DEFAULT_USED_USIM_ERROR_TEXT_DISPLAY:			      		
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)DEFAULT_USED_USIM_ERROR_DISPLAY_TEXT), DEFAULT_USED_USIM_ERROR_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
			break;
		}
		
    case FETCH_COMMAND_TYPE_BLE_OFF_0X88_COMMAND_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_OFF_0X88_COMMAND_DISPLAY_TEXT), BLE_OFF_0X88_COMMAND_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}		

    case FETCH_COMMAND_TYPE_BLE_ON_0X88_COMMAND_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_ON_0X88_COMMAND_DISPLAY_TEXT), BLE_ON_0X88_COMMAND_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}		

    case FETCH_COMMAND_TYPE_BLE_ON_OFF_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_ON_OFF_DISPLAY_TEXT), BLE_ON_OFF_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}		

    case FETCH_COMMAND_TYPE_WIRELESS_SIM_ON_OFF_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)WIRELESS_SIM_ON_OFF_DISPLAY_TEXT), WIRELESS_SIM_ON_OFF_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}		

    case FETCH_COMMAND_TYPE_BLE_ON_OFF_0X88_TEXT_DISPLAY:
		{
      phone_command_fetch_diaplay_text(bytes_command, strlen((char *)BLE_ON_OFF_0X88_DISPLAY_TEXT), BLE_ON_OFF_0X88_DISPLAY_TEXT, etu_length, pin_number_phone);  
      USAT_BYTE_LENGTH_BACK = 0;
			
      break;
		}		

	  /* normal phone-SIM fetch command session */            
		/* to be updated for co-exist with BLE stack */
    default:
		{
#if (IF_LOG_OUTPUT)    
		  if (!IF_SOFTDEVICE_RUNNING)	
		  {	
        printf("--------------------------- Normal phone-SIM commande session for phone_command_fetch......, FETCH_COMMAND_TYPE = %d ---------------------------\r\n", FETCH_COMMAND_TYPE);
		  }
#endif 
	    usim_server_command_get_data(bytes_command, etu_length, ETU_TICKS_SIM);

      break; 
	  } 
  }
	
	return(0);
}

/********************************************************************************/
uint32_t phone_usat_menu_selection(uint8_t usat_menu_type, uint8_t *status_bytes)
/*--------------------------------------------------------------------------------
| phone usat menu selection results
|
--------------------------------------------------------------------------------*/
{
  switch(usat_menu_type)
  {
    case USAT_MENU_ABOUT:
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_DISPLAY_TEXT_ABOUT;
  
      *(status_bytes + 0) = 0x91;
      *(status_bytes + 1) = 14 + strlen((char *)ABOUT_DISPLAY_TEXT);
			USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)ABOUT_DISPLAY_TEXT);
      
			break;
		}
      
    case USAT_MENU_WIRELESS_USIM_ON_OFF:
		{
      *(status_bytes + 0) = 0x91;
			
			if (WIRELESS_SIM_ON_OFF_RAM == 0)
			{
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT_DISPLAY;
	      *(status_bytes + 1) = 14 + strlen((char *)WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT);
				USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT);
			}
			else
			{
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT_DISPLAY;
	      *(status_bytes + 1) = 14 + strlen((char *)WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT);
				USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT);
			}
      
      break;
		}
		
    case USAT_MENU_USIM_SWITCH:
		{
      *(status_bytes + 0) = 0x91;
			
			/* wireless USIM used */
      if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
			{
				/* wireless USIM interface on */
				if ((WIRELESS_SIM_ON_OFF_RAM) && (CONNECTION_STATE & 1))
				{
				  /* current used USIM0 */
				  if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
				  {
					  /* USIM1 existed */
					  if (((PHONE_WIRELESS_USED_USIM >> 2) & 1) == 1)
					  {
				      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT;
				      *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT); 
							USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT); 
					  }
					  /* USIM1 not existed */
					  else
					  {
				      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT;
				      *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT); 
							USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM1_NO_WIRELESS_DISPLAY_TEXT);  
					  }
				  }
				  /* current used USIM1 */
				  else if ((PHONE_WIRELESS_USED_USIM & 1) == 1)	
				  {
					  /* USIM0 existed */
					  if (((PHONE_WIRELESS_USED_USIM >> 1) & 1) == 1)
					  {
				      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT;
				      *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT); 
				      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT); 
					  }
					  /* USIM0 not existed */
					  else
					  {
				      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT;
				      *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT); 
				      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM0_NO_WIRELESS_DISPLAY_TEXT); 
					  }
				  }
			  }
				/* wireless USIM interface off */
			  else
				{
		      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT;
		      *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT); 						
		      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT); 						
				}
			}
			/* local USIM used */
			else
			{
			  /* USIM0 is used by phone locally */
			  if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
			  {
				  if (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1) 
				  {
					  FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_DISPLAY_TEXT;
					  *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM1_YES_DISPLAY_TEXT);  
					  USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM1_YES_DISPLAY_TEXT);  
				  }
					else
					{
					  FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_NO_DISPLAY_TEXT;
					  *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM1_NO_DISPLAY_TEXT);  
					  USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM1_NO_DISPLAY_TEXT);  
					}
			  }
				/* USIM1 is used by phone locally */
				else if (((PHONE_LOCAL_USED_USIM >> 2) & 1) == 1)
				{
				  if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1) 
				  {
					  FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_DISPLAY_TEXT;
					  *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM0_YES_DISPLAY_TEXT);  
					  USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM0_YES_DISPLAY_TEXT);  
				  }
					else
				  {
					  FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_NO_DISPLAY_TEXT;
					  *(status_bytes + 1) = 14 + strlen((char *)USIM_SWITCH_USIM0_NO_DISPLAY_TEXT);  
					  USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_SWITCH_USIM0_NO_DISPLAY_TEXT);  
				  }
				}
			}

      break;
		}						
			
    case USAT_MENU_BLE_ON_OFF:
		{
      *(status_bytes + 0) = 0x91;
			
			if (BLE_STATUS_ON_OFF_RAM == 0)
			{
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_STATUS_ON_DISPLAY_TEXT_DISPLAY;
	      *(status_bytes + 1) = 14 + strlen((char *)BLE_STATUS_ON_DISPLAY_TEXT);
	      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_STATUS_ON_DISPLAY_TEXT);
			}
			else
			{
        FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_STATUS_OFF_DISPLAY_TEXT_DISPLAY;
	      *(status_bytes + 1) = 14 + strlen((char *)BLE_STATUS_OFF_DISPLAY_TEXT);
	      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_STATUS_OFF_DISPLAY_TEXT);
			}
			
			break;
		}
     			
    case USAT_MENU_CONFIG_SETTINGS:
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_COMMAND_LINE_INPUT;
  
      *(status_bytes + 0) = 0x91;
      *(status_bytes + 1) = 128;     
			USAT_BYTE_LENGTH_BACK = 128;
      
      break;
		}
		
		case USAT_MENU_DEVICE_RESET:
		{	      
      break;
		}
		   
    default:
		{
      break; 
	  } 
  }
	
	return (0);  
}

/********************************************************************************/
uint32_t phone_command_envelope_0xc2(uint8_t *phone_command, uint32_t etu_length, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone command envelope session, command 0xc2
|
--------------------------------------------------------------------------------*/
{
  uint8_t envelope_confirm[1] = {0xc2};
  uint8_t status_bytes[2] = {0x90, 0x0};
  uint32_t tmp_length;
  uint8_t *tmp_save_data;
  uint32_t i;
	
	if (COMMOD_ENVELOPE_0XC2_COUNTER > 3)
	{
		COMMOD_ENVELOPE_0XC2_COUNTER = 0;
		/* return without response, it will be reseted by phone */					
		return(1);
	}

  if ((*(phone_command + 5)) == 0x00)
  {
    tmp_length = 256;	
  }	
  else
  {
    tmp_length = (*(phone_command + 5));
  }
  tmp_save_data = (uint8_t *)malloc((tmp_length) * sizeof(uint8_t));
  if (tmp_save_data == NULL)
  {
#if (IF_LOG_OUTPUT)    
		if (!IF_SOFTDEVICE_RUNNING)	
		{	
      printf("memory allocation error in phone_command_envelope_0xc2...\r\n");
		}
#endif    
    return(1);
  }
  
#if (IF_LOG_OUTPUT)  
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(1, phone_command + 2);
	}
#endif  
  
  write_bytes(1, envelope_confirm, etu_length, PIN_DATA_PHONE);   
	
#if (IF_SOFTDEIVE_USED)
	if (IF_SOFTDEVICE_RUNNING) 
	{
    if(read_bytes_ble(tmp_length, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
    {
	    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
#if (IF_LOG_OUTPUT)  
      printf("0xc2 error, reset ...\r\n");			
#endif
			/* system reset */
			sd_nvic_SystemReset( );
								
      return(1);
    }
	}
  else
#endif		
	{
    if(read_bytes_phone(tmp_length, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
    {
	    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
      return(1);
    }
	}
  /* save the read bytes */
  for (i=0; i<tmp_length; i++)
  {
	  *(tmp_save_data + i) = *(READ_BYTE_UICC_TERMINAL + i + 1);
  }
  
  if (*(READ_BYTE_UICC_TERMINAL + 1) == 0xd3) /* phone SAT menu selection */
  {
    COMMOD_ENVELOPE_0XC2_COUNTER++;
		
		phone_usat_menu_selection(*(READ_BYTE_UICC_TERMINAL + 9), status_bytes);

    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
  }
  /* normal phone-SIM envelope command */
	/* to be updated for co-exist with BLE stack */
  else
  {
		if (IF_SOFTDEVICE_RUNNING)	
		{
	    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
		}	
		else
		{ 	  
#if (IF_LOG_OUTPUT)    
      printf("Normal phone-SIM commande session for phone_command_envelope_0xc2......\r\n");
#endif   
	    /* write the phone command to SIM, 5 bytes */
	    write_bytes(5, phone_command + 1, ETU_TICKS_SIM, PIN_DATA_SIM);	

	    /* read SIM confirm bytes of the command, 1 or 2 bytes */	
	    if (usim_server_command_confirm_sim(phone_command, ETU_TICKS_SIM))
	    {
	      return(1);
	    }
	
      /* write phone envelope bytes to SIM, tmp_length bytes */		 
      write_bytes(tmp_length, tmp_save_data, ETU_TICKS_SIM, PIN_DATA_SIM);
	
      /* read SIM returned status bytes, 2 bytes */		
      if (read_bytes_sim(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
      {
        return(1);
      }
	
      /* write the SIM returned status bytes to phone */
      write_bytes(2, READ_BYTE_UICC_TERMINAL + 1, etu_length, PIN_DATA_PHONE);
		}	
  }
    
  return(0);  
}

/********************************************************************************/
uint32_t phone_command_terminal_response_0x14(uint8_t *phone_command, uint32_t etu_length,
         uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone command terminal response session, command 0x14
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t status_bytes[2] = {0x90, 0x0};  
  uint8_t response_confirm[1] = {0x14};
  uint8_t command_line_string[33];
  uint32_t usta_command_return_value;
    
  write_bytes(1, response_confirm, etu_length, PIN_DATA_PHONE); 
	
#if (IF_SOFTDEIVE_USED)
	if (IF_SOFTDEVICE_RUNNING) 
	{
    if(read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
    { 
			 
	    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
#if (IF_LOG_OUTPUT)  
      printf("0x14 error, reset ...\r\n");			
#endif
			/* system reset */
			sd_nvic_SystemReset( );
    }
	} 
  else
#endif		
	{
    if(read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
    {  
	    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
      return(1);
    }
	}
  
  /* phone feedback for get input */
  if (((*(READ_BYTE_UICC_TERMINAL + 1) == 0x81) || (*(READ_BYTE_UICC_TERMINAL + 1) == 0x1)) && (*(READ_BYTE_UICC_TERMINAL + 4) == 0x23) && 
    ((*(READ_BYTE_UICC_TERMINAL + 10) == 0x83) || (*(READ_BYTE_UICC_TERMINAL + 10) == 0x3))
    && (*(READ_BYTE_UICC_TERMINAL + 12) == 0x0))
  {
    switch (FETCH_COMMAND_TYPE)
    {    
      case FETCH_COMMAND_TYPE_COMMAND_LINE_INPUT:
			{
        /* get the user input cammand line string */
        command_line_string[0] = *(READ_BYTE_UICC_TERMINAL + 14) - 1;
        for (i=0; i<command_line_string[0]; i++)
        {
          command_line_string[i + 1] = *(READ_BYTE_UICC_TERMINAL + 16 + i);
        } 
        for (i=command_line_string[0]; i<32; i++)  
        {
          command_line_string[i + 1] = 0;
        }  
#if (IF_LOG_OUTPUT)
		    if (IF_SOFTDEVICE_RUNNING == 0)
		    {
          printf("====================== Command Line Input =======================\r\n");
			    printf_log_rx(command_line_string[0], command_line_string + 1);
		    }
#endif
				usta_command_return_value = usta_command_line(command_line_string);
				if (usta_command_return_value)
				{
          status_bytes[0] = 0x91;
          status_bytes[1] = usta_command_return_value;
				}
				
        break;
			}

      default:
        break;  
    }
  }
  /* phone feedback for diaplay text */
  else if (((*(READ_BYTE_UICC_TERMINAL + 1) == 0x81) || (*(READ_BYTE_UICC_TERMINAL + 1) == 0x1)) && (*(READ_BYTE_UICC_TERMINAL + 4) == 0x21) && 
    ((*(READ_BYTE_UICC_TERMINAL + 10) == 0x83) || (*(READ_BYTE_UICC_TERMINAL + 10) == 0x3))
    && (*(READ_BYTE_UICC_TERMINAL + 12) == 0x0))
	{
    switch (FETCH_COMMAND_TYPE)
    {
			/* USIM switch */
#if (PIN_VCC_SIM1 != PIN_NULL)					
			case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_WIRELESS_DISPLAY_TEXT:
			case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_WIRELESS_DISPLAY_TEXT:
			case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM0_YES_DISPLAY_TEXT:
			case FETCH_COMMAND_TYPE_USIM_SWITCH_USIM1_YES_DISPLAY_TEXT:
			{
				/* clear pending USIM binary update */
				SIM_FILE_UPDATE_BINARY_TASK_QUEUE = 0;
				
				/* wireless USIM used */
	      if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				{
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
				}
				/* local USIM used */
				else
				{
				  /* USIM0 is used by phone locally */
					if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
					{							
						DEFAULT_USED_USIM_RAM = 1;
						
					  /* set bit0=0 */
						PHONE_LOCAL_USED_USIM &= (~(1 << 0));
					  /* set bit2=1 */
						PHONE_LOCAL_USED_USIM |= (1 << 2);
						set_default_usim1_etu( );		
					}
					/* USIM1 is used by phone locally */
					else
					{							
				    /* write to flash */
					  DEFAULT_USED_USIM_RAM = 0;
						
					  /* set bit0=1 */
						PHONE_LOCAL_USED_USIM |= (1 << 0);
					  /* set bit2=0 */
						PHONE_LOCAL_USED_USIM &= (~(1 << 2));
						set_default_usim0_etu( );		
					}
				}	
					
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
	        status_bytes[0] = 0x91;
	        status_bytes[1] = 11;  
			    USAT_BYTE_LENGTH_BACK = 11;
				}
				
				break;
			}
#endif					
						
			case FETCH_COMMAND_TYPE_USIM_SWITCH_WIRELESS_OFF_DISPLAY_TEXT:
			{
		    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
				break;
			}
			
			case FETCH_COMMAND_TYPE_BLE_STATUS_ON_DISPLAY_TEXT_DISPLAY: 
			{
				BLE_STATUS_ON_OFF_RAM = 1;
						
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_OFFSET_POS);
									
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
				
				break;
			}
			
			case FETCH_COMMAND_TYPE_BLE_STATUS_OFF_DISPLAY_TEXT_DISPLAY: 
			{
				BLE_STATUS_ON_OFF_RAM = 0;
				
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_OFFSET_POS);
											
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
				
				break;
			}
			
			case FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_ON_DISPLAY_TEXT_DISPLAY: 
			{
				WIRELESS_SIM_ON_OFF_RAM = 1;
						
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_WIRELESS_SIM_ON_OFF_OFFSET_POS);
				
			  /* update idle text */
				/* wireless USIM */
				if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				{
					if (WIRELESS_SIM_CONNECTION_STATUS) 
					{
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
					else 
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
				}
				/* local USIM */
				else
				{
	        /* set idle mode text to indicate Wireless SIM ON/OFF state */  
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
				   
				break;
			}
			
			case FETCH_COMMAND_TYPE_WIRELESS_SIM_STATUS_OFF_DISPLAY_TEXT_DISPLAY: 
			{
				WIRELESS_SIM_ON_OFF_RAM = 0;
						
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_WIRELESS_SIM_ON_OFF_OFFSET_POS);
									
			  /* update idle text */
			  /* wireless USIM */
				if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				{
			  	/* update idle text */
		      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_SIM_OFF_ADDRESS_CLIENT;  
		  	  USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)SIM_CLIENT_OFF_ADDRESS_DISPLAY_TEXT);  	
				}
				/* local USIM */
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
				
				break;
			}
			
			case FETCH_COMMAND_TYPE_BLE_BOND_DELETE_TEXT_DISPLAY:
			{
				BLE_BOND_DEL_STATUS_RAM = 1;
				
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_BLE_BOND_DELETE_OFFSET_POS);
				
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
							
				break;				
			}
			
	    case FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY:
			{
        USIM_PHONE_CONNECTION_COMMAND |= (1 << CONFIG_SYNCH_POS);
				
				FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_IDLE_TEXT_CONFIG_SYCH_SERVER;
        status_bytes[0] = 0x91;
        status_bytes[1] = 14 + strlen((char *)CONFIG_SYCH_IDLE_DISPLAY_TEXT);  
				USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)CONFIG_SYCH_IDLE_DISPLAY_TEXT);				
							
				break;
			}
			
	    case FETCH_COMMAND_TYPE_CONFIG_RESTORE_TEXT_DISPLAY:
			{
				/* set the mark bit to indicate user data restore */ 
				BLE_STOPPED_TASK_QUEUE |= (1 << BLE_STOPPED_TASK_CONFIG_RESTORE_POS);
				
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
							
				break;
			}
			
			case FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_TEXT_DISPLAY:
			{
				/* set the mark bit to indicate user data restore */ 
				BLE_STOPPED_TASK_QUEUE |= (1 << BLE_STOPPED_TASK_USIM0_EF_UPDATE_POS);
				
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
							
				break;
			}
			
			case FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_TEXT_DISPLAY:
			{
				/* set the mark bit to indicate user data restore */ 
				BLE_STOPPED_TASK_QUEUE |= (1 << BLE_STOPPED_TASK_USIM1_EF_UPDATE_POS);
				
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
							
				break;
			}
						
			case FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY:
		  {
        USIM_PHONE_CONNECTION_COMMAND |= (1 << REMOTE_TASK_CONFIG_RESTORE_POS);
	      USAT_BYTE_LENGTH_BACK = 0;
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	    
				break;
			}
			
	    case FETCH_COMMAND_TYPE_BLE_OFF_0X88_COMMAND_TEXT_DISPLAY:
	    case FETCH_COMMAND_TYPE_BLE_ON_0X88_COMMAND_TEXT_DISPLAY:
			{
				AUTHEN_0x88_BLE_ON_OFF_RAM = 1 - (AUTHEN_0x88_BLE_ON_OFF_RAM & 1);
				
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_BLE_ON_OFF_0X88_COMMAND_OFFSET_POS);
				
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE;  
	      USAT_BYTE_LENGTH_BACK = 0;
										
	      break;
			}		
	      		
			default:
			{
				break;
			}
		}
	}
	else
	{
		if (USAT_BYTE_LENGTH_BACK)
		{
			status_bytes[0] = 0x91;  
			status_bytes[1] = USAT_BYTE_LENGTH_BACK;  
		}
	}
  
  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 

  return(0);  
}

/********************************************************************************/
uint32_t phone_command_default_response(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| phone command default response
|
--------------------------------------------------------------------------------*/
{
  uint8_t status_bytes[2] = {0x93, 0x00};
  
#if (IF_LOG_OUTPUT)
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf("++++++++++++++++++++++++++++++++++++++++ Phone command default response ++++++++++++++++++++++++++++++++++++++++\r\n");
	}
#endif
	
	/* update the ETU time length of phone-UICC interface, since it is possible that inccorect ETU length */
  update_etu_time( );
	  	
  /* write SIM response to phone */    
  write_bytes(2, status_bytes, etu_length, pin_number_phone);  
	 
  return(0);
}

/********************************************************************************/
uint32_t usta_command_line(uint8_t *command_line_string)
/*--------------------------------------------------------------------------------
| USAT command line input
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
	
	/* RST: reset teh device */
	if ((*(command_line_string + 0) == 3) &&
		((*(command_line_string + 1) == 'r') || (*(command_line_string + 1) == 'R')) &&
		((*(command_line_string + 2) == 's') || (*(command_line_string + 2) == 'S')) &&
		((*(command_line_string + 3) == 't') || (*(command_line_string + 3) == 'T')))
	{
#if (IF_LOG_OUTPUT)
	  printf("---------------------------------- USAT command to reset the device  -------------------------------\r\n");
#endif
		/* system reset */
		sd_nvic_SystemReset( );							
	}
	/* LED: LED flash pattern setting */
	else if ((*(command_line_string + 0) > 4) &&
		((*(command_line_string + 1) == 'l') || (*(command_line_string + 1) == 'L')) &&
		((*(command_line_string + 2) == 'e') || (*(command_line_string + 2) == 'E')) &&
		((*(command_line_string + 3) == 'd') || (*(command_line_string + 3) == 'D')))
	{
		if (*(command_line_string + 0) == 5)
		{
			*(LED_PATTERN_BLE_FLASH + 1) = *(command_line_string + 5);
		}
		else if(*(command_line_string + 0) > 5)
		{
      /* get the user input cammand line string */
      *(LED_PATTERN_BLE_FLASH + 0) = *(command_line_string + 0) - 4 <= 15 ? *(command_line_string + 0) - 4 : 15;
      for (i=0; i<*(LED_PATTERN_BLE_FLASH + 0); i++)
      {
        *(LED_PATTERN_BLE_FLASH + i + 1) = *(command_line_string + 5 + i);
      }
		  /* if the LED setting input is only the first char to indicate the LED flash speed, keep the LED pattern unchanged */
      for (i=(*(LED_PATTERN_BLE_FLASH + 0)); i<15; i++)  
      {
        *(LED_PATTERN_BLE_FLASH + i + 1) = 0;
			}
    }
		else
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_LED_PATTERN_ERROR_TEXT_DISPLAY;  
			return(14 + strlen((char *)LED_PATTERN_ERROR_DISPLAY_TEXT));		
			
			return(0);
		}  
		
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== LED Pattern Input =======================\r\n");
			printf_log_rx(*(LED_PATTERN_BLE_FLASH + 0), (uint8_t *)LED_PATTERN_BLE_FLASH + 1);
		}
#endif
		/* copy the LED flash pattern seting */
		memcpy(LED_PATTERN_RAM, (uint8_t *)LED_PATTERN_BLE_FLASH, KEY_LENGTH);
	  /* set flash write flag */
		FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
		/* set the mark bit for flash data write check */
		FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_LED_PATTERN_OFFSET_POS);
			
		return(0);
	}
	/* PWD: wireless USIM interface password input */
	else if ((*(command_line_string + 0) > 4) &&
		((*(command_line_string + 1) == 'p') || (*(command_line_string + 1) == 'P')) &&
		((*(command_line_string + 2) == 'w') || (*(command_line_string + 2) == 'W')) &&
		((*(command_line_string + 3) == 'd') || (*(command_line_string + 3) == 'D')))
	{
    /* get the user input cammand line string */
    uint8_t pwd_length = *(command_line_string + 0) - 4 <= KEY_LENGTH ? *(command_line_string + 0) - 4 : KEY_LENGTH;
		
    /* get the user input password, pad 0 at the end */
    for (i=0; i<pwd_length; i++)
    {
      *(USER_PASSWORD_RAM + i) = *(command_line_string + 5 + i);
    } 
    for (i=pwd_length; i<KEY_LENGTH; i++)  
    {
      *(USER_PASSWORD_RAM + i) = 0;
    }  
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== PWD Input =======================\r\n");
			printf_log_rx(KEY_LENGTH, USER_PASSWORD_RAM);
		}
#endif
		
		if (pwd_length < 6)
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_PASSWORD_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)PASSWD_ERROR_DISPLAY_TEXT));  
		}
    else
		{
			/* set flash write flag */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
			
			/* set the mark bit for flash data write check */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_USER_PASSWORD_OFFSET_POS);
			
			return(0);
 		} 
	}
	/* KEY: wireless USIM interface system key input */
	else if ((*(command_line_string + 0) > 4) &&
		((*(command_line_string + 1) == 'k') || (*(command_line_string + 1) == 'K')) &&
		((*(command_line_string + 2) == 'e') || (*(command_line_string + 2) == 'E')) &&
		((*(command_line_string + 3) == 'y') || (*(command_line_string + 3) == 'Y')))
	{
    /* get the user input cammand line string */
    uint8_t key_length = *(command_line_string + 0) - 4 <= KEY_LENGTH ? *(command_line_string + 0) - 4 : KEY_LENGTH;
		
    /* get the user input password, pad 0 at the end */
    for (i=0; i<key_length; i++)
    {
      *(SYSTEM_KEY_RAM + i) = *(command_line_string + 5 + i);
    } 
    for (i=key_length; i<KEY_LENGTH; i++)  
    {
      *(SYSTEM_KEY_RAM + i) = 0;
    }  
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== KEY Input =======================\r\n");
			printf_log_rx(KEY_LENGTH, SYSTEM_KEY_RAM);
		}
#endif
		if (key_length < 6)
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_SYSTEM_KEY_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)SYATEM_KEY_ERROR_DISPLAY_TEXT));  
		}
    else
		{
			/* set flash write flag */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
			
			/* set the mark bit for flash data write check */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_SYSTEM_KEY_OFFSET_POS);
			
			return(0);
		} 
	}
	
	/* NAM: smart SIM tray BLE name input */
	else if ((*(command_line_string + 0) > 4) &&
		((*(command_line_string + 1) == 'n') || (*(command_line_string + 1) == 'N')) &&
		((*(command_line_string + 2) == 'a') || (*(command_line_string + 2) == 'A')) &&
		((*(command_line_string + 3) == 'm') || (*(command_line_string + 3) == 'M')))
	{
    /* get the user input phone/watch name, pad 0 at the end */
    *(SMART_USIM_DEVICE_NAME_RAM + 0) =  *(command_line_string + 0) - 4 <= 15 ? *(command_line_string + 0) - 4 : 15;
    for (i=0; i<*(SMART_USIM_DEVICE_NAME_RAM + 0); i++)
    {
      *(SMART_USIM_DEVICE_NAME_RAM + i + 1) =  *(command_line_string + 5 + i);
    } 
    for (i=*(SMART_USIM_DEVICE_NAME_RAM + 0); i<15; i++)  
    {
      *(SMART_USIM_DEVICE_NAME_RAM + i + 1) = 0;
    }  
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== Name Input =======================\r\n");
			printf_log_rx(*(SMART_USIM_DEVICE_NAME_RAM + 0), SMART_USIM_DEVICE_NAME_RAM + 1);
		}
#endif
		
		if (*(SMART_USIM_DEVICE_NAME_RAM + 0) < 2)
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_SMART_USIM_TRAY_DEVICE_NAME_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)NAME_ERROR_DISPLAY_TEXT));      			
		}
    else
		{
			/* set flash write flag */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
			/* clear the mark bit for flash data write check */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_SMART_USIM_DEVICE_NAME_OFFSET_POS);
			
			return(0);
		}   
	}
	/* CRS: user config restore to default value */
	else if ((*(command_line_string + 0) == 3) &&
		((*(command_line_string + 1) == 'c') || (*(command_line_string + 1) == 'C')) &&
		((*(command_line_string + 2) == 'r') || (*(command_line_string + 2) == 'R')) &&
		((*(command_line_string + 3) == 's') || (*(command_line_string + 3) == 'S')))
	{
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== User Config Restore =======================\r\n");
		}
#endif
    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_CONFIG_RESTORE_TEXT_DISPLAY;
    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)CONFIG_RESTORE_DISPLAY_TEXT);		
    return(14 + strlen((char *)CONFIG_RESTORE_DISPLAY_TEXT));
	}
	/* CSY: user config synchronize from wireless USIM server to client */
	else if ((*(command_line_string + 0) == 3) &&
		((*(command_line_string + 1) == 'c') || (*(command_line_string + 1) == 'C')) &&
		((*(command_line_string + 2) == 's') || (*(command_line_string + 2) == 'S')) &&
		((*(command_line_string + 3) == 'y') || (*(command_line_string + 3) == 'Y')))
	{
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== User Config Synchronization =======================\r\n");
		}
#endif
		/* wireless USIM used */
    if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY_CLIENT;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)CONFIG_SYCH_USIM_CLIENT_DISPLAY_TEXT);
      return(14 + strlen((char *)CONFIG_SYCH_USIM_CLIENT_DISPLAY_TEXT));
 		}
		/* local USIM used */
		else
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_CONFIG_SYCH_TEXT_DISPLAY;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)CONFIG_SYCH_USIM_SERVER_DISPLAY_TEXT);
      return(14 + strlen((char *)CONFIG_SYCH_USIM_SERVER_DISPLAY_TEXT));
		}
	}
	/* DBN: Delete BLE device bonding with iOS device */
	else if ((*(command_line_string + 0) == 3) &&
		((*(command_line_string + 1) == 'd') || (*(command_line_string + 1) == 'D')) &&
		((*(command_line_string + 2) == 'b') || (*(command_line_string + 2) == 'B')) &&
		((*(command_line_string + 3) == 'n') || (*(command_line_string + 3) == 'N')))
	{
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== Delete BLE device bonding =======================\r\n");
		}
#endif
    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_BOND_DELETE_TEXT_DISPLAY;
    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_BOND_DELETE_DISPLAY_TEXT);
    return(14 + strlen((char *)BLE_BOND_DELETE_DISPLAY_TEXT));
	}
	/* USM: read the files data in USIM card and save to flash */
	else if ((*(command_line_string + 0) == 5) &&
		((*(command_line_string + 1) == 'u') || (*(command_line_string + 1) == 'U')) &&
		((*(command_line_string + 2) == 's') || (*(command_line_string + 2) == 'S')) &&
		((*(command_line_string + 3) == 'm') || (*(command_line_string + 3) == 'M')))
	{
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== update USIM file date =======================\r\n");
		}
#endif
		if ((*(command_line_string + 5) == '1') && (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1))
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM0_EF_UPDATE_TEXT_DISPLAY;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM0_EF_UPDATE_DISPLAY_TEXT);
      return(14 + strlen((char *)USIM0_EF_UPDATE_DISPLAY_TEXT));
		}
		else if ((*(command_line_string + 5) == '2') && (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1))
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM1_EF_UPDATE_TEXT_DISPLAY;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM1_EF_UPDATE_DISPLAY_TEXT);
      return(14 + strlen((char *)USIM1_EF_UPDATE_DISPLAY_TEXT));
		}
		else
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_USIM_EF_UPDATE_ERROR_TEXT_DISPLAY;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)USIM_EF_UPDATE_ERROR_DISPLAY_TEXT);
      return(14 + strlen((char *)USIM_EF_UPDATE_ERROR_DISPLAY_TEXT));
		}
	}
	/* RCR: USIM-phone connection command: remote USIM data restore */
	else if ((*(command_line_string + 0) == 3) &&
		((*(command_line_string + 1) == 'r') || (*(command_line_string + 1) == 'R')) &&
		((*(command_line_string + 2) == 'c') || (*(command_line_string + 2) == 'C')) &&
		((*(command_line_string + 3) == 'r') || (*(command_line_string + 3) == 'R')))
	{
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== remote USIM data restore =======================\r\n");
		}
#endif
		/* wireless USIM used */
    if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY_CLIENT;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)REMOTE_USIM_RESTORE_DISPLAY_TEXT_CLIENT);
      return(14 + strlen((char *)REMOTE_USIM_RESTORE_DISPLAY_TEXT_CLIENT));
 		}
		/* local USIM used */
		else
		{
      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_REMOTE_USIM_RESTORE_TEXT_DISPLAY;
      USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)REMOTE_USIM_RESTORE_DISPLAY_TEXT);
      return(14 + strlen((char *)REMOTE_USIM_RESTORE_DISPLAY_TEXT));
		}
	}
	/* ADD: wireless USIM logical address */
	else if (((*(command_line_string + 1) == 'a') || (*(command_line_string + 1) == 'A')) &&
		((*(command_line_string + 2) == 'd') || (*(command_line_string + 2) == 'D')) &&
		((*(command_line_string + 3) == 'd') || (*(command_line_string + 3) == 'D')))
	{
    /* get the user input wireless USIM logical address */
		if ( *(command_line_string + 0)  < 5)
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)LOGICAL_ADDRESS_ERROR_DISPLAY_TEXT));      			
		}
    else
		{
			DEVICE_LOGICAL_ADDRESS_RAM = (*(command_line_string + 5) & 7);
			if (DEVICE_LOGICAL_ADDRESS_RAM < 8)
			{
				USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS = DEVICE_LOGICAL_ADDRESS_RAM;
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_LOGICAL_ADDRESS_OFFSET_POS);
			
			  return(0);
			}
			else
			{
		    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_LOGICAL_ADDRESS_ERROR_TEXT_DISPLAY;  
		    return(14 + strlen((char *)LOGICAL_ADDRESS_ERROR_DISPLAY_TEXT));      			
			}
		}   
	}
	/* ICD: 2FE2 data mode */
	else if (((*(command_line_string + 1) == 'i') || (*(command_line_string + 1) == 'I')) &&
		((*(command_line_string + 2) == 'c') || (*(command_line_string + 2) == 'C')) &&
		((*(command_line_string + 3) == 'd') || (*(command_line_string + 3) == 'D')))
	{
    /* get the user input wireless USIM logical address */
		if ( *(command_line_string + 0)  < 5)
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)ICCID_2FE2_DATA_MODE_ERROR_DISPLAY_TEXT));      			
		}
    else
		{
			ICCID_2FE2_DATA_MODE_RAM = (*(command_line_string + 5) & 0x0f);
			if (ICCID_2FE2_DATA_MODE_RAM < 3)
			{
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_ICCID_2FE2_DATA_MODE_OFFSET_POS);
			
			  return(0);
			}
			else
			{
		    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_ICCID_2FE2_DATA_MODE_ERROR_TEXT_DISPLAY;  
		    return(14 + strlen((char *)ICCID_2FE2_DATA_MODE_ERROR_DISPLAY_TEXT));      			
			}
		}   
	}
	/* UWM: wireless USIM working mode */
	else if (((*(command_line_string + 1) == 'u') || (*(command_line_string + 1) == 'U')) &&
		((*(command_line_string + 2) == 'w') || (*(command_line_string + 2) == 'W')) &&
		((*(command_line_string + 3) == 'm') || (*(command_line_string + 3) == 'M')))
	{
    /* get the user input wireless USIM logical address */
		if ( *(command_line_string + 0)  < 5)
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_USIM_WORK_MODE_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)WIRELESS_USIM_WORK_MODE_ERROR_DISPLAY_TEXT));      			
		}
    else
		{
			WIRELESS_USIM_WORK_MODE_RAM = (*(command_line_string + 5) & 0x01);
			/* set flash write flag */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
			/* set the mark bit for flash data write check */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_WIRELESS_USIM_WORK_MODE_OFFSET_POS);
			
			return(0);
		}   
	}
#if (PIN_VCC_SIM1 != PIN_NULL)					
	/* DUM: Default USIM set to save flash */
	else if (((*(command_line_string + 1) == 'd') || (*(command_line_string + 1) == 'D')) &&
		((*(command_line_string + 2) == 'u') || (*(command_line_string + 2) == 'U')) &&
		((*(command_line_string + 3) == 'm') || (*(command_line_string + 3) == 'M')))
	{
    /* get the user input wireless USIM logical address */
		if ((*(command_line_string + 0)  < 5) || ((*(command_line_string + 5) != '1') && (*(command_line_string + 5) != '2')))
		{
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_DEFAULT_USED_USIM_ERROR_TEXT_DISPLAY;  
	    return(14 + strlen((char *)DEFAULT_USED_USIM_ERROR_DISPLAY_TEXT));      			
		}
    else
		{
			/* clear pending USIM binary update */
			SIM_FILE_UPDATE_BINARY_TASK_QUEUE = 0;
			
			if (*(command_line_string + 5) == '1')
			{
			  /* set the default used USIM */
				DEFAULT_USED_USIM_RAM = 0;
				
				/* wireless USIM used */
	      if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				{
					/* set bit0 = 0 */
					PHONE_WIRELESS_USED_USIM &= 0xfe;
						
					/* set bit4=1 */
				  PHONE_LOCAL_USED_USIM |= (1 << 4);
					/* set bit6=0 */
				  PHONE_LOCAL_USED_USIM &= (~(1 << 6));
				}
				/* local USIM used */
				else
				{						
					/* set bit0=1 */
				  PHONE_LOCAL_USED_USIM |= (1 << 0);
					/* set bit2=0 */
				  PHONE_LOCAL_USED_USIM &= (~(1 << 2));
					set_default_usim0_etu( );		
				}	
			}
			else if (*(command_line_string + 5) == '2')
			{
				DEFAULT_USED_USIM_RAM = 1;

				/* wireless USIM used */
	      if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				{
					/* set bit0 = 1 */
					PHONE_WIRELESS_USED_USIM |= 1;
						
					/* set bit4=0 */
					PHONE_LOCAL_USED_USIM &= (~(1 << 4));
					/* set bit6=1 */
					PHONE_LOCAL_USED_USIM |= (1 << 6);
				}
				/* local USIM used */
				else
				{
					/* set bit0=0 */
					PHONE_LOCAL_USED_USIM &= (~(1 << 0));
					/* set bit2=1 */
					PHONE_LOCAL_USED_USIM |= (1 << 2);
					set_default_usim1_etu( );		
				}	
			}
			
		  /* write to flash */
		  FLASH_UPDATE_WAITING_STAGE = 0; 
		  if (IF_SOFTDEVICE_RUNNING)
		  {
				/* set flash write flag */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS);
			
				return(0);
			}
			else
			{
		    FLAG_USIM_SWITCH = 1;
				
				start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
				/* set the mark bit for flash data write check */
				FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_DEFAULT_USED_USIM_OFFSET_POS);
								
			  FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_WIRELESS_SIM_RESET_FETCH;				
				return(11);
			}
		}   
	}
#endif	
	/* AUT: BLE on/off for phone command 0x88 authentication */
	else if (((*(command_line_string + 1) == 'a') || (*(command_line_string + 1) == 'A')) &&
		((*(command_line_string + 2) == 'u') || (*(command_line_string + 2) == 'U')) &&
		((*(command_line_string + 3) == 't') || (*(command_line_string + 3) == 'T')))
	{
		if (AUTHEN_0x88_BLE_ON_OFF_RAM)
		{
#if (IF_LOG_OUTPUT)
		  if (IF_SOFTDEVICE_RUNNING == 0)
		  {
        printf("====================== switch BLE off for 0x88 command =======================\r\n");
		  }
#endif
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_OFF_0X88_COMMAND_TEXT_DISPLAY;
	    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_OFF_0X88_COMMAND_DISPLAY_TEXT);
	    return(14 + strlen((char *)BLE_OFF_0X88_COMMAND_DISPLAY_TEXT));
		}
		else
		{
#if (IF_LOG_OUTPUT)
		  if (IF_SOFTDEVICE_RUNNING == 0)
		  {
        printf("====================== switch BLE on for 0x88 command =======================\r\n");
		  }
#endif
	    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_BLE_ON_0X88_COMMAND_TEXT_DISPLAY;
	    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)BLE_ON_0X88_COMMAND_DISPLAY_TEXT);
	    return(14 + strlen((char *)BLE_ON_0X88_COMMAND_DISPLAY_TEXT));
		}
	}
	/* 000: display command key word list */
	else if (((*(command_line_string + 1) == '0') || (*(command_line_string + 1) == '9')) &&
		((*(command_line_string + 2) == '0') || (*(command_line_string + 2) == '9')) &&
		((*(command_line_string + 3) == '0') || (*(command_line_string + 3) == '9')))
	{
#if (IF_LOG_OUTPUT)
		if (IF_SOFTDEVICE_RUNNING == 0)
		{
      printf("====================== Display Command Key Word List =======================\r\n");
		}
#endif
    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_CONFIG_SETTINGS_KEY_WORD_LIST_TEXT_DISPLAY;
    USAT_BYTE_LENGTH_BACK = 14 + strlen((char *)CONFIG_SETTINGS_KEY_WORD_LIST_DISPLAY_TEXT);
    return(14 + strlen((char *)CONFIG_SETTINGS_KEY_WORD_LIST_DISPLAY_TEXT));
	}
	else
	{
    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_CONFIG_SETTING_KEY_WORD_TEXT_DISPLAY;  
		return(14 + strlen((char *)WRONG_CONFIG_SETTING_KEY_WORD_DISPLAY_TEXT));		
	}		
		
	return(0);			
}

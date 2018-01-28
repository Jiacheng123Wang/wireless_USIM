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
#include "phone_wireless_interface.h"
#include "file_data_usim.h"
#include "flash_file_address.h"
#include "comm_uicc_terminal_interface.h"
#include "comm_phone_command.h"
#include "comm_initial.h"
#include "usim_server_wireless_interface.h"
#include "usim_server_sim_interface.h"
#include "nrf_nvic.h"
#include "wireless_sim_phone.h"
#include "usim_server_wireless_interface.h"
#include "global_data_exchange_extern.h"
#include "flash_data_write.h"
#include "define_all_data.h"
#include "sdk_config.h"

static volatile uint16_t   UPDATE_BNARY_OFFSET_6F08 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F09 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F5B = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_4F52 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_4F20 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F73 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F7E = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F7B = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F31 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F78 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6FAD = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F5C = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6FC4 = 0;
static volatile uint16_t   UPDATE_BNARY_OFFSET_6F56 = 0;

/********************************************************************************/
uint32_t phone_command_status_response_0xf2(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone)
/*--------------------------------------------------------------------------------
| phone command 0xf2: status
|
--------------------------------------------------------------------------------*/
{
  uint8_t fcp_ADF[0x13 + 1] = {0xff,  0xf2,  0x84,  0x10,  0xa0,  0x0,  0x0,  0x0, 0x87, 
          0x10,  0x2,  0xff,  0x86,  0xff,  0xff,  0x89,  0xff,  0xff,  0xff,  0xff};
  uint8_t status_bytes[2];
  static uint8_t fetch_type = 0;
  static uint8_t fetch_couter= 0;
  
#if (IF_LOG_OUTPUT)  
  uint32_t i;  
  
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf("Phone status command: ");

    for (i=0; i<5; i++)
    {
      printf("%x,  ", *(bytes_command + i + 1));
    }
    printf("\r\n");
  
    /* 0xf2 phone command, directly feedback */
    printf("Status command feedback: ");
    printf("%x,  ", 0x93);
    printf("%x,  \r\n", 0x0);
	}
#endif
	
	/* phone initial USAT menu setup is not finished */
	if ((FLAG_INITIAL_USAT_MENU_SETUP == 0) && (USAT_BYTE_LENGTH_BACK == 0))
	{
    /* initial USAT menu setting */
    USAT_BYTE_LENGTH_BACK = USAT_BYTE_LENGTH_BACK_INIT;
    FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_INITIAL_USAT_MENU_SETUP;		
	}
	
	COMMOD_ENVELOPE_0XC2_COUNTER = 0;

  if (*(bytes_command + 4) == 0x1)
	{
		if (*(bytes_command + 5) == 0x12)
		{
      write_bytes(0x13, fcp_ADF + 1, etu_length, pin_number_phone);
		
      if (USAT_BYTE_LENGTH_BACK)
      {
				if (fetch_type == FETCH_COMMAND_TYPE)
				{
					fetch_couter++;
				}
				else
				{
					fetch_type = FETCH_COMMAND_TYPE;
					fetch_couter = 0;
				}
				
				if (fetch_couter > 3)
				{
					fetch_type = 0;
					fetch_couter = 0;
					USAT_BYTE_LENGTH_BACK = 0;
		      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE; 
					
					/* system reset */
					sd_nvic_SystemReset( );					
					return(1);
				}
				else
				{
          status_bytes[0] = 0x91;
          status_bytes[1] = USAT_BYTE_LENGTH_BACK;
				}
      }
      else
      {
        status_bytes[0] = 0x93;
        status_bytes[1] = 0;
      }
	
      /* write SIM response to phone */		
      write_bytes(2, status_bytes, etu_length, pin_number_phone); 
		}
		else if (*(bytes_command + 5) == 0x0)
		{
	    if (USAT_BYTE_LENGTH_BACK)
	    {
				if (fetch_type == FETCH_COMMAND_TYPE)
				{
					fetch_couter++;
				}
				else
				{
					fetch_type = FETCH_COMMAND_TYPE;
					fetch_couter = 0;
				}
				
				if (fetch_couter > 3)
				{
					fetch_type = 0;
					fetch_couter = 0;
					USAT_BYTE_LENGTH_BACK = 0;
		      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE; 
					
					/* return without response, it will be reseted by phone */					
					return(1);
				}
				else
				{
          status_bytes[0] = 0x91;
          status_bytes[1] = USAT_BYTE_LENGTH_BACK;
				}
	    }
	    else
	    {
	      status_bytes[0] = 0x93;
	      status_bytes[1] = 0;
	    }
      /* write SIM response to phone */		
      write_bytes(2, status_bytes, etu_length, pin_number_phone); 
		}
		else
		{
      status_bytes[0] = 0x6c;
      status_bytes[1] = 0x12;
      /* write SIM response to phone */		
      write_bytes(2, status_bytes, etu_length, pin_number_phone); 
		}				
	}
	else
	{
    if (USAT_BYTE_LENGTH_BACK)
    {
			if (fetch_type == FETCH_COMMAND_TYPE)
			{
				fetch_couter++;
			}
			else
			{
				fetch_type = FETCH_COMMAND_TYPE;
				fetch_couter = 0;
			}
			
			if (fetch_couter > 3)
			{
				fetch_type = 0;
				fetch_couter = 0;
				USAT_BYTE_LENGTH_BACK = 0;
	      FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_NULL_VALUE; 
				
				/* system reset */
				sd_nvic_SystemReset( );					
				return(1);
			}
			else
			{
        status_bytes[0] = 0x91;
        status_bytes[1] = USAT_BYTE_LENGTH_BACK;
			}
    }
    else
    {
      status_bytes[0] = 0x93;
      status_bytes[1] = 0;
    }
  
    /* write SIM response to phone */    
    write_bytes(2, status_bytes, etu_length, pin_number_phone);  
  } 
	
  return(0);
}

/********************************************************************************/
uint32_t phone_command_unblock_pin_0x2c(uint8_t *phone_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command 0x2c: unlock user PIN
|
--------------------------------------------------------------------------------*/
{
uint8_t response_bytes[2] = {0x63, 0xca};

#if (IF_LOG_OUTPUT)    
  if (!IF_SOFTDEVICE_RUNNING)	
  {	
    printf("unblocked successfully......\r\n");
	}
#endif  
  
  write_bytes(2, response_bytes, etu_length, PIN_DATA_PHONE);   
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_verify_0x20(uint8_t *phone_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command 0x20: verify
|
--------------------------------------------------------------------------------*/
{
uint8_t response_bytes[2] = {0x63, 0xc3};

#if (IF_LOG_OUTPUT)    
  if (!IF_SOFTDEVICE_RUNNING)	
  {	
    printf("verified successfully......\r\n");
	}
#endif  
  
  write_bytes(2, response_bytes, etu_length, PIN_DATA_PHONE);   

  return(0);  
}

/********************************************************************************/
uint32_t usim_initial_selected_file_check(void)
/*--------------------------------------------------------------------------------
| select usim file 0x3f00, then 0x7fff
|
--------------------------------------------------------------------------------*/
{
	/* wireless USIM is used */
	if (PHONE_LOCAL_USED_USIM & 0xF0)
	{
		return(0);		
	}
	
	/* USIM0 is used by phone locally */
	if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
	{
#if (PIN_VCC_SIM1 != PIN_NULL)					
		/* if USIM1 is presented */
		if (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1)
		{
		  set_default_usim1_etu( );		
						 		
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
				/* start sim clock signal */
				clock_sim_start_4m(PIN_CLOCK_SIM);

			  if ((SELECTED_FILE_SIM == 0x3f00) || ((SELECTED_FILE_SIM >> 16) == 0x3f00))
				{
				  sim_file_7fff_selected_check( );
				}
		    sim_file_3f00_selected_check( );
		
				/* stop SIM clock */
				clock_sim_stop_4m(PIN_CLOCK_SIM);	
		  }
		}
#endif
		
		set_default_usim0_etu( );		
		if (!IF_SOFTDEVICE_RUNNING)
		{
			/* start sim clock signal */
			clock_sim_start_4m(PIN_CLOCK_SIM);

		  if ((SELECTED_FILE_SIM == 0x3f00) || ((SELECTED_FILE_SIM >> 16) == 0x3f00))
			{
				sim_file_7fff_selected_check( );
			}
		  sim_file_3f00_selected_check( );
			
			/* stop SIM clock */
			clock_sim_stop_4m(PIN_CLOCK_SIM);	
		}
	}	
#if (PIN_VCC_SIM1 != PIN_NULL)					
	/* USIM1 is used by phone locally */
	else
	{
		/* if USIM0 is presented */
		if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
	  {
		  set_default_usim0_etu( );		
		  if (!IF_SOFTDEVICE_RUNNING)
		  {
				/* start sim clock signal */
				clock_sim_start_4m(PIN_CLOCK_SIM);

			  if ((SELECTED_FILE_SIM == 0x3f00) || ((SELECTED_FILE_SIM >> 16) == 0x3f00))
				{
				  sim_file_7fff_selected_check( );
				}
		    sim_file_3f00_selected_check( );
			
				/* stop SIM clock */
				clock_sim_stop_4m(PIN_CLOCK_SIM);	
		  }
		}
		
		set_default_usim1_etu( );		
		if (!IF_SOFTDEVICE_RUNNING)
		{
			/* start sim clock signal */
			clock_sim_start_4m(PIN_CLOCK_SIM);

		  if ((SELECTED_FILE_SIM == 0x3f00) || ((SELECTED_FILE_SIM >> 16) == 0x3f00))
			{
			  sim_file_7fff_selected_check( );
		  }
		  sim_file_3f00_selected_check( );
			
			/* stop SIM clock */
			clock_sim_stop_4m(PIN_CLOCK_SIM);	
	  }
	}
#endif
		
	return(0);
}

/********************************************************************************/
uint32_t phone_command_select_0xa4(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone command 0xa4: file selection for necessary EFs
|
--------------------------------------------------------------------------------*/
{
  uint8_t selecet_fd[2];
  uint8_t confirm_bytes[1] = {0xa4};
  uint8_t no_file_bytes[2] = {0x6a, 0x82};
  uint8_t wrong_parameter_bytes[2] = {0x6a, 0x87};
  uint8_t status_bytes[2] = {0x90, 0x0};
  uint8_t class_not_support_bytes[2] = {0x6e, 0x0};

  /* SIM application command is not supported */
  if (*(phone_command + 1) == 0xa0)
  {
    write_bytes(2, class_not_support_bytes, etu_length, PIN_DATA_PHONE);
		
		return(0);    
	}
	
  if (USAT_BYTE_LENGTH_BACK)
  {
    status_bytes[0] = 0x91;
    status_bytes[1] = USAT_BYTE_LENGTH_BACK;
  } 
	
	/* USIM detection */
	if (!IF_SOFTDEVICE_RUNNING)
	{
#if (PIN_VCC_SIM1 != PIN_NULL)					
	  if (((PHONE_LOCAL_USED_USIM >> 1) == 0) || ((PHONE_LOCAL_USED_USIM >> 3) == 0))
#else
	  if ((PHONE_LOCAL_USED_USIM >> 1) == 0)
#endif		
	  {
	    usim_existence_detection( );
	  }
	  else
	  {
      usim_initial_selected_file_check( );
		}
  }
	     
  if (*(phone_command + 4) == 0x4)
  {
    selecet_fd[0] = 0x61;    
    if (*(phone_command + 5) == 0x2)
    {
      write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);     

#if (IF_SOFTDEIVE_USED)
      if (IF_SOFTDEVICE_RUNNING) 
			{
        if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
        {
	        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
			}
			else
#endif				
			{
        if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
        {
	        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
			}
			
      if ((*(READ_BYTE_UICC_TERMINAL + 1) ==0x7f) && (*(READ_BYTE_UICC_TERMINAL + 2) ==0xff))
      {
        SELECTED_FILE  = 0x7fff;
#if (IF_LOG_OUTPUT)
				if (!IF_SOFTDEVICE_RUNNING)	
				{	
          printf_selected_file(SELECTED_FILE);
				}
#endif
        
        selecet_fd[1] = 0x39;    
        write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
        
#if (IF_SOFTDEIVE_USED)
	      if (IF_SOFTDEVICE_RUNNING) 
				{
          if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
          {
		        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
            return(1);
          }
				}
				else
#endif					
				{
          if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
          {
		        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
            return(1);
          }
				}
        write_phone_response(0x39, M_FILE_CONTROL_PARAMETER_ADF + 1, etu_length);
      }
      else if ((*(READ_BYTE_UICC_TERMINAL + 1) ==0x3f) && (*(READ_BYTE_UICC_TERMINAL + 2) ==0x0))
      {
        SELECTED_FILE  = 0;
#if (IF_LOG_OUTPUT)
				if (!IF_SOFTDEVICE_RUNNING)	
				{	
          printf_selected_file(SELECTED_FILE);
				}
#endif
        
        selecet_fd[1] = 0x2b;    
        write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
        
#if (IF_SOFTDEIVE_USED)
	      if (IF_SOFTDEVICE_RUNNING) 
				{
          if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
          {
		        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
            return(1);
          }
				}
				else
#endif					
				{
          if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
          {
		        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
            return(1);
          }
				}
        write_phone_response(0x2b, M_FILE_CONTROL_PARAMETER_3F00 + 1, etu_length);
      }
      else
      {
				if ((*(phone_command + 3)) & 0x08) /* select by path from MF */
				{
					 SELECTED_FILE = 0;
           SELECTED_FILE |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x100);
           SELECTED_FILE |= (*(READ_BYTE_UICC_TERMINAL + 2));
				}
				else
				{
          if ((SELECTED_FILE == 0x7fff) || (SELECTED_FILE == 0x7fff5f3b) ||(SELECTED_FILE == 0x7f10) ||
						(SELECTED_FILE == 0x7f105f3a) || (SELECTED_FILE == 0x7f105f3c))
          {
            SELECTED_FILE <<= 16;
          }
          SELECTED_FILE &= MASK_SELECTED_FILE;
          SELECTED_FILE |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x100);
          SELECTED_FILE |= (*(READ_BYTE_UICC_TERMINAL + 2));
				}
        
#if (IF_LOG_OUTPUT)
				if (!IF_SOFTDEVICE_RUNNING)	
				{	
          printf_selected_file(SELECTED_FILE);
				}
#endif
        
        switch (SELECTED_FILE)
        {
          case 0x2fe2:
					{
    		    selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
			      if (IF_SOFTDEVICE_RUNNING) 
						{
							if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
						}
            else
#endif							
						{
							if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
						}
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_2FE2 + 1, etu_length);
            break;
					}

          case 0x2f05:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
			      if (IF_SOFTDEVICE_RUNNING) 
						{
							if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
						{
							if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
						}
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_2F05 + 1, etu_length);
            break;
					}

          case 0x2f00:
					{
            selecet_fd[1] = 0x28;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
			      if (IF_SOFTDEVICE_RUNNING) 
						{
							if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
						}
            else
#endif							
						{
							if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
						}
            write_phone_response(0x28, M_FILE_CONTROL_PARAMETER_2F00 + 1, etu_length);
            break;
					}

          case 0x7fff6fb7:
					{
            selecet_fd[1] = 0x28;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x28, M_FILE_CONTROL_PARAMETER_7FFF_6FB7 + 1, etu_length);
            break;
					}

          case 0x7fff6fad:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6FAD + 1, etu_length);
            break;
					}

          case 0x7fff6f07:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F07 + 1, etu_length);
            break;
					}

          case 0x7fff6f78:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F78 + 1, etu_length);
            break;
					}

          case 0x7fff6f7e:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F7E + 1, etu_length);
            break;
					}

          case 0x7fff6f38:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F38 + 1, etu_length);
            break;
					}

          case 0x7fff6f08:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F08 + 1, etu_length);
            break;
					}

          case 0x7fff6f09:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F09 + 1, etu_length);
            break;
					}

          case 0x7fff6f5b:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F5B + 1, etu_length);
            break;
					}

          case 0x7fff6f5c:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F5C + 1, etu_length);
            break;
					}

          case 0x7fff6fc4:
					{
            selecet_fd[1] = 0x24;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x24, M_FILE_CONTROL_PARAMETER_7FFF_6FC4 + 1, etu_length);
            break;
					}
        
          case 0x7fff6f7b:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F7B + 1, etu_length);
            break;
					}
                
          case 0x7fff6f06:
					{
            selecet_fd[1] = 0x28;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x28, M_FILE_CONTROL_PARAMETER_7FFF_6F06 + 1, etu_length);
            break;
					}
        
          case 0x7fff6f31:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F31 + 1, etu_length);
            break;
					}

          case 0x7fff6f73:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F73 + 1, etu_length);
            break;
					}

          case 0x7fff6f56:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F56 + 1, etu_length);
            break;
					}

          case 0x7fff6f4b:
					{
            selecet_fd[1] = 0x27;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F4B + 1, etu_length);
            break;
					}

          case 0x7fff6f49:
					{
            selecet_fd[1] = 0x27;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F49 + 1, etu_length);
            break;
					}

          case 0x7fff6f3c:
					{
            selecet_fd[1] = 0x27;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F3C + 1, etu_length);
            break;
					}

          case 0x7fff6f43:
					{
            selecet_fd[1] = 0x24;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F43 + 1, etu_length);
            break;
					}

          case 0x7fff6f42:
					{
            selecet_fd[1] = 0x1e;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F42 + 1, etu_length);
            break;
					}

          case 0x7fff6f46:
					{
            selecet_fd[1] = 0x18;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x18, M_FILE_CONTROL_PARAMETER_7FFF_6F46 + 1, etu_length);
            break;
					}

          case 0x7fff5f3b4f52:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_5F3B_4F52 + 1, etu_length);
            break;
					}

          case 0x7fff5f3b4f20:
					{
            selecet_fd[1] = 0x25;    
            write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
            if (IF_SOFTDEVICE_RUNNING) 
            {
              if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
              {
                write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            else
#endif							
            {
              if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
              {
				        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
                return(1);
              }
            }
            write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_5F3B_4F20 + 1, etu_length);
            break;
					}
      
          default:
					{
#if (IF_LOG_OUTPUT)
					  if (!IF_SOFTDEVICE_RUNNING)	
					  {	
              printf("selected file not found......\r\n");
					  }
#endif

            write_bytes(2, no_file_bytes, etu_length, PIN_DATA_PHONE);
            break; 
					}     
        }
      }
    }
    else if (*(phone_command + 5) == 0x4)
    {
      write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE); 
      
#if (IF_SOFTDEIVE_USED)
			if (IF_SOFTDEVICE_RUNNING) 
      {
			  if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
        {
	        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
			}
			else
#endif				
      {
			  if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
        {
	        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
			}
      SELECTED_FILE  = 0;
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x1000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 2) * 0x10000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 3) * 0x100);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 4));
#if (IF_LOG_OUTPUT)
			if (!IF_SOFTDEVICE_RUNNING)	
			{	
        printf_selected_file(SELECTED_FILE);
			}
#endif
      
      switch (SELECTED_FILE)
      {
        case 0x7fff6fb7:
				{
          selecet_fd[1] = 0x28;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x28, M_FILE_CONTROL_PARAMETER_7FFF_6FB7 + 1, etu_length);
          break;
				}

        case 0x7fff6fad:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6FAD + 1, etu_length);
          break;
				}

        case 0x7fff6f07:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F07 + 1, etu_length);
          break;
				}

        case 0x7fff6f78:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F78 + 1, etu_length);
          break;
				}

        case 0x7fff6f7e:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F7E + 1, etu_length);
          break;
				}

        case 0x7fff6f38:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F38 + 1, etu_length);
          break;
				}

        case 0x7fff6f08:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F08 + 1, etu_length);
          break;
				}

        case 0x7fff6f09:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F09 + 1, etu_length);
          break;
				}

        case 0x7fff6f5b:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F5B + 1, etu_length);
          break;
				}

        case 0x7fff6f5c:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F5C + 1, etu_length);
          break;
				}

        case 0x7fff6fc4:
				{
          selecet_fd[1] = 0x24;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x24, M_FILE_CONTROL_PARAMETER_7FFF_6FC4 + 1, etu_length);
          break;
				}
      
        case 0x7fff6f7b:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F7B + 1, etu_length);
          break;
				}
              
        case 0x7fff6f06:
				{
          selecet_fd[1] = 0x28;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x28, M_FILE_CONTROL_PARAMETER_7FFF_6F06 + 1, etu_length);
          break;
				}
      
        case 0x7fff6f31:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F31 + 1, etu_length);
          break;
				}

        case 0x7fff6f73:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F73 + 1, etu_length);
          break;
				}

        case 0x7fff6f56:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_6F56 + 1, etu_length);
          break;
				}
          
        case 0x7fff6f4b:
				{
          selecet_fd[1] = 0x27;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F4B + 1, etu_length);
          break;
				}
        
        case 0x7fff6f49:
				{
          selecet_fd[1] = 0x27;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F49 + 1, etu_length);
          break;
				}

        case 0x7fff6f3c:
				{
          selecet_fd[1] = 0x28;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F3C + 1, etu_length);
          break;
				}

        case 0x7fff6f43:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F43 + 1, etu_length);
          break;
				}

        case 0x7fff6f42:
				{
          selecet_fd[1] = 0x1e;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(selecet_fd[1], M_FILE_CONTROL_PARAMETER_7FFF_6F42 + 1, etu_length);
					
          break;
				}
      
        case 0x7fff6f46:
				{
          selecet_fd[1] = 0x18;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
            
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
				      write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x18, M_FILE_CONTROL_PARAMETER_7FFF_6F46 + 1, etu_length);
					
          break;
			  }

        default:
				{
#if (IF_LOG_OUTPUT)
				  if (!IF_SOFTDEVICE_RUNNING)	
				  {
            printf("selected file not found......\r\n");
				  }
#endif
        
          write_bytes(2, no_file_bytes, etu_length, PIN_DATA_PHONE);
					
          break;
				}     
      }  
    }
    else if (*(phone_command + 5) == 0x6)
    {
      write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
#if (IF_SOFTDEIVE_USED)
      if (IF_SOFTDEVICE_RUNNING) 
      {
        if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
        {
          write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
      }
      else
#endif				
      {
	      if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
	      {
		      write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
	        return(1);
	      }
      }

      SELECTED_FILE  = 0;
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x10000000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 2) * 0x100000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 3) * 0x1000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 4) * 0x10000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 5) * 0x100);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 6));
#if (IF_LOG_OUTPUT)
			if (!IF_SOFTDEVICE_RUNNING)	
			{	
        printf_selected_file(SELECTED_FILE);
			}
#endif	  
      switch (SELECTED_FILE)
      {
        case 0x7fff5f3b4f20:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_5F3B_4F20 + 1, etu_length);
					
          break;
				}
      
        case 0x7fff5f3b4f52:
				{
          selecet_fd[1] = 0x25;    
          write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
          
#if (IF_SOFTDEIVE_USED)
          if (IF_SOFTDEVICE_RUNNING) 
          {
            if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
            {
              write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          else
#endif						
          {
            if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
            {
			        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
              return(1);
            }
          }
          write_phone_response(0x25, M_FILE_CONTROL_PARAMETER_7FFF_5F3B_4F52 + 1, etu_length);
					
          break;
				}
            
        default:
				{
#if (IF_LOG_OUTPUT)
				  if (!IF_SOFTDEVICE_RUNNING)	
				  {	
            printf("selected file not found......\r\n");
				  }
#endif      
          write_bytes(2, no_file_bytes, etu_length, PIN_DATA_PHONE);
					
          break; 
				}     
      }      
    }
    else if (*(phone_command + 5) == 0x10)
    {
      write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
#if (IF_SOFTDEIVE_USED)
      if (IF_SOFTDEVICE_RUNNING) 
      {
        if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
        {
          write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
      }
      else
#endif				
      {
	      if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
	      {
		      write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
	        return(1);
	      }
      }

      SELECTED_FILE  = 0;
      SELECTED_FILE  |= (0x7f * 0x1000000);
      SELECTED_FILE  |= (0xff * 0x10000);
      
      selecet_fd[1] = 0x39;    
      write_bytes(2, selecet_fd, etu_length, PIN_DATA_PHONE);
      
#if (IF_SOFTDEIVE_USED)
      if (IF_SOFTDEVICE_RUNNING) 
      {
        if (read_bytes_ble(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
        {
          write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
      }
      else
#endif				
      {
        if (read_bytes_phone(5, READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
        {
	        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
          return(1);
        }
      }
      write_phone_response(0x39, M_FILE_CONTROL_PARAMETER_ADF + 1, etu_length);
    }
    else
    {
#if (IF_LOG_OUTPUT)
			if (!IF_SOFTDEVICE_RUNNING)	
			{	
        printf("selected file wrong parameters P1 and P2......\r\n");
			}
#endif
      
      write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);

      return(1);      
    }
  }
  else
  {
    write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
#if (IF_SOFTDEIVE_USED)
    if (IF_SOFTDEVICE_RUNNING) 
    {
      if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us))
      {
        write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
        return(1);
      }
    }
    else
#endif			
    {
	    if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
	    {
	      write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
	      return(1);
	    }
    }
				
    if (*(phone_command + 5) == 2)
    {
      if (*(READ_BYTE_UICC_TERMINAL + 1) == 0x2f && *(READ_BYTE_UICC_TERMINAL + 2) == 0xe2)
      {
        SELECTED_FILE  = 0x2fe2;
      }
      else if (*(READ_BYTE_UICC_TERMINAL + 1) == 0x3f && *(READ_BYTE_UICC_TERMINAL + 2) == 0x0)
      {
        SELECTED_FILE  = 0;
      }
      else if (*(READ_BYTE_UICC_TERMINAL + 1) == 0x7f && *(READ_BYTE_UICC_TERMINAL + 2) == 0xff)
      {
        SELECTED_FILE  = 0x7fff;
      }
      else if (*(READ_BYTE_UICC_TERMINAL + 1) == 0x7f && *(READ_BYTE_UICC_TERMINAL + 2) == 0x10)
      {
        SELECTED_FILE  = 0x7f10;
#if (IF_LOG_OUTPUT)
			  if (!IF_SOFTDEVICE_RUNNING)	
			  {	
          printf("--------------------------------------------- Hi, EF 7f10 selected ---------------------------------------------------\r\n");
			  }
#endif
      }
      else
      {
        if ((SELECTED_FILE == 0x7fff) || (SELECTED_FILE == 0x7fff5f3b) || (SELECTED_FILE == 0x7f10) ||
					(SELECTED_FILE == 0x7f105f3a) || (SELECTED_FILE == 0x7f105f3c))
        {
          SELECTED_FILE <<= 16;
        }
        SELECTED_FILE  &= MASK_SELECTED_FILE;
        SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x100);
        SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 2));
      }
    }
    else if (*(phone_command + 5) == 4)
    {
      SELECTED_FILE  = 0;
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x1000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 2) * 0x10000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 3) * 0x100);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 4));
    }
    else if (*(phone_command + 5) == 6)
    {
      SELECTED_FILE  = 0;
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 1) * 0x10000000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 2) * 0x100000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 3) * 0x1000000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 4) * 0x10000);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 5) * 0x100);
      SELECTED_FILE  |= (*(READ_BYTE_UICC_TERMINAL + 6));
    }
    else if (*(phone_command + 5) == 0x10)
    {
      SELECTED_FILE  = 0x7fff;
    }
    else
    {
#if (IF_LOG_OUTPUT)
			if (!IF_SOFTDEVICE_RUNNING)	
			{	
        printf("selected file wrong parameters P1 and P2......\r\n");
			}
#endif
      
      write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);      
      return(1);      
    }
#if (IF_LOG_OUTPUT)
		if (!IF_SOFTDEVICE_RUNNING)
		{
      printf_selected_file(SELECTED_FILE);
		}
#endif
      
    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
  }
  
  return(0);
}	

/********************************************************************************/
uint32_t phone_command_read_binary_2fe2(uint8_t *phone_command, uint32_t etu_length,
         uint8_t usim_card_no)
/*--------------------------------------------------------------------------------
| phone command 0xb0: USIM1 0x2FE2 read binary 
|
--------------------------------------------------------------------------------*/
{
	uint8_t *bytes_all_file;
	uint32_t usim_flash_address;
	
	if (FLAG_USIM_SWITCH)
	{
#if (IF_LOG_OUTPUT)    
    if (!IF_SOFTDEVICE_RUNNING)	
    {	
      printf("USIM switch, Read EF 0x2FE2 from USIM%d flash .......\r\n", usim_card_no);
    }
#endif
		
		return(0);
	}
	
	if (usim_card_no == 0)
	{
		bytes_all_file = USIM0_EF_DATA_RAM;
		usim_flash_address = USIM0_EF_FLASH_ADDR;
	}
	else
	{
		bytes_all_file = USIM1_EF_DATA_RAM;
		usim_flash_address = USIM1_EF_FLASH_ADDR;
	}
	
	/* read ICCID data 2FE2 from USIM card */
  if (sim_file_update(0x2fe2, bytes_all_file))
  {
	  return(1);
  }
	
	/* compare the ICCID read from USIM card and the ICCID saved in flalsh */
	if (byte_string_comparison(FILE_SIZE_2FE2, bytes_all_file + EF_2FE2_OFFSET, (uint8_t *)usim_flash_address + EF_2FE2_OFFSET))
	{
#if (IF_LOG_OUTPUT)    
	  if (!IF_SOFTDEVICE_RUNNING)	
		{	
		  printf("Different ICCID, Read EF 0x2FE2 data from USIM%d card ......\r\n", usim_card_no);
		  printf("ICCID in flash ");
      printf_log_rx(FILE_SIZE_2FE2, (uint8_t *)usim_flash_address + EF_2FE2_OFFSET);
	  }
#endif
		
		*(bytes_all_file + FLAG_2F05_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F07_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F08_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F09_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F31_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F78_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F7E_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F73_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F7B_OFFSET) = 0;
		*(bytes_all_file + FLAG_6FAD_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F5C_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F5B_OFFSET) = 0;
		*(bytes_all_file + FLAG_6FC4_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F56_OFFSET) = 0;
		*(bytes_all_file + FLAG_4F20_OFFSET) = 0;
		*(bytes_all_file + FLAG_4F52_OFFSET) = 0;
		*(bytes_all_file + FLAG_6F42_OFFSET) = 0;
		
		/* write the read EF data to flash */
		start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
		
  }
#if (IF_LOG_OUTPUT)    
  else
  {
    if (!IF_SOFTDEVICE_RUNNING)	
    {
      printf("Same ICCID, Read EF files data from USIM%d flash  ......\r\n", usim_card_no);
    }
  }
#endif
	
	return(0);	
}

/********************************************************************************/
uint32_t get_usim_file_position_index(uint32_t file_id, uint8_t *file_size, uint32_t *file_offset, uint32_t *flag_offset)
/*--------------------------------------------------------------------------------
| read binary from USIM card
|
--------------------------------------------------------------------------------*/
{
	switch (file_id)
	{
	  case 0x2fe2:
		{
		  *file_size = FILE_SIZE_2FE2;			
			*file_offset = EF_2FE2_OFFSET;
			*flag_offset = FLAG_2FE2_OFFSET;
			
			break;
		}		
		
	  case 0x2f05:
		{
		  *file_size = FILE_SIZE_2F05;			
			*file_offset = EF_2F05_OFFSET;
			*flag_offset = FLAG_2F05_OFFSET;
			
			break;
		}		
		
	  case 0x6f07:
		{
		  *file_size = FILE_SIZE_6F07;			
			*file_offset = EF_6F07_OFFSET;
			*flag_offset = FLAG_6F07_OFFSET;
			
			break;
		}		
		
	  case 0x6f08:
		{
		  *file_size = FILE_SIZE_6F08;			
			*file_offset = EF_6F08_OFFSET;
			*flag_offset = FLAG_6F08_OFFSET;
			
			break;
		}		
		
	  case 0x6f09:
		{
		  *file_size = FILE_SIZE_6F09;			
			*file_offset = EF_6F09_OFFSET;
			*flag_offset = FLAG_6F09_OFFSET;
			
			break;
		}		
		
	  case 0x6f31:
		{
		  *file_size = FILE_SIZE_6F31;			
			*file_offset = EF_6F31_OFFSET;
			*flag_offset = FLAG_6F31_OFFSET;
			
			break;
		}		
		
	  case 0x6f78:
		{
		  *file_size = FILE_SIZE_6F78;			
			*file_offset = EF_6F78_OFFSET;
			*flag_offset = FLAG_6F78_OFFSET;
			
			break;
		}		
		
	  case 0x6f7e:
		{
		  *file_size = FILE_SIZE_6F7E;			
			*file_offset = EF_6F7E_OFFSET;
			*flag_offset = FLAG_6F7E_OFFSET;
			
			break;
		}		
		
	  case 0x6f73:
		{
		  *file_size = FILE_SIZE_6F73;			
			*file_offset = EF_6F73_OFFSET;
			*flag_offset = FLAG_6F73_OFFSET;
			
			break;
		}		
		
	  case 0x6f7b:
		{
		  *file_size = FILE_SIZE_6F7B;			
			*file_offset = EF_6F7B_OFFSET;
			*flag_offset = FLAG_6F7B_OFFSET;
			
			break;
		}		
		
	  case 0x6fad:
		{
		  *file_size = FILE_SIZE_6FAD;			
			*file_offset = EF_6FAD_OFFSET;
			*flag_offset = FLAG_6FAD_OFFSET;
			
			break;
		}		
		
	  case 0x6f5c:
		{
		  *file_size = FILE_SIZE_6F5C;			
			*file_offset = EF_6F5C_OFFSET;
			*flag_offset = FLAG_6F5C_OFFSET;
			
			break;
		}		
		
	  case 0x6f5b:
		{
		  *file_size = FILE_SIZE_6F5B;			
			*file_offset = EF_6F5B_OFFSET;
			*flag_offset = FLAG_6F5B_OFFSET;
			
			break;
		}		
		
	  case 0x6fc4:
		{
		  *file_size = FILE_SIZE_6FC4;			
			*file_offset = EF_6FC4_OFFSET;
			*flag_offset = FLAG_6FC4_OFFSET;
			
			break;
		}		
		
	  case 0x6f56:
		{
		  *file_size = FILE_SIZE_6F56;			
			*file_offset = EF_6F56_OFFSET;
			*flag_offset = FLAG_6F56_OFFSET;
			
			break;
		}		
		
	  case 0x4f20:
		{
		  *file_size = FILE_SIZE_4F20;			
			*file_offset = EF_4F20_OFFSET;
			*flag_offset = FLAG_4F20_OFFSET;
			
			break;
		}		
		
	  case 0x4f52:
		{
		  *file_size = FILE_SIZE_4F52;			
			*file_offset = EF_4F52_OFFSET;
			*flag_offset = FLAG_4F52_OFFSET;
			
			break;
		}		
		
	  case 0x6f42:
		{
		  *file_size = FILE_SIZE_6F42;			
			*file_offset = EF_6F42_OFFSET;
			*flag_offset = FLAG_6F42_OFFSET;
			
			break;
		}		
		
		default:
		{
#if (IF_LOG_OUTPUT)    
      printf("error file ID for USIM card file datat read, file_id = 0x%X%X......\r\n", (uint8_t)(file_id >> 8), (uint8_t)(file_id));
#endif								
			
			break;
		}		
	}

	return(0);
}

/********************************************************************************/
uint32_t phone_command_read_binary_usim_card(uint32_t file_id, uint8_t usim_card_no)
/*--------------------------------------------------------------------------------
| read binary from USIM card
|
--------------------------------------------------------------------------------*/
{
	uint8_t *bytes_all_file;
	uint8_t file_size = 0;
	uint32_t file_offset = 0;
	uint32_t flag_offset = 0;
	
	if (FLAG_USIM_SWITCH)
	{
#if (IF_LOG_OUTPUT)    
		if (!IF_SOFTDEVICE_RUNNING)	
		{	
		  printf("USIM switch, Read EF 0x%X%X data from USIM%d flash .......\r\n", (uint8_t)(file_id >> 8), (uint8_t)(file_id), usim_card_no);
	  }
#endif
		
		return (0);
  }
	
	if (usim_card_no == 0)
	{
		bytes_all_file = USIM0_EF_DATA_RAM;
	}
	else
	{
		bytes_all_file = USIM1_EF_DATA_RAM;
	}
	
  get_usim_file_position_index(file_id, &file_size, &file_offset, &flag_offset);
	
	if (*(bytes_all_file + flag_offset) || IF_SOFTDEVICE_RUNNING)
	{
#if (IF_LOG_OUTPUT)    
	  if (!IF_SOFTDEVICE_RUNNING)	
	  {	
		  printf("Read EF 0x%X%X data from USIM%d flash .......\r\n", (uint8_t)(file_id >> 8), (uint8_t)(file_id), usim_card_no);
	  }
#endif
	}
	else
	{
	  if (sim_file_update(file_id, bytes_all_file))
		{
#if (IF_LOG_OUTPUT)    
			printf("Read EF 0x%X%X data from USIM card error, use the data in USIM%d flash ......\r\n", (uint8_t)(file_id >> 8), (uint8_t)(file_id), usim_card_no);
#endif								
    }
	  else
	  {
			/* write the read EF data to flash */
			start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);
			
#if (IF_LOG_OUTPUT)    
      printf("Read EF 0x%X%X data from USIM%d card ......\r\n", (uint8_t)(file_id >> 8), (uint8_t)(file_id), usim_card_no);
#endif								
		}
  }
	
	return(0);
}		

/********************************************************************************/
uint32_t phone_command_read_binary_file_data(uint8_t *phone_command, uint32_t etu_length, uint32_t file_id, 
         uint32_t tmp_length, uint32_t read_offset, uint8_t *confirm_bytes, uint8_t *status_bytes)
/*--------------------------------------------------------------------------------
| read binary file 6F78
|
--------------------------------------------------------------------------------*/
{
	uint8_t *bytes_all_file;
	uint8_t file_size = 0;
	uint32_t flag_offset = 0;
	uint32_t file_offset = 0;
	
  get_usim_file_position_index(file_id, &file_size, &file_offset, &flag_offset);
		
	/* wireless USIM */
	if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
	{
		if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
		{
			bytes_all_file = USIM0_EF_DATA_RAM;
	    /* get SIM EF data by wireless interface */
	    if (*(bytes_all_file + flag_offset) != 1)
	    {
	      phone_command_get_file_data(0xffff, bytes_all_file);
			}
		}
		else
		{
			bytes_all_file = USIM1_EF_DATA_RAM;
	    /* get SIM EF data by wireless interface */
	    if (*(bytes_all_file + flag_offset) != 1)
	    {
	      phone_command_get_file_data(0xffee, bytes_all_file);
			}
		}
	}
	/* local USIM */
	else
	{
		/* USIM0 is used by phone locally */
	  if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
		{
		  bytes_all_file = USIM0_EF_DATA_RAM;
		  if (*(bytes_all_file + flag_offset) != 1)
		  {
			  phone_command_read_binary_usim_card(file_id, 0);
			}
		}
		/* USIM1 is used by phone locally */
		else
		{
			bytes_all_file = USIM1_EF_DATA_RAM;
		  if (*(bytes_all_file + flag_offset) != 1)
		  {
				phone_command_read_binary_usim_card(file_id, 1);
			}
		}
  }

	write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
	write_bytes(tmp_length, bytes_all_file + file_offset + read_offset, etu_length, PIN_DATA_PHONE); 
	write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
	
	return(0);
}

/********************************************************************************/
uint32_t phone_command_read_binary_0xb0(uint8_t *phone_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command 0xb0: read binary for necessary EFs
|
--------------------------------------------------------------------------------*/
{
  uint32_t read_offset = 0;
  uint32_t tmp_length;
  uint8_t tmp_bytes[2] = {0x69, 0x86};
  uint8_t confirm_bytes[1] = {0xb0};
  uint8_t status_bytes[2] = {0x90, 0x0};
	
  usim_initial_selected_file_check( );
     
	/* USAT bytes feedback */	   					
  if (USAT_BYTE_LENGTH_BACK)
  {
    status_bytes[0] = 0x91;
    status_bytes[1] = USAT_BYTE_LENGTH_BACK;
  }
	
  if ((*(phone_command + 5)) == 0x00)
    tmp_length = 256;
  else
    tmp_length = (*(phone_command + 5));
	
  /* start sim clock signal */
  clock_sim_start_4m(PIN_CLOCK_SIM);
		
  if ((*(phone_command + 3)) & 0x80)
  {
    read_offset = *(phone_command + 4);
	
    switch ((*(phone_command + 3)) & 0x1f)
    {
			/* 6FAD */
	    case 0x03:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6fad, tmp_length, read_offset, confirm_bytes, status_bytes);
        break;
			}
		
		  /* 6F38 */
      case 0x04:
			{
			  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
			  write_bytes(tmp_length, UPDATED_BYTES_7FFF_6F38 + read_offset + 1, etu_length, PIN_DATA_PHONE); 
			  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);

        break;
			}

      /* 6F56 */
      case 0x05:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f56, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F78 */
      case 0x06:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f78, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F07 */
  	  case 0x07:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f07, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}
		  
      /* 6F08 */
      case 0x08:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f08, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F09 */
      case 0x09:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f09, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F7E */
      case 0x0b:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f7e, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F73 */
      case 0x0c:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f73, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}
		
      /* 6F7B */
      case 0x0d:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f7b, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F5B */
      case 0x0f:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f5b, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}
		
      /* 6F5C */
      case 0x10:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f5c, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}

      /* 6F31 */
      case 0x12:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, 0x6f31, tmp_length, read_offset, confirm_bytes, status_bytes);
				
				break;
			}
		
      default:
			{
#if (IF_LOG_OUTPUT)
			  if (!IF_SOFTDEVICE_RUNNING)	
			  {				
	        printf("--------------------------------------------------------------------\r\n");  
          printf("Wrong SFI in phone_command_read_binary_0xb0, error...\r\n");
          printf("SFI = %x, \r\n", (*(phone_command + 3)) & 0x1f);
	        printf("--------------------------------------------------------------------\r\n");    
				}
#endif
        /* write SIM response to phone, Command not allowed, no EF selected */    
        write_bytes(2, tmp_bytes, etu_length, PIN_DATA_PHONE);
      
#if (PHONE == 0)					
        /* stop SIM clock */
        clock_sim_stop_4m(PIN_CLOCK_SIM);		
#endif	
        return(1);	
			}	
	  }
  }
  else
  {
    read_offset = (*(phone_command + 3)) * 0x100 + (*(phone_command + 4));
	
    switch (SELECTED_FILE)
    {
      case 0x2fe2:
			{
        if (*((uint8_t *)ICCID_2FE2_DATA_MODE_FLASH_ADDR) == 0)
				{
		      /* get SIM EF data by wireless interface */
			    if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				  {
					  /* wireless USIM0 */
				    if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
					  {
		          if (*(USIM0_EF_DATA_RAM + FLAG_2FE2_OFFSET) == 0)
		          {
	              phone_command_get_file_data(0xffff, USIM0_EF_DATA_RAM);
						  }
					    write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
					    write_bytes(tmp_length, USIM0_EF_DATA_RAM + EF_2FE2_OFFSET + read_offset, etu_length, PIN_DATA_PHONE); 
					    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
					  }
					  /* wireless USIM1 */
					  else
					  {
		          if (*(USIM1_EF_DATA_RAM + FLAG_2FE2_OFFSET) == 0)
		          {
	              phone_command_get_file_data(0xffee, USIM1_EF_DATA_RAM);
						  }
					    write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
					    write_bytes(tmp_length, USIM1_EF_DATA_RAM + EF_2FE2_OFFSET + read_offset, etu_length, PIN_DATA_PHONE); 
					    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
					  }
				  }
				  else
				  {
					  /* USIM0 is used by phone locally */
					  if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
					  {
			        if (*(USIM0_EF_DATA_RAM + FLAG_2FE2_OFFSET) == 0)
			        {
							  phone_command_read_binary_2fe2(phone_command, etu_length, 0);
						  }
						
					    write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
					    write_bytes(tmp_length, USIM0_EF_DATA_RAM + EF_2FE2_OFFSET + read_offset, etu_length, PIN_DATA_PHONE); 
					    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
					  }
					  /* USIM1 is used by phone locally */
					  else
					  {
			        if (*(USIM1_EF_DATA_RAM + FLAG_2FE2_OFFSET) == 0)
			        {
							  phone_command_read_binary_2fe2(phone_command, etu_length, 1);
						  }
						
					    write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
					    write_bytes(tmp_length, USIM1_EF_DATA_RAM + EF_2FE2_OFFSET + read_offset, etu_length, PIN_DATA_PHONE); 
					    write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
					  }
          }
				}
        else if (*((uint8_t *)ICCID_2FE2_DATA_MODE_FLASH_ADDR) == 1)
				{
		      /* get SIM EF data by wireless interface */
			    if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0)
				  {
		        if (*(USIM0_EF_DATA_RAM + FLAG_2FE2_OFFSET) == 0)
		        {
	            phone_command_get_file_data(0xffff, USIM0_EF_DATA_RAM);
						}
					  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
					  write_bytes(tmp_length, USIM0_EF_DATA_RAM + EF_2FE2_OFFSET + read_offset, etu_length, PIN_DATA_PHONE); 
					  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
				  }
					/* read USIM EF data from USIM card */
				  else
				  {
			      if (*(USIM0_EF_DATA_RAM + FLAG_2FE2_OFFSET) == 0)
			      {
							phone_command_read_binary_2fe2(phone_command, etu_length, 0);
						}
						
					  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
					  write_bytes(tmp_length, USIM0_EF_DATA_RAM + EF_2FE2_OFFSET + read_offset, etu_length, PIN_DATA_PHONE); 
					  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
          }
				}
        else if (*((uint8_t *)ICCID_2FE2_DATA_MODE_FLASH_ADDR) == 2)
				{					 
				  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
				  write_bytes(tmp_length, ICCID_2FE2_DATA, etu_length, PIN_DATA_PHONE); 
				  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
				}
        break;
      }
			
      case 0x7fff6f38:
			{
			  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
			  write_bytes(tmp_length, UPDATED_BYTES_7FFF_6F38 + read_offset + 1, etu_length, PIN_DATA_PHONE); 
			  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);

        break;
			}

      case 0x7fff6f43:
			{
			  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
			  write_bytes(tmp_length, UPDATED_BYTES_7FFF_6F43 + read_offset + 1, etu_length, PIN_DATA_PHONE); 
			  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
				
        break;
			}
			
			case 0x7fff6f46:
			{
			  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
			  write_bytes(tmp_length, UPDATED_BYTES_7FFF_6F46 + read_offset + 1, etu_length, PIN_DATA_PHONE);
			  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);

			  break;
			}

      case 0x2f05:
      case 0x7fff6fad:
      case 0x7fff6f07: 
      case 0x7fff6f78: 
      case 0x7fff6f7e: 
      case 0x7fff6f73: 
      case 0x7fff6f08: 
      case 0x7fff6f09: 
      case 0x7fff6f5b: 
      case 0x7fff6f5c: 
      case 0x7fff6fc4: 
      case 0x7fff6f31: 
      case 0x7fff6f7b: 
      case 0x7fff6f56: 
      case 0x7fff5f3b4f20:
      case 0x7fff5f3b4f52:
			{
				phone_command_read_binary_file_data(phone_command, etu_length, (SELECTED_FILE & 0xFFFF), tmp_length, read_offset, confirm_bytes, status_bytes);
				
        break;
			}

      default:
			{
#if (IF_LOG_OUTPUT)
			  if (!IF_SOFTDEVICE_RUNNING)	
			  {	
          printf("Wrong file ID in phone_command_read_binary_0xb0, error...\r\n");
          printf_selected_file(SELECTED_FILE);
			  }
#endif
        /* write SIM response to phone, Command not allowed, no EF selected */    
        write_bytes(2, tmp_bytes, etu_length, PIN_DATA_PHONE);
      
        /* stop SIM clock */
        clock_sim_stop_4m(PIN_CLOCK_SIM);		

        return(1);
			}
    }
  }

  /* stop SIM clock */
  clock_sim_stop_4m(PIN_CLOCK_SIM);	
  
  return(0);
}

#if (IF_USIM_BINARY_UPDATE)	
/********************************************************************************/
uint32_t sim_command_update_binary_get_offset_0xd6(uint8_t *phone_command, uint32_t update_file_id)
/*--------------------------------------------------------------------------------
| get the update binary USIM command offset data
|
--------------------------------------------------------------------------------*/
{
	switch (update_file_id)
	{
		case 0x6f08:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F08 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F08;
			break;
		}
		
		case 0x6f09:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F09 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F09;
			break;
		}
		
		case 0x6f5b:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F5B >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F5B;
			break;
		}
		
		case 0x4f52:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_4F52 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_4F52;
			break;
		}
		
		case 0x4f20:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_4F20 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_4F20;
			break;
		}
		
		case 0x6f73:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F73 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F73;
			break;
		}
		
		case 0x6f7e:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F7E >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F7E;
			break;
		}
		
		case 0x6f7b:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F7B >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F7B;
			break;
		}
		
		case 0x6f31:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F31 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F31;
			break;
		}
		
		case 0x6f78:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F78 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F78;
			break;
		}
		
		case 0x6fad:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6FAD >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6FAD;
			break;
		}
		
		case 0x6f5c:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F5C >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F5C;
			break;
		}
		
		case 0x6fc4:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6FC4 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6FC4;
			break;
		}
		
		case 0x6f56:
		{
			*(phone_command + 2) = (uint8_t)(UPDATE_BNARY_OFFSET_6F56 >> 8);
		  *(phone_command + 3) = (uint8_t)UPDATE_BNARY_OFFSET_6F56;
			break;
		}
		
		default:
		{
			*(phone_command + 2) = 0;
		  *(phone_command + 3) = 0;
			break;
		}
	}
	
	return(0);	
}
		
/********************************************************************************/
void stop_sim_clock_timer1(void)
/*--------------------------------------------------------------------------------
| SIM command 0xD6 to update EF data
|
--------------------------------------------------------------------------------*/
{
	/* stop SIM clock */
	clock_sim_stop_4m(PIN_CLOCK_SIM);
  /* stop ETU timer tick */
  NRF_TIMER1->TASKS_CLEAR = 1;
  NRF_TIMER1->TASKS_SHUTDOWN = 1;	
}
		
/********************************************************************************/
uint32_t sim_command_usim_card_update_binary_0xd6(uint32_t update_file_id)
/*--------------------------------------------------------------------------------
| SIM command 0xD6 to update EF data
|
--------------------------------------------------------------------------------*/
{
	uint8_t *bytes_all_file;
	uint8_t file_size = 0;
	uint32_t file_offset = 0;
	uint32_t flag_offset = 0;
  uint8_t phone_command[5] = {0x0, 0xd6, 0x0, 0x0, 0x0};
	uint32_t update_offset;
	
  /* USIM0 */
  if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
  {
    set_default_usim0_etu( );		
		bytes_all_file = USIM0_EF_DATA_RAM;
	}
#if (PIN_VCC_SIM1 != PIN_NULL)					
  /* USIM1 */
	else
	{
    set_default_usim1_etu( );		
		bytes_all_file = USIM1_EF_DATA_RAM;
	}
#endif
	
  get_usim_file_position_index(update_file_id, &file_size, &file_offset, &flag_offset);
	
  sim_command_update_binary_get_offset_0xd6(phone_command, update_file_id);
  /* update binary data length */
	*(phone_command + 4) = file_size;
  
#if (IF_LOG_OUTPUT)    
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(5, phone_command);
	}
#endif
  
  if ((*(phone_command + 2)) & 0x80)
  {
    update_offset = *(phone_command + 3);
  }
  else
  {
    update_offset = ((*(phone_command + 2)) << 8) | (*(phone_command + 3));
  }
	
  NRF_TIMER1->TASKS_START = 1;
 	/* start sim clock signal */
	clock_sim_start_4m(PIN_CLOCK_SIM);
	
	/* USIM command select */
	switch (update_file_id)
	{
	  case 0x4f20:
		{
			if (((SELECTED_FILE_SIM != 0x7fff5f3b) && ((SELECTED_FILE_SIM >> 16) != 0x7fff5f3b)))
			{
			  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);	
			  if (sim_file_7fff_selected_check( ))
			  {
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
						
				  return(1);
			  }
		    if (sim_command_select(0x5f3b, ETU_TICKS_SIM))
		    {
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
		
		      return(1);
		    }
			}
		  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);	
	    if (sim_command_select(0x4f20, ETU_TICKS_SIM))
	    {
				/* stop SIM clock signal and timer 1*/
        stop_sim_clock_timer1( );
		
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
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
		
			    return(1);
		    }
	      if (sim_command_select(0x5f3b, ETU_TICKS_SIM))
	      {
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
		
	        return(1);
	      }
		  }

	    if (sim_command_select(0x4f52, ETU_TICKS_SIM))
	    {
				/* stop SIM clock signal and timer 1*/
        stop_sim_clock_timer1( );
		
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
		  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);	
	    if (sim_file_7fff_selected_check( ))
	    {
				/* stop SIM clock signal and timer 1*/
        stop_sim_clock_timer1( );
		
	      return(1);
	    }
		  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);	
	    if (sim_command_select(update_file_id, ETU_TICKS_SIM))
	    {
				/* stop SIM clock signal and timer 1*/
        stop_sim_clock_timer1( );
		
	      return(1);
	    }

		  SELECTED_FILE_SIM = 0x7fff0000;
		  SELECTED_FILE_SIM |= (update_file_id & 0xffff);
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

	/* write USIM command to updat binary */
  write_bytes(5, phone_command, ETU_TICKS_SIM, PIN_DATA_SIM);
	
	/* read the USIM feedback confirm byte command */
  if (read_bytes_sim(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
  {
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );
		
    return(1);
  }
	
  write_bytes(*(phone_command + 4), bytes_all_file + file_offset + update_offset, ETU_TICKS_SIM, PIN_DATA_SIM);
	
	if (read_bytes_sim(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, 1))
	{
		/* stop SIM clock signal and timer 1*/
    stop_sim_clock_timer1( );
		
	  return(1);
	}
	
	/* stop SIM clock signal and timer 1*/
  stop_sim_clock_timer1( );
	
  return(0);
}

/********************************************************************************/
uint32_t sim_command_usim_card_update_binary_0xd6_ble(uint32_t update_file_id, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| SIM command 0xD6 to update EF data
|
--------------------------------------------------------------------------------*/
{
  uint8_t phone_command[5] = {0x0, 0xd6, 0x0, 0x0, 0x0};
	uint8_t *bytes_all_file;
	uint8_t file_size = 0;
	uint32_t file_offset = 0;
	uint32_t flag_offset = 0;
  uint32_t tmp_rtc2;
	
  sim_command_update_binary_get_offset_0xd6(phone_command, update_file_id);	
	
  /* USIM0 */
  if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
  {
    set_default_usim0_etu( );		
		bytes_all_file = USIM0_EF_DATA_RAM;
	}
#if (PIN_VCC_SIM1 != PIN_NULL)					
  /* USIM1 */
	else
	{
    set_default_usim1_etu( );		
		bytes_all_file = USIM1_EF_DATA_RAM;
	}
#endif
		
  NRF_TIMER1->TASKS_START = 1;
 	/* start sim clock signal */
	clock_sim_start_4m(PIN_CLOCK_SIM);
			
	/* stage 0 */	
	if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage0)
  {
		if ((update_file_id == 0x4f20) || (update_file_id == 0x4f52))
		{
		  if (((SELECTED_FILE_SIM != 0x7fff5f3b) && ((SELECTED_FILE_SIM >> 16) != 0x7fff5f3b)))
		  {
		    rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);	
		    if (sim_file_7fff_selected_check( ))
		    {
			    /* set SWI3_EGU3 for SIM card state reset, EF selection 7fff */
			    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
					FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
		
			    return(1);
		    }
	      if (sim_command_select(0x5f3b, ETU_TICKS_SIM))
	      {
			    /* set SWI3_EGU3 for SIM card state reset, EF selection 7fff */
			    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
					FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
		
	        return(2);
	      }
			}
		}
		else
		{
			if ((SELECTED_FILE_SIM != 0x7fff) && ((SELECTED_FILE_SIM >> 16) != 0x7fff))
			{
		    rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);	
		    if (sim_file_7fff_selected_check( ))
		    {
			    /* set SWI3_EGU3 for SIM card state reset, EF selection 7fff */
			    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
					FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			
					/* stop SIM clock signal and timer 1*/
          stop_sim_clock_timer1( );
		
			    return(3);
		    }
			}			
		}
  }		
	
	/* stage 1 */	
	else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage1)
  {
    if(sim_command_select(update_file_id, ETU_TICKS_SIM))
		{
	    /* set SWI3_EGU3 for SIM card state reset, EF selection 7fff */
	    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
			FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			
			/* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
		
			return(4);
		}
	
		if ((update_file_id == 0x4f20) || (update_file_id == 0x4f52))
		{
      SELECTED_FILE_SIM = 0x7fff5f3b0000 | update_file_id;
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
		else
		{
      SELECTED_FILE_SIM = 0x7fff0000 | update_file_id;
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
  }		
	
	/* stage 2 */	
	else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage2)
	{
	  get_usim_file_position_index(update_file_id, &file_size, &file_offset, &flag_offset);
  
	  /* update binary data length */
		*(phone_command + 4) = file_size;
 
	  /* write USIM command to updat binary */
    write_bytes(5, phone_command, ETU_TICKS_SIM, PIN_DATA_SIM);
	
		tmp_rtc2 = NRF_RTC2->COUNTER; 
	  /* read the USIM feedback confirm byte command */
    if (read_bytes_ble(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, start_time_us, time_length_us))
    {
			/* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
		
      return(NRF_RTC2->COUNTER - tmp_rtc2);
    }
		
	  /* write the updta binary to USIM */
    write_bytes(file_size, bytes_all_file + file_offset, ETU_TICKS_SIM, PIN_DATA_SIM);
		 
		tmp_rtc2 = NRF_RTC2->COUNTER; 
	  if (read_bytes_ble(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, ETU_TICKS_SIM, start_time_us, time_length_us))
	  {
			/* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
		
	    return(NRF_RTC2->COUNTER - tmp_rtc2);
	  }			
	}
	
	/* stage 3 */		
	else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage3)
	{
    if (sim_command_select(0x3f00, ETU_TICKS_SIM))
		{
	    /* set SWI3_EGU3 for SIM card state reset, EF selection 7fff */
	    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
			FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			
			/* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
		
			return(5);
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
	
	/* stage 4 */		
	else if (SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_STAGE == SIM_COMMAND_UPDATE_BINARY_0XD6_BLE_Stage4)
	{
		if (sim_command_select_adf(ETU_TICKS_SIM))
		{
	    /* set SWI3_EGU3 for SIM card state reset, EF selection 7fff */
	    sd_nvic_SetPendingIRQ(SWI3_EGU3_IRQn);
			FLAG_SWI3_EGU3_IRQn = SWI3_EGU3_ISR_SIM_CARD_STATE_RESET;
			
			/* stop SIM clock signal and timer 1*/
      stop_sim_clock_timer1( );
		
			return(6);
		}
		
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
	}	
	
	else
	{
		/* do nothing */
	}	
	
	/* stop SIM clock signal and timer1 */
  stop_sim_clock_timer1( );
		
  return(0);	
}
#endif

/********************************************************************************/
uint32_t sim_command_update_binary_all_files(uint32_t update_file_id, uint32_t update_offset, 
         uint32_t update_length, uint8_t *file_data_source)
/*--------------------------------------------------------------------------------
| update binary of USIM files 
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;	
	uint8_t *bytes_all_file;
	uint8_t file_size = 0;
	uint32_t file_offset = 0;
	uint32_t flag_offset = 0;
	
	/* local USIM is used */
	if ((PHONE_LOCAL_USED_USIM & 0xF0) == 0)
	{
	  /* USIM0 is used by phone locally */
	  if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
	  {
		  bytes_all_file = USIM0_EF_DATA_RAM;
	  }
	  /* USIM1 is used by phone locally */
	  else
	  {
		  bytes_all_file = USIM1_EF_DATA_RAM;
	  }
	}
	/* wireless USIM is used */
	else
	{
		/* current used wireless USIM0 */
		if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
		{						
		  bytes_all_file = USIM0_EF_DATA_RAM;
		}
		/* current used wireless USIM1 */
		else
		{		
		  bytes_all_file = USIM1_EF_DATA_RAM;
		}				
	}
	
  get_usim_file_position_index(update_file_id, &file_size, &file_offset, &flag_offset);
	
	*(bytes_all_file + flag_offset) = 1;
	for (i=0; i<update_length; i++)
	{
	  *(bytes_all_file + file_offset + update_offset + i) = *(file_data_source + i);
	}				
	
	return(0);
}

/********************************************************************************/
uint32_t phone_command_update_binary_0xd6_file(uint8_t *phone_command, uint8_t update_offset,
         uint8_t update_length)
/*--------------------------------------------------------------------------------
| phone command 0xd6: update binary for necessary EFs
|
--------------------------------------------------------------------------------*/
{
#if (IF_USIM_BINARY_UPDATE)		
	uint8_t task_queue_offset_pos = 0;
	
  switch (SELECTED_FILE)
  {
		case 0x7fff6f08:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F08_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F08 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f09:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F09_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F09 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f5b:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F5B_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F5B = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff5f3b4f52:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_4F52_OFFSET_POS;
      UPDATE_BNARY_OFFSET_4F52 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff5f3b4f20:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_4F20_OFFSET_POS;
      UPDATE_BNARY_OFFSET_4F20 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f73:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F73_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F73 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f7e:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F7E_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F7E = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f7b:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F7B_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F7B = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f31:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F31_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F31 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f78:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F78_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F78 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6fad:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6FAD_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6FAD = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f5c:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F5C_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F5C = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6fc4:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6FC4_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6FC4 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
		
		case 0x7fff6f56:
		{
			task_queue_offset_pos = SIM_FILE_UPDATE_BINARY_6F56_OFFSET_POS;
      UPDATE_BNARY_OFFSET_6F56 = (*(phone_command + 3)) << 8 | (*(phone_command + 4));
			break;
		}
	}
#endif	
	
	sim_command_update_binary_all_files(SELECTED_FILE & 0xFFFF, update_offset, update_length, READ_BYTE_UICC_TERMINAL + 1);

#if (IF_USIM_BINARY_UPDATE)		
	SIM_FILE_UPDATE_BINARY_TASK_QUEUE |= (0x1 << task_queue_offset_pos);
	
#if (IF_LOG_OUTPUT)
	if (IF_SOFTDEVICE_RUNNING == 0)
	{
		if ((PHONE_LOCAL_USED_USIM & 0xF0) == 0) /* local USIM is used */
		{
		  printf ("======================================= Hi,  USIM%d EF_ALL %X%X update binary task queue   =======================================\r\n", 
					     (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 0), (uint8_t)(SELECTED_FILE >> 8), (uint8_t)SELECTED_FILE);
		}
		else /* wireless USIM is used */
		{
		  printf ("======================================= Hi,  USIM%d EF_ALL %X%X update binary task queue   =======================================\r\n", 
					     ((PHONE_WIRELESS_USED_USIM & 1) == 1), (uint8_t)(SELECTED_FILE >> 8), (uint8_t)SELECTED_FILE);
		}
	}
#endif
#endif
	
	return(0);	
}
		  
/********************************************************************************/
uint32_t phone_command_update_binary_0xd6(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone command 0xd6: update binary for necessary EFs
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t update_offset;
  uint32_t update_length;
  uint8_t tmp_bytes[2] = {0x69, 0x86};
  uint8_t confirm_bytes[1] = {0xd6};
  uint8_t status_bytes[2] = {0x90, 0x0};

  usim_initial_selected_file_check( );

  if (USAT_BYTE_LENGTH_BACK)
  {
    status_bytes[0] = 0x91;
    status_bytes[1] = USAT_BYTE_LENGTH_BACK;
  }
  
  if ((*(phone_command + 5)) == 0x00)
    update_length = 256;
  else
    update_length = (*(phone_command + 5));
  
#if (IF_LOG_OUTPUT)    
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(5, phone_command + 1);
	}
#endif
  
  if ((*(phone_command + 3)) & 0x80)
  {
    update_offset = *(phone_command + 4);
  }
  else
  {
    update_offset = ((*(phone_command + 3)) << 8) | (*(phone_command + 4));
  }
  
  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
  
#if (IF_SOFTDEIVE_USED)
  if (IF_SOFTDEVICE_RUNNING) 
	{
    if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us - 200))
    {
		  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
      return(1);
    }
	}
	else
#endif		
	{
    if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
    {
		  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);
      return(1);
    }
	}
  
  switch (SELECTED_FILE)
  {
    case 0x7fff6f08:
		case 0x7fff6f09:
    case 0x7fff6f5b:
    case 0x7fff5f3b4f52:
    case 0x7fff5f3b4f20:
    case 0x7fff6f73:
    case 0x7fff6f7e:
    case 0x7fff6f7b:
    case 0x7fff6f31:
    case 0x7fff6f78:
    case 0x7fff6fad:
    case 0x7fff6f5c:
    case 0x7fff6fc4:
    case 0x7fff6f56:
		{
			phone_command_update_binary_0xd6_file(phone_command, update_offset, update_length);
      break;
    } 
		  
    case 0x7fff6f43:
		{
      for (i=0; i<update_length; i++)
      {
        UPDATED_BYTES_7FFF_6F43[update_offset + i + 1] = *(READ_BYTE_UICC_TERMINAL + 1 + i);
      }
      break;
		}
      
    default:
		{
#if (IF_LOG_OUTPUT)
		  if (!IF_SOFTDEVICE_RUNNING)	
		  {
        printf("update binary file in phone_command_update_binary_0xd6 error......\r\n");
        printf_selected_file(SELECTED_FILE);
		  }
#endif
      /* write SIM response to phone, Command not allowed, no EF selected */    
      write_bytes(2, tmp_bytes, etu_length, PIN_DATA_PHONE);
    
      return(1);
		}
  }
  
  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);

  return(0);  
}  

/********************************************************************************/
uint32_t phone_command_read_record_0xb2(uint8_t *phone_command, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| phone command 0xb2: read record for necessary EFs
|
--------------------------------------------------------------------------------*/
{
  uint8_t *bytes_read;
  uint32_t read_offset;
  uint32_t i;
  uint32_t tmp_length;
  uint8_t tmp_bytes[2] = {0x6a, 0x83};
  uint8_t confirm_bytes[1] = {0xb2};
  uint8_t status_bytes[2] = {0x90, 0x0};	
	
  usim_initial_selected_file_check( );
	
  if (USAT_BYTE_LENGTH_BACK)
  {
    status_bytes[0] = 0x91;
    status_bytes[1] = USAT_BYTE_LENGTH_BACK;
  }
  
  if ((*(phone_command + 5)) == 0x00)
    tmp_length = 256;
  else
    tmp_length = (*(phone_command + 5));
    
#if (IF_LOG_OUTPUT)    
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(5, phone_command + 1);
	}
#endif
  
  bytes_read = (uint8_t *)malloc(*(phone_command + 5) * sizeof(uint8_t));
  if (bytes_read == NULL)
  {
#if (IF_LOG_OUTPUT)    
		if (!IF_SOFTDEVICE_RUNNING)	
		{	
      printf("memory allocation error...\r\n");
		}
#endif
    
    return(1);
  }
  
  if ((*(phone_command + 3)))
  {
    read_offset = (*(phone_command + 3) - 1) * (*(phone_command + 5));
  }
  else
  {
    /* to be added */
    read_offset = 0;
  }
  
  switch (SELECTED_FILE)
  {
    case 0x2f00:
		{
      for (i=0; i<tmp_length; i++)
      {
        *(bytes_read + i) = UPDATED_BYTES_2F00[read_offset + i + 1];
      }
      break;
		}
            
    case 0x7fff6f06:
		{
      for (i=0; i<tmp_length; i++)
      {
        *(bytes_read + i) = UPDATED_BYTES_7FFF_6F06[(read_offset % 0x2c) + i];
      }
      break;
		}
		
    case 0x7fff6fb7:
    case 0x7fff6f49:
    case 0x7fff6f4b:
    case 0x7fff6f3c:
		{
			/* BYTES_7FFF_6FB7[0x14] = {0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff;
			   BYTES_7FFF_6F49[0x18] = {0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff}; 
			   BYTES_7FFF_6F4B[0xd] = {0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff};
			   BYTES_7FFF_6F3C[0xb0] = {0x0,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  
			   0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff}; 
			*/
      for (i=0; i<tmp_length; i++)
      {
        *(bytes_read + i) = 0xff;
      }
      break;
		}
      
    case 0x7fff6f42:
		{
			if ((PHONE_LOCAL_USED_USIM & 0xF0) != 0) /* wireless USIM EF data */
		  {
	      /* get SIM EF data by wireless interface */
			  if ((PHONE_WIRELESS_USED_USIM & 1) == 0)
				{
					/* wireless USIM0 */
	        if (*(USIM0_EF_DATA_RAM + FLAG_6F42_OFFSET) == 0)
					{
            phone_command_get_file_data(0xffff, USIM0_EF_DATA_RAM);
					}
					
		      for (i=0; i<tmp_length; i++)
		      {
		        *(bytes_read + i) = *(USIM0_EF_DATA_RAM + EF_6F42_OFFSET + read_offset + i);
		      }
				}
				else
				{
					/* wireless USIM1 */
	        if (*(USIM1_EF_DATA_RAM + FLAG_6F42_OFFSET) == 0)
					{
            phone_command_get_file_data(0xffee, USIM1_EF_DATA_RAM);
					}
					
		      for (i=0; i<tmp_length; i++)
		      {
		        *(bytes_read + i) = *(USIM1_EF_DATA_RAM + EF_6F42_OFFSET + read_offset + i);
		      }
			  }
			}
			else /* local USIM EF data */
			{
        clock_sim_start_4m(PIN_CLOCK_SIM);

			  /* USIM0 is used by phone locally */
				if (((PHONE_LOCAL_USED_USIM >> 0) & 1) == 1)
				{
	        if (*(USIM0_EF_DATA_RAM + FLAG_6F42_OFFSET) == 0)
					{
						/* update the EF data from USIM card */
						phone_command_read_binary_usim_card(0x6f42, 0);
					}
					
		      for (i=0; i<tmp_length; i++)
		      {
		        *(bytes_read + i) = *(USIM0_EF_DATA_RAM + EF_6F42_OFFSET + read_offset + i);
		      }
				}
				/* USIM1 is used by phone locally */
				else
				{
	        if (*(USIM1_EF_DATA_RAM + FLAG_6F42_OFFSET) == 0)
					{
						/* update the EF data from USIM card */
						phone_command_read_binary_usim_card(0x6f42, 1);
					}
					
		      for (i=0; i<tmp_length; i++)
		      {
		        *(bytes_read + i) = *(USIM1_EF_DATA_RAM + EF_6F42_OFFSET + read_offset + i);
		      }
		    } 
				
        clock_sim_stop_4m(PIN_CLOCK_SIM);	
      }

      break;
		}

    default:
#if (IF_LOG_OUTPUT)
		if (!IF_SOFTDEVICE_RUNNING)	
		{	
      printf("++++++++++++++++++++++ read record file in phone_command_read_record_0xb2 error......\r\n");
      printf_selected_file(SELECTED_FILE);
		}
#endif
      /* write SIM response to phone, Command not allowed, no EF selected */    
      write_bytes(2, tmp_bytes, etu_length, PIN_DATA_PHONE);
        
      return(1);
  }
  
  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);
  
  write_bytes(tmp_length, bytes_read, etu_length, PIN_DATA_PHONE);
  
  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE);    
  
  free(bytes_read);
  
  return(0);
}

/********************************************************************************/
uint32_t phone_command_update_record_0xdc(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| phone command 0xdc: update record
|
--------------------------------------------------------------------------------*/
{
  uint8_t status_bytes[2] = {0x90, 0x0};
  uint8_t confirm_bytes[1] = {0xdc};
  uint8_t wrong_parameter_bytes[2] = {0x6a, 0x87};
	uint16_t phone_dial_command_type = 0;
	 
  write_bytes(1, confirm_bytes, etu_length, PIN_DATA_PHONE);     

#if (IF_SOFTDEIVE_USED)
  if (IF_SOFTDEVICE_RUNNING) 
	{
    if (read_bytes_ble(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length, start_time_us, time_length_us - 200))
    {
		  write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
      return(1);
    }
	}
	else
#endif		
	{
    if (read_bytes_phone(*(phone_command + 5), READ_BYTE_UICC_TERMINAL, PIN_DATA_PHONE, etu_length))
    {
	    write_bytes(2, wrong_parameter_bytes, etu_length, PIN_DATA_PHONE);
      return(1);
		}
  }		
			
	/* phone dial command type, update SMS number command, "*5005*7672*xxxx#" */
  if (((*(READ_BYTE_UICC_TERMINAL + (*(phone_command + 5)) - 14)) == 0x3) && ((*(READ_BYTE_UICC_TERMINAL + (*(phone_command + 5)) - 13)) == 0x81))
	{
	  phone_dial_command_type = ((uint8_t)(((*(READ_BYTE_UICC_TERMINAL + (*(phone_command + 5)) - 12)) >> 4) | ((*(READ_BYTE_UICC_TERMINAL + (*(phone_command + 5)) - 12)) << 4))) * 0x100 + 
			(uint8_t)(((*(READ_BYTE_UICC_TERMINAL + (*(phone_command + 5)) - 11)) >> 4) | ((*(READ_BYTE_UICC_TERMINAL + (*(phone_command + 5)) - 11)) << 4));

#if (IF_LOG_OUTPUT)   
		if (!IF_SOFTDEVICE_RUNNING)
		{
		  printf("********************************* phone_dial_command_type = 0x%2x, %2x, *****************************\r\n", (uint8_t)(phone_dial_command_type >> 8), (uint8_t)(phone_dial_command_type));
		}
#endif		
		
		if (phone_dial_command_type == 0x0)
		{
		  FETCH_COMMAND_TYPE = FETCH_COMMAND_TYPE_INITIAL_USAT_MENU_SETUP;
		  USAT_BYTE_LENGTH_BACK = USAT_BYTE_LENGTH_BACK_INIT;
			status_bytes[0] = 0x91;
			status_bytes[1] = USAT_BYTE_LENGTH_BACK_INIT;
		}
		else if ((phone_dial_command_type >= 0x0001) && (phone_dial_command_type <= 0x0006))
		{
			phone_usat_menu_selection(phone_dial_command_type, status_bytes);
		}
		else if ((phone_dial_command_type >= 0x1000) && (phone_dial_command_type <= 0x1010))
		{
			ICCID_2FE2_DATA_MODE_RAM = phone_dial_command_type & 0xFF;
			/* set flash write flag */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_TASK_OFFSET_POS);
			/* set the mark bit for flash data write check */
			FLASH_DATA_WRITE_CHECK_TASK_QUEUE |= (1 << FLASH_DATA_WRITE_CHECK_ICCID_2FE2_DATA_MODE_OFFSET_POS);
					
#if (IF_LOG_OUTPUT)   
			if (!IF_SOFTDEVICE_RUNNING)
		  {
		    printf("================================== ICCID_2FE2_DATA_MODE_RAM = 0x%x, ==================================\r\n", ICCID_2FE2_DATA_MODE_RAM);
		  }
#endif		
		}	
  }		
	
  write_bytes(2, status_bytes, etu_length, PIN_DATA_PHONE); 
         	
  return(0);
}

/********************************************************************************/
uint32_t printf_selected_file(uint64_t selected_file)
/*--------------------------------------------------------------------------------
| ptint the selected fine name
|
--------------------------------------------------------------------------------*/
{
  printf("selected file: %x, %x, %x, %x, %x, %x\r\n", (uint8_t)(selected_file >> 40), (uint8_t)(selected_file >> 32), 
    (uint8_t)(selected_file >> 24), (uint8_t)(selected_file >> 16), (uint8_t)(selected_file >> 8), (uint8_t)selected_file);
		
	return(0);
}



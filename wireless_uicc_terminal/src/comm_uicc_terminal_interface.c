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
#include "nrf_delay.h"
#include "comm_uicc_terminal_interface.h"
#include "global_data_exchange_extern.h"
#include "nrf_library_update.h"
#include "app_error.h"
#include "comm_initial.h"
#include "nrf_nvic.h"
#include "wireless_sim_phone.h"

#define MAX_PRINTF_SIZE             270
	
/********************************************************************************/
void timer1_initialization(void)
/*--------------------------------------------------------------------------------
| timer1 initializaion, for UICC interface timing control
| 
--------------------------------------------------------------------------------*/
{
  NVIC_EnableIRQ(TIMER1_IRQn);
  NVIC_SetPriority(TIMER1_IRQn, TIMER1_IRQ_PRIORITY);  
  
  NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;
	/* 16M timer frequency */
  NRF_TIMER1->PRESCALER = 0;
  NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  NRF_TIMER1->TASKS_CLEAR = 1;
  NRF_TIMER1->TASKS_SHUTDOWN = 1;
}

/********************************************************************************/
void read_byte(uint8_t *info_byte, uint8_t *check_bit, uint32_t etu_ticks, uint32_t pin_number)
/*--------------------------------------------------------------------------------
|  read a byte and its check bit from the pin_number
|  The initial delay has some dajustment to adapt the ETU error for each bit
|  Limitation: no read error parity check and re-read the same byte again
|	
--------------------------------------------------------------------------------*/
{
  volatile uint32_t time_now; 
  uint8_t etu_timing_offset = 10; 

  /* initialization the read byte */
  *info_byte = 0;
	
	/* get the initial start time */
  NRF_TIMER1->TASKS_CAPTURE[0] = 1;
  time_now = NRF_TIMER1->CC[0]; 
	
  /* waiting time for the initial bit */
  while (time_now + etu_timing_offset + etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  
  /*** read bit 0 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 0);
  while (time_now + etu_timing_offset + 2 * etu_ticks > NRF_TIMER1->CC[0])
  {
    NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  
  /*** read bit 1 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 1);
  while (time_now + etu_timing_offset + 3 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }

  /*** read bit 2 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 2);
  while (time_now + etu_timing_offset + 4 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }

  /*** read bit 3 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 3);
  while (time_now + etu_timing_offset + 5 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }

  /*** read bit 4 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 4);
  while (time_now + etu_timing_offset + 6 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }

  /*** read bit 5 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 5);
  while (time_now + etu_timing_offset + 7 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }

  /*** read bit 6 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 6);
  while (time_now + etu_timing_offset + 8 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }

  /*** read bit 7 ***********************/
  *info_byte |= (nrf_gpio_pin_read(pin_number) << 7);
  while (time_now + etu_timing_offset + 9 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  
  /*** read check bit ***********************/
  *check_bit = nrf_gpio_pin_read(pin_number);
  while (time_now + etu_timing_offset + 10 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
    
  /* wait for the guard time */
  while (time_now + etu_timing_offset + 11 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
}

/********************************************************************************/
void write_byte(uint8_t word_byte, uint8_t parity_bit, uint32_t etu_ticks, uint32_t pin_number)
/*--------------------------------------------------------------------------------
|  write a byte and its check bit to the pin_number
|  Limitation: no write error parity bit check and re-write the same byte again
|
--------------------------------------------------------------------------------*/
{
  volatile uint32_t time_now; 
  
	/* get the initial start time */
  NRF_TIMER1->TASKS_CAPTURE[0] = 1;
  time_now = NRF_TIMER1->CC[0]; 
	
  /* initial bit */	
  nrf_gpio_pin_write(pin_number, 0);
  while (time_now + etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
	
  /* bit 0 */ 
  nrf_gpio_pin_write(pin_number, (word_byte >> 0) & 0x01);
  while (time_now + 2 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 1 */
  nrf_gpio_pin_write(pin_number, (word_byte >> 1) & 0x01);
  while (time_now + 3 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 2 */
  nrf_gpio_pin_write(pin_number, (word_byte >> 2) & 0x01);
  while (time_now + 4 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 3 */
  nrf_gpio_pin_write(pin_number, (word_byte >> 3) & 0x01);
  while (time_now + 5 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 4 */ 
  nrf_gpio_pin_write(pin_number, (word_byte >> 4) & 0x01);
  while (time_now + 6 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 5 */
  nrf_gpio_pin_write(pin_number, (word_byte >> 5) & 0x01);
  while (time_now + 7 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 6 */
  nrf_gpio_pin_write(pin_number, (word_byte >> 6) & 0x01);
  while (time_now + 8 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
  /* bit 7 */ 
  nrf_gpio_pin_write(pin_number, (word_byte >> 7) & 0x01);
  while (time_now + 9 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
	
  /* parity bit */
  nrf_gpio_pin_write(pin_number, parity_bit);
  while (time_now + 10 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
	
  /* guard time, 2 ETU */	 
  nrf_gpio_pin_write(pin_number, 1);
  while (time_now + 12 * etu_ticks > NRF_TIMER1->CC[0])
  {
	  NRF_TIMER1->TASKS_CAPTURE[0] = 1; 
  }
}

#if (IF_LOG_OUTPUT)  
/********************************************************************************/
void printf_log_tx(uint32_t bytes_size, uint8_t *bytes_infor)
/*--------------------------------------------------------------------------------
| bytes string information print at TX side 
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t tmp_size;
	
  tmp_size = (bytes_size < MAX_PRINTF_SIZE) ? bytes_size : MAX_PRINTF_SIZE; 	

  printf("Transmitted_length=%ld: ", bytes_size);
  for (i=0; i<tmp_size; i++)
  {
    printf("%x,  ", *(bytes_infor + i));
  }	
  printf("\r\n");
}

/********************************************************************************/
void printf_log_rx(uint32_t bytes_size, uint8_t *bytes_infor)
/*--------------------------------------------------------------------------------
| bytes string information print at RX side 
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t tmp_size;
	
  tmp_size = (bytes_size < MAX_PRINTF_SIZE) ? bytes_size : MAX_PRINTF_SIZE; 

  printf("Received_length=%ld: ", bytes_size);
  for (i=0; i<tmp_size; i++)
  {
    printf("%x,  ", *(bytes_infor + i));	
  }
  printf("\r\n");
}
#endif

/********************************************************************************/
void get_parity_byte(uint32_t length_byte, uint8_t *byte_in, uint8_t *byte_parity)
/*--------------------------------------------------------------------------------
| get the parity byte of bytes vector 
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;

  *byte_parity = *byte_in;
  for (i=1; i<length_byte; i++)
  {
    *byte_parity ^= *(byte_in + i); 
  }
}
	
/********************************************************************************/
void get_parity_bit(uint8_t byte_in, uint8_t *bit_parity)
/*--------------------------------------------------------------------------------
| get the parity bit of a byte 
|
--------------------------------------------------------------------------------*/
{
  *bit_parity = ((byte_in >> 0) & 0x01) ^ ((byte_in >> 1) & 0x01);
  *bit_parity ^= ((byte_in >> 2) & 0x01);
  *bit_parity ^= ((byte_in >> 3) & 0x01);
  *bit_parity ^= ((byte_in >> 4) & 0x01);
  *bit_parity ^= ((byte_in >> 5) & 0x01);
  *bit_parity ^= ((byte_in >> 6) & 0x01);
  *bit_parity ^= ((byte_in >> 7) & 0x01);
}

/********************************************************************************/
uint32_t write_bytes(uint32_t bytes_length, uint8_t *bytes_info, uint32_t etu_length, 
         uint32_t pin_number)
/*--------------------------------------------------------------------------------
|  write bytes string with length bytes_length to PIN number pin_number
|  Limitation: no write error parity bit check and re-write the same byte again
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t check_bits;
	
#if (IF_LOG_OUTPUT)
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_rx(bytes_length, bytes_info);
  }
#endif  
	
	nrf_delay_us(100);
  for (i=0; i<bytes_length; i++)
  {
    get_parity_bit(*(bytes_info + i), &check_bits );
    write_byte(*(bytes_info + i), check_bits, etu_length, pin_number);
  }  

   return(0);
}

/********************************************************************************/
uint32_t read_bytes_phone(uint32_t read_length, uint8_t *bytes_info,
         uint32_t pin_number, uint32_t etu_length)
/*--------------------------------------------------------------------------------
|  read bytes string with length read_length from PIN number pin_number of phone side
|  Limitation: no read error parity bit check and re-read the same byte again
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t initial_timer;
	uint8_t  check_bit[1]; 
	
  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  
  for (i=1; i<read_length + 1; i++)
  {
    while (nrf_gpio_pin_read(pin_number))
    {
      /* re-load watch dog request register */
		  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
      {				
        return(1);
      }
    }	
    read_byte(bytes_info + i, check_bit, etu_length, pin_number);

    /* clock signal, false alarm read, discard the byte */
    if ((*(bytes_info + 1) == 0xff) && (*check_bit == 0x1) && (i == 1))
    {
      i = 1;
    }
  }
	
  /* To be updated, read_length maybe larger than 255 */
  /* for wireless SIM-phone interface, the read bytes length is less than 255,
  * since the authentication command (0x88) data length is less than 255 */
  *(bytes_info + 0)	= read_length;
	
#if (IF_LOG_OUTPUT)
	if (!IF_SOFTDEVICE_RUNNING)	
	{	
    printf_log_tx(read_length, bytes_info + 1);
	}
#endif  
			
	nrf_delay_us(100);
  return(0);  	
}

/********************************************************************************/
uint32_t read_bytes_ble(uint32_t read_length, uint8_t *bytes_info, uint32_t pin_number, 
         uint32_t etu_length, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| read bytes vector with length read_length from PIN number pin_number of phone side,
| for BLE stack time slot
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t check_bit[1];
  volatile uint32_t time_now; 

  *bytes_info = read_length;
	for (i=1; i<read_length + 1; i++)
  {
    while (nrf_gpio_pin_read(pin_number))
    {
			/* get the current timer0 */
		  NRF_TIMER0->TASKS_CAPTURE[0] = 1;
		  time_now = NRF_TIMER0->CC[0]; 
      if ((time_now - start_time_us) > time_length_us)
      {	
        return(1);
      }
    }	
    read_byte(bytes_info + i, check_bit, etu_length, pin_number);

    /* clock signal, false alarm read, discard the byte */		
    if ((*(bytes_info + 1) == 0xff) && (*check_bit == 0x1) && (i == 1))
    {
      i = 1;
    }
  }

	nrf_delay_us(100);
  return(0);  	
}

/********************************************************************************/
uint32_t parity_bit_check(uint32_t length_byte, uint8_t *byte_in, uint8_t *bit_parity)
/*--------------------------------------------------------------------------------
| check if the parity is correct for each byte of length length_byte
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint8_t tmp_parity_bit;
	
  for (i=0; i<length_byte; i++)
  {
    get_parity_bit(*(byte_in + i), &tmp_parity_bit);
    if (tmp_parity_bit != *(bit_parity + i))
    {
#if (IF_LOG_OUTPUT)
			if (!IF_SOFTDEVICE_RUNNING)	
			{			
        printf("\r\n-----------Parity bit check error, byte=%x, parity bit =%x, \r\n", 
			    *(byte_in + i), *(bit_parity + i));
			}
#endif
      return (1);			
    }
  }	
	
  return (0);		
}



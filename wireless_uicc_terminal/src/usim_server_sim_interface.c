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
#include "comm_phone_command.h"
#include "phone_command_sim.h"
#include "nrf_library_update.h"
#include "wireless_sim_phone.h"
#include "flash_file_address.h"
#include "usim_server_sim_interface.h"
#include "nrf_library_update.h"
#include "comm_uicc_terminal_interface.h"
#include "flash_file_address.h"
#include "comm_initial.h"
#include "global_data_exchange_extern.h"
#include "flash_data_write.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

/********************************************************************************/
void set_default_usim0(void)
/*--------------------------------------------------------------------------------
| set USIM0 be the default phone used local SIM
|
--------------------------------------------------------------------------------*/
{
  PIN_CLOCK_SIM = PIN_CLOCK_SIM0;
  PIN_DATA_SIM  = PIN_DATA_SIM0;
  PIN_RESET_SIM = PIN_RESET_SIM0;
  PIN_VCC_SIM   = PIN_VCC_SIM0;
}

/********************************************************************************/
void set_default_usim0_etu(void)
/*--------------------------------------------------------------------------------
| USIM0 is the default phone used local SIM and ETU tick, selected USIM EF
|
--------------------------------------------------------------------------------*/
{
  set_default_usim0( );
  ETU_TICKS_SIM = ETU_TICKS_SIM0;
  SELECTED_FILE_SIM = SELECTED_FILE_USIM0;
}

#if (PIN_VCC_SIM1 != PIN_NULL)
/********************************************************************************/
void set_default_usim1(void)
/*--------------------------------------------------------------------------------
| set USIM1 be the default phone used local SIM
|
--------------------------------------------------------------------------------*/
{
  PIN_CLOCK_SIM = PIN_CLOCK_SIM1;
  PIN_DATA_SIM  = PIN_DATA_SIM1;
  PIN_RESET_SIM = PIN_RESET_SIM1;
  PIN_VCC_SIM   = PIN_VCC_SIM1;
}

/********************************************************************************/
void set_default_usim1_etu(void)
/*--------------------------------------------------------------------------------
| USIM1 is the default phone used local SIM and ETU tick, selected USIM EF
|
--------------------------------------------------------------------------------*/
{
  set_default_usim1( );
  ETU_TICKS_SIM = ETU_TICKS_SIM1;
  SELECTED_FILE_SIM = SELECTED_FILE_USIM1;
}
#endif

/********************************************************************************/
void sim_clock_signal_config(uint32_t pin_clock)
/*--------------------------------------------------------------------------------
| 4M SIM clock signal config, use GPIOTE channel 0
|
--------------------------------------------------------------------------------*/
{
  /* Configure GPIOTE channel 0 to toggle the pin state */
  nrf_gpiote_task_configure(0, pin_clock, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
  nrf_gpiote_task_enable(0);

  /* Configure PPI channel 0 to toggle OUTPUT_PIN on every TIMER2 COMPARE[0] match */
  NRF_PPI->CH[0].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[0];
  NRF_PPI->CH[0].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];

  /* Enable PPI channel 0 */
  NRF_PPI->CHEN |= (PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos);

  /* Configure timer 2 */
  NRF_TIMER2->MODE      = TIMER_MODE_MODE_Timer;
  NRF_TIMER2->BITMODE   = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
  NRF_TIMER2->PRESCALER = 0;
  NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;

  /* Clears the timer, sets it to 0 */
  NRF_TIMER2->TASKS_CLEAR = 1;

  /* Load the initial values to TIMER2 CC registers, TIMER2 clock frequency is 16MHz,
  every 2 clock tick toggle the pin, the clock frequecy output from the pin is 4MHz */
  NRF_TIMER2->CC[0] = 2;
}

/********************************************************************************/
void clock_sim_start_4m(uint32_t pin_clock)
/*--------------------------------------------------------------------------------
| start to output 4M clock signal to pin_clock
|
--------------------------------------------------------------------------------*/
{
  /* Configure and enable GPIOTE channel 0 to toggle the pin state */
  sim_clock_signal_config(pin_clock);
  /* start timer 2 */
  NRF_TIMER2->TASKS_START = 1;
  nrf_delay_us(200);
}

/********************************************************************************/
void clock_sim_stop_4m(uint32_t pin_clock)
/*--------------------------------------------------------------------------------
| stop to output 4M clock signal to pin_clock at low state
|
--------------------------------------------------------------------------------*/
{
  /* disable GPIOTE channel 0 */
  NRF_GPIOTE->CONFIG[0] =  0;
  /* stop timer 2 */
  NRF_TIMER2->TASKS_STOP = 1;
  /* put the CLK pin low state */
  nrf_gpio_cfg_output(pin_clock);
  nrf_gpio_pin_write(pin_clock, 0);

  nrf_delay_us(100);
}

/********************************************************************************/
void uicc_reader_sim_pin_setup(void)
/*--------------------------------------------------------------------------------
| phone-USIM interface GPIO pin setting for USIM card reader
|
--------------------------------------------------------------------------------*/
{
  nrf_gpio_cfg_output(PIN_RESET_SIM0);
  nrf_gpio_cfg_output_H0H1(PIN_VCC_SIM0);
  nrf_gpio_cfg_output_H0D1(PIN_DATA_SIM0);

#if (PIN_VCC_SIM1 != PIN_NULL)
  nrf_gpio_cfg_output(PIN_RESET_SIM1);
  nrf_gpio_cfg_output_H0H1(PIN_VCC_SIM1);
  nrf_gpio_cfg_output_H0D1(PIN_DATA_SIM1);
#endif
}

/********************************************************************************/
void uicc_reader_sim_init(void)
/*--------------------------------------------------------------------------------
| USIM card reader initialization
|
--------------------------------------------------------------------------------*/
{
  /* set all pin at low state */
  clock_sim_stop_4m(PIN_CLOCK_SIM);
  nrf_gpio_pin_write(PIN_RESET_SIM, 0);
  nrf_gpio_pin_write(PIN_DATA_SIM, 0);
  nrf_gpio_pin_write(PIN_VCC_SIM, 0);

  /* put VCC High */
  nrf_delay_us(500);
  nrf_gpio_pin_write(PIN_VCC_SIM, 1);
  /* start clock */
  nrf_delay_us(500);
  clock_sim_start_4m(PIN_CLOCK_SIM);
  /* put IO High */
  nrf_delay_us(500);
  nrf_gpio_pin_write(PIN_DATA_SIM, 1);
}

/********************************************************************************/
uint32_t soft_warm_reset_sim(uint32_t etu_length, uint32_t if_PPS)
/*--------------------------------------------------------------------------------
| reset USIM session
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t length_ATR;
  uint32_t is_td;
  uint8_t tmp_byte;
  uint32_t initial_RTC2_ms;
  const uint32_t max_usim_detection_ms = 15;
  uint8_t check_bit;

  initial_RTC2_ms = NRF_RTC2->COUNTER;
  /* read TS */
  for (i=0; i<1; i++)
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      /* the max response time is 40000 CLK cycles, for 4MHz CLK signal, it is 10ms  */
      if ((NRF_RTC2->COUNTER - initial_RTC2_ms) > max_usim_detection_ms)
      {
        return (2);
      }
    }
    nrf_delay_us(10);
    read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
  }

  /* read T0 */
  length_ATR = 1;
  for (i=0; i<length_ATR; i++)
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
    nrf_delay_us(10);
    read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
  }

  /* TA1, TB1, TC1, TD1 transmitted */
  is_td = 0;
  if (*(READ_BYTE_UICC_TERMINAL + i - 1) & 0xf0)
  {
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x10)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x20)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x40)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x80)
    {
      length_ATR++;
      is_td = 1;
    }
    /* read TA1, TB1, TC1, TD1 */
    for (i=i; i<length_ATR; i++)
    {
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
      nrf_delay_us(10);
      read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
    }
  }

  /* TA2, TB2, TC2, TD2 transmitted */
  if (is_td)
  {
    is_td = 0;
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x10)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x20)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x40)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x80)
    {
      length_ATR++;
      is_td = 1;
    }
    /* read TA2, TB2, TC2, TD2 */
    for (i=i; i<length_ATR; i++)
    {
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
      nrf_delay_us(10);
      read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
    }
  }

  /* TA3, TB3, TC3, TD3 transmitted */
  if (is_td)
  {
    is_td = 0;
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x10)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x20)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x40)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x80)
    {
      length_ATR++;
      is_td = 1;
    }
    /* read TA3, TB3, TC3, TD3 */
    for (i=i; i<length_ATR; i++)
    {
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
      nrf_delay_us(10);
      read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
    }
  }

  /* TA4, TB4, TC4, TD4 transmitted */
  if (is_td)
  {
    is_td = 0;
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x10)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x20)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x40)
    {
      length_ATR++;
    }
    if(*(READ_BYTE_UICC_TERMINAL + i - 1) & 0x80)
    {
      length_ATR++;
      is_td = 1;
    }
    /* read TA4, TB4, TC4, TD4 */
    for (i=i; i<length_ATR; i++)
    {
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
      nrf_delay_us(10);
      read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
    }
  }

  /* historical character bytes number */
  length_ATR += (*(READ_BYTE_UICC_TERMINAL + 0) & 0x0f);
  /* read historical character bytes */
  for (i=i; i<length_ATR; i++)
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
    }
    nrf_delay_us(10);
    read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
  }

  if((((*(READ_BYTE_UICC_TERMINAL + 0) >> 4) & 0x0f) != 1) && (*(READ_BYTE_UICC_TERMINAL + 2) != 0x00))
  {
    /* parity byte */
    length_ATR++;
    /* read parity byte */
    for (i=i; i<length_ATR; i++)
    {
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
      nrf_delay_us(10);
      read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
    }
  }

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

#if (IF_LOG_OUTPUT)
  if (!IF_SOFTDEVICE_RUNNING)
  {
    printf_log_tx(length_ATR, READ_BYTE_UICC_TERMINAL);
  }
#endif

  tmp_byte = *(READ_BYTE_UICC_TERMINAL + 1);
  if (if_PPS)
  {
    *(READ_BYTE_UICC_TERMINAL + 0) = 0xff;
    *(READ_BYTE_UICC_TERMINAL + 1) = 0x10;
    if (tmp_byte == 0x11)
    {
      *(READ_BYTE_UICC_TERMINAL + 2) = 0x11;
      ETU_TICKS_SIM = 1488;
    }
    else if (tmp_byte == 0x94)
    {
      *(READ_BYTE_UICC_TERMINAL + 2) = 0x94;
      ETU_TICKS_SIM = 256;
    }
    else if (tmp_byte == 0x95)
    {
      *(READ_BYTE_UICC_TERMINAL + 2) = 0x95;
      ETU_TICKS_SIM = 256 >> 1;
    }
    else if (tmp_byte == 0x96)
    {
      *(READ_BYTE_UICC_TERMINAL + 2) = 0x96;
      ETU_TICKS_SIM = 256 >> 2;
    }
    else
    {
#if (IF_LOG_OUTPUT)
      if (!IF_SOFTDEVICE_RUNNING)
      {
        printf("ETU time not supported by UICC-terminal interface...");
      }
#endif
      return (1);
    }
    /* save USIM ETU ticks number */
    if (PIN_DATA_SIM == PIN_DATA_SIM0)
    {
      ETU_TICKS_SIM0 = ETU_TICKS_SIM;
    }
#if (PIN_VCC_SIM1 != PIN_NULL)
    else if (PIN_DATA_SIM == PIN_DATA_SIM1)
    {
      ETU_TICKS_SIM1 = ETU_TICKS_SIM;
    }
#endif

    get_parity_byte(3, READ_BYTE_UICC_TERMINAL, READ_BYTE_UICC_TERMINAL + 3);

#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      printf_log_rx(4, READ_BYTE_UICC_TERMINAL);
    }
#endif

    nrf_delay_ms(1);

    write_bytes(4, READ_BYTE_UICC_TERMINAL, etu_length, PIN_DATA_SIM);

    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;

    for (i=0; i<4; i++)
    {
      while (nrf_gpio_pin_read(PIN_DATA_SIM))
      {
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      }
      nrf_delay_us(10);
      read_byte(READ_BYTE_UICC_TERMINAL + i, &check_bit, etu_length, PIN_DATA_SIM);
    }

#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      printf_log_tx(4, READ_BYTE_UICC_TERMINAL);
    }
#endif
  }

  return(0);
}

/********************************************************************************/
uint32_t initialization_sim_soft(uint32_t etu_length)
/*--------------------------------------------------------------------------------
| Initialization of USIM session
|
--------------------------------------------------------------------------------*/
{
  /* send reset signal */
  nrf_delay_us(500);
  nrf_gpio_pin_write(PIN_RESET_SIM, 1);
  nrf_delay_us(100);

  return(soft_warm_reset_sim(etu_length, 1));
}

/********************************************************************************/
uint32_t sim_command_select(uint32_t selected_file_ID, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| send command to SIM, select file with file ID selected_file_ID
|
--------------------------------------------------------------------------------*/
{
  uint8_t select_command[5] = {0x0, 0xa4, 0x0, 0x4, 0x2};
  uint8_t selected_file[2] = {0x0, 0x0};
  uint8_t get_response_command[5] = {0x0, 0xc0, 0x0, 0x0, 0x0};

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  selected_file[0] = (uint8_t)(selected_file_ID >> 8);
  selected_file[1] = (uint8_t)(selected_file_ID);

  write_bytes(5, select_command, etu_length, PIN_DATA_SIM);

  if (read_bytes_sim(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 1))
  {
    return(1);
  }
  if (*(READ_BYTE_UICC_TERMINAL + 1) != 0xa4)
  {
    return(1);
  }

  write_bytes(2, selected_file, etu_length, PIN_DATA_SIM);

  if (read_bytes_sim(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 1))
  {
    return(1);
  }

  if ((*(READ_BYTE_UICC_TERMINAL + 1)  == 0x6a) && (*(READ_BYTE_UICC_TERMINAL + 2)  == 0x82))
  {
#if (IF_LOG_OUTPUT)
    printf("Selected file not existed......\r\n");
#endif

    return(1);
  }

  get_response_command[4] = *(READ_BYTE_UICC_TERMINAL + 2);
  write_bytes(5, get_response_command, etu_length, PIN_DATA_SIM);
  if (read_bytes_sim(get_response_command[4] + 3, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 0))
  {
    return(1);
  }

  return(0);
}

/********************************************************************************/
uint32_t reset_sim_cold(void)
/*--------------------------------------------------------------------------------
| cold reset USIM card
|
--------------------------------------------------------------------------------*/
{
  /* USIM card reader initialization */
  nrf_delay_ms(5);
  uicc_reader_sim_init( );
  /* SIM initialization */
  initialization_sim_soft(ETU_TICKS_SIM_INITIAL);

  /* read USIM EF data */
  sim_command_select(0x3f00, ETU_TICKS_SIM);
  sim_command_select_adf(ETU_TICKS_SIM);

  return(0);
}

/********************************************************************************/
uint32_t read_bytes_sim(uint32_t read_length, uint8_t *bytes_info,
         uint32_t pin_number, uint32_t etu_length, uint8_t if_procedure_byte)
/*--------------------------------------------------------------------------------
|  read bytes string with length of read_length from PIN number pin_number of USIM side
|  Limitation: no read error parity bit check and re-read the same byte again
|
--------------------------------------------------------------------------------*/
{
  uint32_t i;
  uint32_t initial_timer;
  uint8_t check_bit[1];

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;

  for (i=1; i<2; i++)
  {
    while (nrf_gpio_pin_read(pin_number))
    {
      /* re-load watch dog request register */
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if ((NRF_RTC2->COUNTER - initial_timer) > (DATA_TX_TIME_MS << 1))
      {
#if (IF_LOG_OUTPUT)
        if (!IF_SOFTDEVICE_RUNNING)
        {
          printf("1 ----------------- Hi, Reset SIM in read_bytes_sim i = %ld ----------------\r\n", i);
        }
#endif
        /* re-load watch dog request register */
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        /* cold reset USIM*/
        reset_sim_cold( );

        return(1);
      }
    }
    read_byte(bytes_info + i, check_bit, etu_length, pin_number);

    /* clock signal, false alarm read, discard the byte */
    if ((*(bytes_info + 1) == 0xff) && (*check_bit == 0x1) && (i == 1))
    {
      i = 1;
    }

    /* for procedure bye read, if SIM request more time to process, discard the byte to read the next byte */
    if (if_procedure_byte)
    {
      while (*(bytes_info + 1) == 0x60)
      {
        for (i=1; i<2; i++)
        {
          while (nrf_gpio_pin_read(pin_number))
          {
            /* re-load watch dog request register */
            rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
            if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
            {
#if (IF_LOG_OUTPUT)
              if (!IF_SOFTDEVICE_RUNNING)
              {
                printf("2 ----------------- Hi, Reset SIM in read_bytes_sim i = %ld ----------------\r\n", i);
              }
#endif
              /* re-load watch dog request register */
              rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
              /* stop ETU timer tick */
              reset_sim_cold( );

              return(1);
            }
          }
          read_byte(bytes_info + i, check_bit, etu_length, pin_number);
        }
      }
    }
  }

  /* re-load watch dog request register */
  NRF_WDT->RR[0] = 0x6E524635;

  for (i=2; i<read_length + 1; i++)
  {
    while (nrf_gpio_pin_read(pin_number))
    {
      /* re-load watch dog request register */
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
      if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
      {
#if (IF_LOG_OUTPUT)
        if (!IF_SOFTDEVICE_RUNNING)
        {
          printf("3 ----------------- Hi, Reset SIM in read_bytes_sim i = %ld ----------------\r\n", i);
          printf_log_tx(i, bytes_info + 1);
        }
#endif
        /* re-load watch dog request register */
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        /* cold reset USIM*/
        reset_sim_cold( );

        return(1);
      }
    }
    read_byte(bytes_info + i, check_bit, etu_length, pin_number);
  }

  /* if SIM request more time to process, discard the 2 bytes to read SW1, SW2 bytes*/
  if (read_length > 1)
  {
    while (*(bytes_info + read_length - 1) == 0x60)
    {
      *(bytes_info + read_length - 1) = *(bytes_info + read_length);

      while (nrf_gpio_pin_read(pin_number))
      {
        /* re-load watch dog request register */
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
        {
#if (IF_LOG_OUTPUT)
          if (!IF_SOFTDEVICE_RUNNING)
          {
            printf("4 ----------------- Hi, Reset SIM in read_bytes_sim i = %ld ----------------\r\n", i);
          }
#endif
          /* re-load watch dog request register */
          rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
          /* cold reset USIM*/
          reset_sim_cold( );

          return(1);
        }
      }
      read_byte(bytes_info + read_length, check_bit, etu_length, pin_number);
    }
  }

  if ((read_length == 1) && ((*(bytes_info + 1) == 0x6a) || (*(bytes_info + 1) == 0x6e)))
  {
    for (i=2; i<3; i++)
    {
      while (nrf_gpio_pin_read(pin_number))
      {
        /* re-load watch dog request register */
        rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
        if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
        {
#if (IF_LOG_OUTPUT)
          if (!IF_SOFTDEVICE_RUNNING)
          {
            printf("5 ----------------- Hi, Reset SIM in read_bytes_sim i = %ld ----------------\r\n", i);
          }
#endif
          /* re-load watch dog request register */
          rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
          /* cold reset USIM*/
          reset_sim_cold( );

          return(1);
        }
      }
      read_byte(bytes_info + i, check_bit, etu_length, pin_number);
    }
    read_length++;
  }

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
uint32_t sim_command_select_adf(uint32_t etu_length)
/*--------------------------------------------------------------------------------
| USIM command for ADF file selection
|
--------------------------------------------------------------------------------*/
{
  uint8_t select_command[5] = {0x0, 0xa4, 0x4, 0x4, 0x0};
  uint8_t ADF_name[0x11];
  uint8_t get_response_command[5] = {0x0, 0xc0, 0x0, 0x0, 0x0};

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  if (sim_command_read_adf_name(ADF_name, etu_length))
  {
    /* cold reset USIM*/
    reset_sim_cold( );

    return(1);
  }

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  select_command[4] = *(ADF_name + 0);
  write_bytes(5, select_command, etu_length, PIN_DATA_SIM);
  if (read_bytes_sim(1, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 1))
  {
    return(1);
  }

  write_bytes(*(ADF_name + 0), ADF_name + 1, etu_length, PIN_DATA_SIM);

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  read_bytes_sim(2, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 1);
  if (*(READ_BYTE_UICC_TERMINAL + 1) == 0x6a && *(READ_BYTE_UICC_TERMINAL + 2) == 0x82)
  {
    /* cold reset USIM*/
    reset_sim_cold( );

    return(1);
  }

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  get_response_command[4] = *(READ_BYTE_UICC_TERMINAL + 2);
  write_bytes(5, get_response_command, etu_length, PIN_DATA_SIM);
  if (read_bytes_sim(get_response_command[4] + 3, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 0))
  {
    /* cold reset USIM*/
    reset_sim_cold( );

    return(1);
  }

  return(0);
}

/********************************************************************************/
uint32_t sim_command_read_adf_name(uint8_t *ADF_name, uint32_t etu_length)
/*--------------------------------------------------------------------------------
| read USIM file 0x2f00, get the ADF content bytes
|
--------------------------------------------------------------------------------*/
{
  uint8_t read_record_command[5] = {0x0, 0xb2, 0x1, 0x4, 0x0};
  uint32_t ADF_record_length;
  uint32_t i;

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  if (sim_command_select(0x2f00, etu_length))
  {
    /* cold reset USIM*/
    reset_sim_cold( );

    return(1);
  }

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  ADF_record_length = *(READ_BYTE_UICC_TERMINAL + 8) * 0x100 + *(READ_BYTE_UICC_TERMINAL + 9);
  read_record_command[4] = ADF_record_length;
  write_bytes(5, read_record_command, etu_length, PIN_DATA_SIM);
  if (read_bytes_sim(ADF_record_length + 3, READ_BYTE_UICC_TERMINAL, PIN_DATA_SIM, etu_length, 0))
  {
    /* cold reset USIM*/
    reset_sim_cold( );

    return(1);
  }

  for (i=0; i< *(READ_BYTE_UICC_TERMINAL + 5) + 1; i++)
  {
    *(ADF_name + i) = *(READ_BYTE_UICC_TERMINAL + 5 + i);
  }

  return(0);
}

/********************************************************************************/
uint32_t reset_sim_command_select_7fff(void)
/*--------------------------------------------------------------------------------
| SIM file selection for 0x7fff
|
--------------------------------------------------------------------------------*/
{
  uint8_t select_command[5] = {0x0, 0xa4, 0x0, 0x4, 0x2};
  uint8_t selected_file[2] = {0x7f, 0xff};
  uint8_t get_response_command[5] = {0x0, 0xc0, 0x0, 0x0, 0x0};
  uint32_t i;
  uint8_t tmp_info_byte[3];
  uint8_t tmp_check_bit[1];
  uint32_t initial_timer;

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  write_bytes(5, select_command, ETU_TICKS_SIM, PIN_DATA_SIM);

  for (i=1; i<2; i++)
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
      {
        return(1);
      }
    }
    read_byte(tmp_info_byte + i, tmp_check_bit, ETU_TICKS_SIM, PIN_DATA_SIM);
  }

#if (IF_LOG_OUTPUT)
  printf_log_tx(1, tmp_info_byte + 1);
#endif

  write_bytes(2, selected_file, ETU_TICKS_SIM, PIN_DATA_SIM);

  for (i=1; i<3; i++)
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      if((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
      {
        return(1);
      }
    }
    read_byte(tmp_info_byte + i, tmp_check_bit, ETU_TICKS_SIM, PIN_DATA_SIM);
  }

#if (IF_LOG_OUTPUT)
  printf_log_tx(2, tmp_info_byte + 1);
#endif

  if ((tmp_info_byte[1] == 0x6a) && (tmp_info_byte[2] == 0x82))
  {
#if (IF_LOG_OUTPUT)
    printf("------------------------- Hi, Reset USIM in reset_sim_command_select_7fff...\r\n");
#endif
    /* cold reset USIM*/
    reset_sim_cold( );
    return(1);
  }

  get_response_command[4] = *(tmp_info_byte + 2);
  write_bytes(5, get_response_command, ETU_TICKS_SIM, PIN_DATA_SIM);

  for (i=1; i<get_response_command[4] + 4; i++)
  {
    while (nrf_gpio_pin_read(PIN_DATA_SIM))
    {
      if((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
      {
        return(1);
      }
    }
    read_byte(tmp_info_byte, tmp_check_bit, ETU_TICKS_SIM, PIN_DATA_SIM);
  }

#if (IF_LOG_OUTPUT)
  printf("reset_sim_command_select_7fff...\r\n");
#endif

  return(0);
}

/********************************************************************************/
uint32_t sim_file_7fff_selected_check(void)
/*--------------------------------------------------------------------------------
| check the USIM file selection status for 0x7fff
|
--------------------------------------------------------------------------------*/
{
  uint32_t return_value = 0;

  if  ((SELECTED_FILE_SIM == 0x7fff) || ((SELECTED_FILE_SIM >> 16) == 0x7fff))
  {
    return(0);
  }

#if (IF_LOG_OUTPUT)
  if (!IF_SOFTDEVICE_RUNNING)
  {
    printf("USIM EF selected in 0x7fff_selected_check: ");
    printf_selected_file(SELECTED_FILE_SIM);
  }
#endif

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  if (SELECTED_FILE_SIM == 0x3f00)
  {
    if (sim_command_select_adf(ETU_TICKS_SIM))
    {
      return_value = 1;
    }
    else
    {
      SELECTED_FILE_SIM = 0x7fff;
      if (PIN_DATA_SIM == PIN_DATA_SIM0)
      {
        SELECTED_FILE_USIM0 = SELECTED_FILE_SIM;
      }
      else
      {
          SELECTED_FILE_USIM1 = SELECTED_FILE_SIM;
      }
    }
  }
  else
  {
    if (sim_command_select(0x3f00, ETU_TICKS_SIM))
    {
      return_value = 1;
    }
    else
    {
      /* renew RTC2 CC event interrupt and watch-dog */
      rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

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

      if (sim_command_select_adf(ETU_TICKS_SIM))
      {
        return_value = 1;
      }
      else
      {
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
    }
  }

  return(return_value);
}

/********************************************************************************/
uint32_t sim_file_3f00_selected_check(void)
/*--------------------------------------------------------------------------------
| SIM file selection for 0x3f00,
|
--------------------------------------------------------------------------------*/
{
  uint32_t return_value = 0;

  if ((SELECTED_FILE_SIM == 0x3f00) || ((SELECTED_FILE_SIM >> 16) == 0x3f00))
  {
    return (0);
  }

  /* renew RTC2 CC event interrupt and watch-dog */
  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);

  if (SELECTED_FILE_SIM == 0x0)
  {
#if (IF_LOG_OUTPUT)
  if (!IF_SOFTDEVICE_RUNNING)
  {
    printf("USIM EF selected in 0x3f00_selected_check: SELECTED_FILE_SIM = 0x0\r\n");
  }
#endif

    /* USIM card reader initialization */
    uicc_reader_sim_init( );

    /* USIM initialization */
    return_value = initialization_sim_soft(ETU_TICKS_SIM_INITIAL);
    if (!return_value)
    {
      SELECTED_FILE_SIM = 0x1;
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

    return(return_value);
  }
  else if (SELECTED_FILE_SIM == 0x1)
  {
#if (IF_LOG_OUTPUT)
  if (!IF_SOFTDEVICE_RUNNING)
  {
    printf("USIM EF selected in 0x3f00_selected_check: SELECTED_FILE_SIM = 0x1\r\n");
  }
#endif

    if (sim_command_select(0x3f00, ETU_TICKS_SIM))
    {
      return(1);
    }
    else
    {
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

      return(0);
    }
  }

  return(return_value);
}

/********************************************************************************/
void usim_initial_power_on(void)
/*--------------------------------------------------------------------------------
| USIM card inital power on and EF 0x7FFF selection
|
--------------------------------------------------------------------------------*/
{
  /* the configured default USIN card */
  if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 0)
  {
    set_default_usim0( );
  }
#if (PIN_VCC_SIM1 != PIN_NULL)
  else if ((*((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR)) == 1)
  {
    set_default_usim1( );
  }
#endif
  else
  {
#if (IF_LOG_OUTPUT)
    if (!IF_SOFTDEVICE_RUNNING)
    {
      printf("+++ Wrong DEFAULT_USED_USIM value, DEFAULT_USED_USIM = %d, use USIM0 +++\r\n", *((uint8_t *)DEFAULT_USED_USIM_FLASH_ADDR));
    }
#endif

    DEFAULT_USED_USIM_RAM = 0;

    start_flash_page_update(FLASH_WRITE_DATA_SIZE_IN_WORD, USER_CONFIG_FLASH_ADDR, (uint32_t *)P_UINT8_FLASH_DATA_RAM_BUFFER);

    set_default_usim0( );
  }

  NRF_TIMER1->TASKS_START = 1;

  if (usim_existence_detection( ))
  {
    /* stop ETU timer tick */
    NRF_TIMER1->TASKS_CLEAR = 1;
    NRF_TIMER1->TASKS_SHUTDOWN = 1;

    return;
  }

  if (((USIM_CARD_PRESENCE >> 0) & 0x01) == 1)
  {
    set_default_usim0_etu( );
    /* start sim clock signal */
    clock_sim_start_4m(PIN_CLOCK_SIM);

    sim_file_7fff_selected_check( );

    /* stop SIM clock */
    clock_sim_stop_4m(PIN_CLOCK_SIM);
  }
#if (PIN_VCC_SIM1 != PIN_NULL)
  if (((USIM_CARD_PRESENCE >> 1) & 0x01) == 1)
  {
    set_default_usim1_etu( );
    /* start sim clock signal */
    clock_sim_start_4m(PIN_CLOCK_SIM);

    sim_file_7fff_selected_check( );

    /* stop SIM clock */
    clock_sim_stop_4m(PIN_CLOCK_SIM);
  }
#endif

  /* stop ETU timer tick */
  NRF_TIMER1->TASKS_CLEAR = 1;
  NRF_TIMER1->TASKS_SHUTDOWN = 1;

  rtc2_compare0_event_postpone(COMPARE0_EVENT_POSTPONE_USIM_MS);
}


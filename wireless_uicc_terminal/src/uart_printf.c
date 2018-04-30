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
#include "uart_printf.h"
#include "define_config.h"
#include "nrf_library_update.h"
#include "nrf52.h"
#include "nrf_soc.h"

#if (IF_LOG_OUTPUT)
/********************************************************************************/
void uart_initialization(void)
/*--------------------------------------------------------------------------------
| UART initialization
|
--------------------------------------------------------------------------------*/
{
  simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, 0);
}

/********************************************************************************/
uint8_t simple_uart_get(void)
{
  while (NRF_UART0->EVENTS_RXDRDY != 1)
  {
    // Wait for RXD data to be received
  }

  NRF_UART0->EVENTS_RXDRDY = 0;
  return (uint8_t)NRF_UART0->RXD;
}

/********************************************************************************/
uint32_t simple_uart_get_with_timeout(uint32_t timeout_ms, uint8_t *rx_data)
{
  uint32_t initial_timer;

  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;

  while (NRF_UART0->EVENTS_RXDRDY != 1)
  {
    if ((NRF_RTC2->COUNTER - initial_timer) > timeout_ms)
    {
      return (1);
    }
    if (NRF_RTC2->COUNTER < initial_timer)
    {
      initial_timer = NRF_RTC2->COUNTER;
    }
    /* re-load watch dog request register */
    NRF_WDT->RR[0] = 0x6E524635;

  }  // Wait for RXD data to be received

  // clear the event and set rx_data with received byte
  NRF_UART0->EVENTS_RXDRDY = 0;
  *rx_data = (uint8_t)NRF_UART0->RXD;

  return (0);
}

/********************************************************************************/
void simple_uart_put(uint8_t cr)
{
  NRF_UART0->TXD = (uint8_t)cr;

  while (NRF_UART0->EVENTS_TXDRDY!=1)
  {
    // Wait for TXD data to be sent
  }

  NRF_UART0->EVENTS_TXDRDY=0;
}

/********************************************************************************/
void simple_uart_put_with_timeout(uint8_t cr)
{
  uint32_t time_now = NRF_RTC2->COUNTER;

  NRF_UART0->TXD = (uint8_t)cr;

  while (NRF_UART0->EVENTS_TXDRDY != 1)
  {
    // Wait for TXD data to be sent
    if ((NRF_RTC2->COUNTER - time_now) > 10)
    {
      break;
    }
  }

  NRF_UART0->EVENTS_TXDRDY=0;
}

/********************************************************************************/
void simple_uart_config(  uint8_t rts_pin_number,
                          uint8_t txd_pin_number,
                          uint8_t cts_pin_number,
                          uint8_t rxd_pin_number,
                          bool    hwfc)
{
/* @snippet [Configure UART RX and TX pin] */
  nrf_gpio_cfg_output_H0H1(txd_pin_number);
  nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);

  NRF_UART0->PSELTXD = txd_pin_number;
  NRF_UART0->PSELRXD = rxd_pin_number;

/* snippet [Configure UART RX and TX pin] */
  if (hwfc)
  {
    nrf_gpio_cfg_output(rts_pin_number);
    nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
    NRF_UART0->PSELCTS = cts_pin_number;
    NRF_UART0->PSELRTS = rts_pin_number;
    NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
  }

  NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
  NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
  NRF_UART0->TASKS_STARTTX    = 1;
  NRF_UART0->TASKS_STARTRX    = 1;
  NRF_UART0->EVENTS_RXDRDY    = 0;
}

/********************************************************************************/
void simple_uart_putstring(const uint8_t *str)
{
  uint32_t i = 0;
  uint8_t ch = str[i++];

  while (ch != '\0')
  {
    simple_uart_put(ch);
    ch = str[i++];
  }
}

/********************************************************************************/
void simple_uart_getstring(uint8_t *str, uint8_t *str_length)
{
  uint_fast8_t i = 0;
  uint8_t ch;

  while (1)
  {
    ch = simple_uart_get();
    if (ch != '\n')
      str[i++] = ch;
    else
    {
      str[i++] = '\0';
    *str_length = i;
      return;
    }
  }
}

/********************************************************************************/
void simple_uart_getstring_with_timeout(uint32_t timeout_ms, uint8_t max_length, uint8_t *str, uint8_t *str_length)
{
  uint_fast8_t i = 0;
  uint8_t ch;

  while (1)
  {
  if ((!simple_uart_get_with_timeout(timeout_ms, &ch)) && (i < max_length))
  {
      if (ch != '\n')
        str[i++] = ch;
      else
      {
        str[i++] = '\n';
      *str_length = i;
        return;
      }
    }
  else
  {
    *str_length = i;
      return;
  }
  }
}

/********************************************************************************/
int _write(int fd, const char *str, int len)
{
  int i;

  for (i = 0; i < len; i++)
  {
    simple_uart_put_with_timeout(*str++);
  }
  return len;
}

/********************************************************************************/
int _read(int file, char *p_char, int len)
{
  int ret_len = len;
  uint8_t input;

  while (len--)
  {
    input = simple_uart_get();
    while (input)
    {
        // No implementation needed.
    }
    *p_char++ = input;
  }

  return ret_len;
}
#endif


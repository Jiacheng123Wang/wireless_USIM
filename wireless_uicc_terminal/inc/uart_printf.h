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
#ifndef UART_PRINTF_H__
#define UART_PRINTF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "nrf52.h"
#include "nrf_soc.h"

void uart_initialization(void);
uint8_t simple_uart_get(void);
uint32_t simple_uart_get_with_timeout(uint32_t timeout_ms, uint8_t *rx_data);
void simple_uart_put(uint8_t cr);
void simple_uart_put_with_timeout(uint8_t cr);
void simple_uart_config(  uint8_t rts_pin_number,
                          uint8_t txd_pin_number,
                          uint8_t cts_pin_number,
                          uint8_t rxd_pin_number,
                          bool    hwfc);
void simple_uart_putstring(const uint8_t *str);
void simple_uart_getstring(uint8_t *str, uint8_t *str_length);
void simple_uart_getstring_with_timeout(uint32_t timeout_ms, uint8_t max_length, uint8_t *str, uint8_t *str_length);

#endif

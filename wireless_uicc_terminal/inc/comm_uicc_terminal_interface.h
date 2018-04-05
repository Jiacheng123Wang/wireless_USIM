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
#ifndef COMM_UICC_TERMINAL_INTERFACE_H__
#define COMM_UICC_TERMINAL_INTERFACE_H__

#include <stdio.h>

void timer1_initialization(void);
void read_byte(uint8_t *info_byte, uint8_t *check_bit, uint32_t etu_ticks, uint32_t pin_number);
void write_byte(uint8_t word_byte, uint8_t parity_bit, uint32_t etu_ticks, uint32_t pin_number);
void write_byte_last(uint8_t word_byte, uint8_t parity_bit, uint32_t etu_ticks, uint32_t pin_number);
void printf_log_tx(uint32_t bytes_size, uint8_t *bytes_infor);
void printf_log_rx(uint32_t bytes_size, uint8_t *bytes_infor);
void get_parity_byte(uint32_t length_byte, uint8_t *byte_in, uint8_t *byte_parity);
void get_parity_bit(uint8_t byte_in, uint8_t *bit_parity);
uint32_t write_bytes(uint32_t bytes_length, uint8_t *bytes_info, uint32_t etu_length,
        uint32_t pin_number);
uint32_t read_bytes_phone(uint32_t read_length, uint8_t *bytes_info,
        uint32_t pin_number, uint32_t etu_length);
uint32_t parity_bit_check(uint32_t length_byte, uint8_t *byte_in, uint8_t *bit_parity);
uint32_t read_bytes_ble(uint32_t read_length, uint8_t *bytes_info, uint32_t pin_number,
         uint32_t etu_length, uint32_t start_time_us, uint32_t time_length_us);

#endif


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
#ifndef PHONE_COMMAND_SIM_H__
#define PHONE_COMMAND_SIM_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t phone_command_status_response_0xf2(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_unblock_pin_0x2c(uint8_t *phone_command, uint32_t etu_length);
uint32_t phone_command_verify_0x20(uint8_t *phone_command, uint32_t etu_length);
uint32_t phone_command_select_0xa4(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us);
uint32_t phone_command_read_binary_0xb0(uint8_t *phone_command, uint32_t etu_length);
uint32_t phone_command_update_binary_0xd6(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us);
uint32_t phone_command_read_record_0xb2(uint8_t *phone_command, uint32_t etu_length);
uint32_t phone_command_update_record_0xdc(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t start_time_us, uint32_t time_length_us);
uint32_t printf_selected_file(uint64_t selected_file);
uint32_t sim_command_usim_card_update_binary_0xd6_ble(uint32_t update_file_id, uint32_t start_time_us, uint32_t time_length_us);
uint32_t get_usim_file_position_index(uint32_t file_id, uint8_t *file_size, uint32_t *file_offset, uint32_t *flag_offset);
uint32_t sim_command_usim_card_update_binary_0xd6(uint32_t update_file_id);

#endif


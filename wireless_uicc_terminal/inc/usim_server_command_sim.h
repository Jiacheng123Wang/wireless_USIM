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
#ifndef WATCH_COMMAND_SIM_H__
#define WATCH_COMMAND_SIM_H__

#include <stdint.h>

void main_usim_server_phone_command(void);
uint32_t usim_server_command_confirm_sim(uint8_t *phone_command, uint32_t etu_length);
uint32_t usim_server_command_GSM_algorithm(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim);
uint32_t usim_server_command_get_data(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim);
uint32_t usim_server_command_verify(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim);
uint32_t usim_server_command_status(uint8_t *bytes_command, uint32_t etu_length_phone);
uint32_t usim_server_command_GSM_algorithm_ble(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim, uint32_t start_time_us, uint32_t time_length_us);
uint32_t usim_server_command_get_data_ble(uint8_t *bytes_command, uint32_t etu_length_phone,
         uint32_t etu_length_sim, uint32_t start_time_us, uint32_t time_length_us);
uint32_t usim_server_command_confirm_sim_ble(uint8_t *phone_command, uint32_t etu_length,
         uint32_t start_time_us, uint32_t time_length_us);
uint32_t usim_command_authentication_0x88(void);

#endif


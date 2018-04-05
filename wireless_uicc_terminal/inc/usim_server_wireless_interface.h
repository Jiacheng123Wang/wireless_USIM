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
#ifndef WATCH_WIRELESS_INTERFACE_H__
#define WATCH_WIRELESS_INTERFACE_H__

#include <stdint.h>
#include <stdio.h>

uint32_t main_watch_phone_sim_wireless_ble_stage0(void);
uint32_t main_watch_phone_sim_wireless_ble_stage1(void);
void main_watch_phone_sim_wireless(void);
void sim_connection_state_check(uint32_t conncetion_listen);
uint32_t radio_carrier_search(uint32_t max_serach_time_ms);
uint32_t received_command_sim(uint8_t *bytes_command, uint32_t etu_length);
uint32_t usim_server_authentication_wireless(uint8_t *phone_command);
uint32_t sim_file_update(uint32_t read_file_id, uint8_t *update_bytes_all_file);
uint32_t connection_listening_sim(uint32_t start_time_us, uint32_t time_length_us);
uint32_t sim_command_send(uint8_t *phone_command, uint32_t start_time_us, uint32_t time_length_us);
uint32_t usim_server_authentication_wireless_ble(uint8_t *phone_command, uint32_t start_time_us, uint32_t time_length_us);
uint32_t usim_server_command_receive_data(uint8_t *received_data, uint32_t start_time_us, uint32_t time_length_us);
uint32_t phone_logical_address_search(void);
uint32_t radio_carrier_search_ble(void);
uint32_t usim_files_data_read_flash_write(uint8_t usim_no, uint8_t *usim_files_all_data);
#if (IF_LOG_OUTPUT)
void connection_listening_sim_return_message(uint32_t return_code_usim_listening);
#endif

#endif

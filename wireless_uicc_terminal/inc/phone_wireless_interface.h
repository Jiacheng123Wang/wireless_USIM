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
#ifndef PHONE_WIRELESS_INTERFACE_H__
#define PHONE_WIRELESS_INTERFACE_H__

#include <stdint.h>

void main_smart_phone_sim_local_wireless(uint32_t start_time_us, uint32_t time_length_us);
void phone_connection_state_check(uint8_t status_connection_request);
uint32_t connection_request_phone(uint8_t connection_type);
uint32_t phone_command_authentication_wireless(uint8_t *phone_command);
uint32_t phone_command_get_file_data(uint16_t file_ID, uint8_t *file_data);
uint32_t phone_command_send_data(uint8_t *send_data, uint8_t data_length, uint8_t data_type);
uint32_t connection_request_phone_ble(uint8_t connection_type, uint32_t start_time_us, uint32_t time_length_us);
uint32_t phone_command_get_file_data_blank(uint16_t file_ID, uint8_t *file_data);
uint32_t phone_command_get_file_data_ble(uint16_t file_ID, uint8_t *file_data);
uint32_t phone_command_authentication_wireless_ble(uint8_t *phone_command, uint32_t start_time_us, uint32_t time_length_us);
uint32_t connection_request_phone_post_decoding(uint8_t connection_type, uint32_t initial_timer);

#endif


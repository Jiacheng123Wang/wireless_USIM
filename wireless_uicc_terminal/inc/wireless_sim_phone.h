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
#ifndef WIRELESS_SIM_PHONE_H__
#define WIRELESS_SIM_PHONE_H__

#include "stdint.h"

void rtc2_timer_config(void);
void rtc2_radio_link_config(void);
void usim_client_connection_request_time_slot_request(void);
void phone_data_send_check(void);
void usim_server_listening_time_slot_request(void);
uint32_t time_slot_request(uint32_t time_length_us);
void rtc2_compare0_event_posepone(uint32_t posepone_time_ms);
void led_status_check(void);
void flash_data_write_check(void);
void ble_stack_status_check(void);
void usim_power_on_status_check(void);
void usim_file_update_status_check(void);
void config_sych_setting_check(void);
uint32_t usim_binary_update_file_id(uint32_t update_binary);
uint32_t usim_binary_update_file_id_clear(uint32_t file_id);
void usim_binary_update_check(void);
uint32_t usim_binary_update_piggyback(uint8_t *piggyback_data);
void usim_binary_update_wireless_connection(uint8_t *piggyback_data);
void nus_command_process(uint8_t *nus_string);
void idle_mode_text_diaplay_status_check(void);
void ble_stop_task_queue_check(void);
void flash_data_write_task(void);
uint32_t my_systick_config(uint32_t ticks);

#endif


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
#ifndef COMM_PHONE_COMMAND_H__
#define COMM_PHONE_COMMAND_H__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
				
void saadc_init_config(void);
void phone_sim_pin_setup(void);
void gpio_event_in_config(void);
void gpio_event_in_set_clock(void);
void gpio_event_in_clear_clock(void);
void start_ble_data(void);
void stop_ble_data(void);
void set_event_phone_command(void);
void clear_event_phone_command(void);
void wireless_usim_client_mode_set(void);
void wireless_usim_server_mode_set(void);
void set_default_USIM(void);

uint32_t read_phone_command(uint8_t *bytes_command, uint32_t pin_number, uint32_t etu_length);
uint32_t read_soft_warm_reset_phone(uint32_t etu_ticks_initial, uint32_t pin_number);
uint32_t usim_existence_detection(void);
uint32_t soft_warm_reset_phone(uint32_t etu_length, uint32_t pin_number_io);
uint32_t write_phone_response(uint32_t bytes_length, uint8_t *bytes_info, uint32_t etu_length);
uint32_t phone_command_pin_0x24(uint8_t *bytes_command, uint32_t etu_length_phone);
uint32_t phone_command_terminal_profile_0x10(uint8_t *phone_command, uint32_t etu_length, 
         uint32_t pin_number_phone);
uint32_t phone_command_fetch_0x12(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_fetch_reset(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_fetch_diaplay_text(uint8_t *bytes_command, uint8_t string_length, 
         uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_fetch_diaplay_text_variable(uint8_t *bytes_command, uint8_t string_length, 
         uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone, uint8_t variable_bytes);
uint32_t phone_command_fetch_at_cfun0(uint8_t *bytes_command, uint32_t etu_length, 
         uint32_t pin_number_phone);
uint32_t phone_command_fetch_idle_mode_text(uint8_t *bytes_command, uint8_t string_length, 
         uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_fetch_idle_mode_text_variable(uint8_t *bytes_command, uint8_t string_length, 
         uint8_t *display_string, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_search_record_0xa2(uint8_t *phone_command, uint32_t etu_length);
uint32_t phone_command_envelope_0xc2(uint8_t *phone_command, uint32_t etu_length, uint32_t start_time_us, uint32_t time_length_us);
uint32_t phone_command_terminal_response_0x14(uint8_t *phone_command, uint32_t etu_length,
         uint32_t start_time_us, uint32_t time_length_us);
uint32_t phone_command_fetch_set_initial_item(uint8_t *bytes_command, uint32_t etu_length, 
         uint32_t pin_number_phone);
uint32_t phone_command_manage_channel_0x70(uint8_t *bytes_command, uint32_t etu_length_phone);
uint32_t phone_command_default_response(uint8_t *bytes_command, uint32_t etu_length, uint32_t pin_number_phone);
uint32_t phone_command_authentication_wireless_test(void);
uint32_t phone_command_fetch_get_input_command_line(uint8_t *bytes_command, uint32_t etu_length, 
				 uint32_t pin_number_phone);
uint32_t phone_command_fetch_update_iccid(uint8_t *bytes_command, uint32_t etu_length, 
				 uint32_t pin_number_phone);
uint32_t usta_command_line(uint8_t *command_line_string);
uint32_t phone_command_fetch_set_menu(uint8_t *bytes_command, uint32_t etu_length, 
     uint32_t pin_number_phone, uint8_t next_fetch_bytes_length);
uint32_t phone_usat_menu_selection(uint8_t usat_menu_type, uint8_t *status_bytes);

#endif

        

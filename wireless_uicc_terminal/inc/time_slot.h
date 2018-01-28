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
#ifndef TIMESLOT_H__
#define TIMESLOT_H__

#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
		
void handler_time_slot_event_signal_usim_client_random_access_connection_request(void);
void handler_time_slot_event_signal_usim_client_wireless_authentication(void);
void handler_time_slot_event_signal_usim_server_wireless_sim_command_receive(void);
void handler_time_slot_event_signal_usim_server_send_file_data(void);
void handler_time_slot_event_signal_usim_server_wireless_authentication_data_send(void);
void handler_time_slot_event_signal_usim_server_sim_config_data_receive(void);
void handler_time_slot_event_signal_usim_server_sim_command_authentication_0x88(void);
void handler_time_slot_event_signal_usim_server_sim_command_get_data_0xc0(void);
void handler_time_slot_event_signal_usim_server_radio_link_listening(void);
void handler_time_slot_event_signal_usim_server_wireless_authentication_sim_command(void);
void handler_time_slot_event_signal_usim_server_sim_command_adf_selection(void);
void handler_time_slot_event_signal_usim_server_ef_binary_update(void);

void time_slot_request_queue(void);

/*--------------------------------------------------------------------------------
| Time slot sinal handler
|
--------------------------------------------------------------------------------*/
void nrf_evt_signal_handler(uint32_t evt_id, void * p_context);

/*--------------------------------------------------------------------------------
| Time slot event handler
|
--------------------------------------------------------------------------------*/
nrf_radio_signal_callback_return_param_t *time_slot_callback(uint8_t signal_type);


#endif
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
#ifndef WATCH_SIM_INTERFACE_H__
#define WATCH_SIM_INTERFACE_H__

#include <stdint.h>
#include <stdio.h>

void set_default_usim0(void);
void set_default_usim0_etu(void);
#if (PIN_VCC_SIM1 != PIN_NULL)
void set_default_usim1(void);
void set_default_usim1_etu(void);
#endif
void sim_clock_signal_config(uint32_t pin_clock);
void clock_sim_start_4m(uint32_t pin_clock);
void clock_sim_stop_4m(uint32_t pin_clock);
void uicc_reader_sim_pin_setup(void);
void uicc_reader_sim_init(void);
uint32_t soft_warm_reset_sim(uint32_t etu_length, uint32_t if_PPS);
uint32_t initialization_sim_soft(uint32_t etu_length);
uint32_t sim_command_select(uint32_t selected_file_ID, uint32_t etu_length);
uint32_t reset_sim_cold(void);
uint32_t read_bytes_sim(uint32_t read_length, uint8_t *bytes_info,
         uint32_t pin_number, uint32_t etu_length, uint8_t if_procedure_byte);
uint32_t sim_command_select_adf(uint32_t etu_length);
uint32_t sim_command_read_adf_name(uint8_t *ADF_name, uint32_t etu_length);
uint32_t reset_sim_command_select_7fff(void);
uint32_t sim_file_3f00_selected_check(void);
uint32_t sim_file_7fff_selected_check(void);
void usim_initial_power_on(void);

#endif


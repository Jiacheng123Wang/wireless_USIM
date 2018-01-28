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
#ifndef COMM_MISC_H__
#define COMM_MISC_H__

#include <stdint.h>
#include <stdio.h>

uint32_t start_oscillator_16m(void);
void stop_oscillator_16m(void);
uint8_t byte_string_comparison(uint32_t byte_length, uint8_t *byte_1, uint8_t *byte_2);
void usim_data_initialization(void);
void watch_dog_timer_init(uint32_t time_out_s); 
void update_user_configuration(void);
void update_etu_time(void);
uint8_t random_vector_generate(uint8_t *p_buff, uint8_t size, uint32_t start_time_us, uint32_t time_length_us, uint8_t if_bias_correction);
uint32_t usim_data_restore(void);

#endif

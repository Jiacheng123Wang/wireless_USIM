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
#ifndef COMM_WIRELESS_INTERFACE_H__
#define COMM_WIRELESS_INTERFACE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void radio_configure();
uint32_t transmit_power_level_set(uint32_t power_level);
uint32_t radio_mode_set(uint32_t mode);
uint32_t radio_rssi_measurement(uint32_t frequency);
uint32_t radio_carrier_detection(uint32_t max_CD_ms, uint32_t noise_threshold, uint32_t frequency);
uint32_t wireless_sim_authentication_algorithm(uint8_t *user_password, uint8_t *system_key,
         uint8_t *challenge_data, volatile uint8_t *response_data);
uint32_t packet_radio_with_parameters_tx(uint8_t *tx_packet, uint32_t frequency,
       uint32_t logical_address, uint32_t start_time, uint32_t time_length, uint32_t power_level);
uint32_t packet_radio_with_parameters_rx(uint8_t *rx_packet, uint32_t frequency,
       uint32_t logical_address, uint32_t start_time, uint32_t time_length);
uint32_t packet_wireless_transmit_ble(uint8_t *tx_info_byte, uint32_t start_time_us, uint32_t time_length_us);
uint32_t packet_wireless_receive_ble(uint8_t *rx_info_byte, uint32_t start_time_us, uint32_t time_length_us);
uint32_t packet_wireless_transmit(uint8_t *tx_info_byte);
uint32_t packet_wireless_receive(uint8_t *rx_info_byte, uint32_t start_time_ms, uint32_t time_length_ms);

/*------------------------------------------------------------------------------*/
uint32_t int2bit(uint8_t state, uint8_t bit_length, /**/uint8_t *bit_array);
uint32_t bit2int(uint8_t *bit_array, uint8_t bit_length, /**/uint8_t *state);
uint32_t distance_between_vector(uint8_t *input1_bit, uint8_t *input2_bit,
                   uint8_t bit_length);
uint32_t min_index_value(uint32_t *vector_in, uint32_t vector_length,
     /**/ uint32_t *min_value, uint32_t *min_index);
uint32_t encoder_halfrate (uint8_t *input_bit, uint32_t bit_length,
             uint8_t initial_state, uint8_t *encoded_bit);
uint32_t decoder_hallfrate(uint8_t *input_receivebit, uint32_t bit_length,
       uint8_t initial_state, uint8_t *decoded_bit);
uint32_t char2bit_sequence(uint8_t *char_sequence, uint8_t char_length,
                         uint8_t *bit_sequence);
uint32_t bit2char_sequence(uint8_t *bit_sequence, uint8_t char_length,
               uint8_t *char_sequence);
uint32_t ccencoder_byte_sequence_32(uint8_t *input_bytes, uint8_t encode_byte_length,
         uint8_t *encoded_bytes);
uint32_t ccdecoder_byte_sequence_32(uint8_t *input_bytes, uint8_t encode_byte_length,
         uint8_t *decoded_bytes);
uint32_t ccencoder_byte_sequence_64(uint8_t *input_bytes, uint8_t encode_byte_length,
     uint8_t *encoded_bytes);
uint32_t ccdecoder_byte_sequence_64(uint8_t *input_bytes, uint8_t encode_byte_length,
     uint8_t *decoded_bytes);
#endif


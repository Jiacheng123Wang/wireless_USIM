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
#include "comm_wireless_interface.h"
#include "comm_uicc_terminal_interface.h"
#include "comm_initial.h"
#include "define_config.h"
#include "nrf_nvic.h"
#include "time_slot.h"
#include "wireless_sim_phone.h"
#include "global_data_exchange_extern.h"
#include "nrf_delay.h"
#include "define_config.h"
#include "define_all_data.h"

#if (IF_DATA_CC_CODED || IF_CONNECTION_CC_CODED)
/* interleaver address index for convolutional code */
static uint8_t INTERLEAVE_PATTERN[256] = 
  {91, 38, 24, 27, 198, 223, 219, 199, 52, 192, 232, 229, 161, 95, 249, 84, 178, 209,
   60, 145, 44, 70, 50, 240, 208, 237, 220, 217, 31, 62, 110, 242, 54, 68, 73, 153, 
   206, 104, 122, 160, 231, 171, 130, 127, 108, 143, 1, 72, 139, 156, 168, 238, 193, 255, 
   162, 96, 69, 4, 118, 83, 32, 157, 87, 34, 239, 61, 212, 176, 40, 109, 59, 230, 
   66, 115, 205, 74, 177, 26, 71, 128, 214, 204, 197, 36, 225, 123, 81, 174, 7, 11, 
   90, 105, 142, 119, 201, 211, 135, 39, 221, 8, 241, 216, 133, 98, 248, 3, 103, 93, 
   112, 126, 215, 25, 6, 245, 250, 37, 233, 228, 56, 55, 149, 19, 148, 12, 246, 14, 
   48, 218, 152, 151, 35, 172, 46, 182, 236, 170, 173, 29, 164, 117, 165, 207, 99, 235, 
   18, 9, 57, 179, 49, 42, 202, 16, 15, 150, 137, 147, 196, 2, 163, 138, 159, 203, 
   132, 144, 194, 113, 134, 191, 195, 131, 77, 80, 146, 51, 244, 167, 111, 169, 89, 64, 
   65, 79, 227, 47, 121, 213, 30, 53, 251, 43, 23, 85, 254, 63, 188, 210, 155, 125, 
   175, 67, 200, 22, 140, 75, 33, 21, 106, 224, 116, 0, 92, 17, 120, 166, 5, 184, 
   252, 190, 141, 94, 154, 20, 189, 101, 10, 82, 28, 100, 76, 226, 41, 129, 107, 247, 
   158, 181, 253, 86, 183, 186, 114, 243, 222, 187, 185, 102, 58, 180, 124, 136, 45, 88, 
   234, 13, 97, 78};
#endif

/* for radio config */	 
#define PACKET_BASE_ADDRESS_LENGTH       (4UL)  /* Packet base address length field size in bytes */
#define PACKET0_S1_SIZE                  (0UL)  /* S1 size in bits */
#define PACKET0_S0_SIZE                  (0UL)  /* S0 size in bits */
#define PACKET0_PAYLOAD_SIZE             (8UL)  /* payload size in bits */
#define PACKET1_BASE_ADDRESS_LENGTH      (4UL)  /* base address length in bytes */
#define PACKET1_STATIC_LENGTH            (0UL)  /* static length in bytes */
#define PACKET1_PAYLOAD_SIZE             (255UL)/* payload size in bytes */

/********************************************************************************/
void radio_configure()
/*--------------------------------------------------------------------------------
| Radio configuration
|
--------------------------------------------------------------------------------*/
{
  /* Radio address config */
  NRF_RADIO->PREFIX0 = 0xC4D3E2F7UL;  /* Prefix byte of addresses 3 to 0 */
  NRF_RADIO->PREFIX1 = 0xA5B6C7D8UL;  /* Prefix byte of addresses 7 to 4 */
  NRF_RADIO->BASE0   = 0xD7C7B7F7UL;  /* Base address for prefix 0 */
  NRF_RADIO->BASE1   = 0xF0A2B2C2UL;  /* Base address for prefix 1-7 */

  /* Packet configuration */
  NRF_RADIO->PCNF0 = (PACKET0_S1_SIZE << RADIO_PCNF0_S1LEN_Pos) |
                     (PACKET0_S0_SIZE << RADIO_PCNF0_S0LEN_Pos) |
                     (PACKET0_PAYLOAD_SIZE << RADIO_PCNF0_LFLEN_Pos); 

  /* Packet configuration */
  NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos)     |
                     (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)           |
                     (PACKET1_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos)       |
                     (PACKET1_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos)           |
                     (PACKET1_PAYLOAD_SIZE << RADIO_PCNF1_MAXLEN_Pos); 

  /* CRC Config */
	NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos);
  NRF_RADIO->CRCINIT = 0xEFDCAUL;      /* Initial value */   
  NRF_RADIO->CRCPOLY = 0x11021UL;      /* CRC poly: x^16+x^12^x^5+1 */
}

/********************************************************************************/
uint32_t transmit_power_level_set(uint32_t power_level)
/*--------------------------------------------------------------------------------
| Radio Tx power level:
| 0:        -40dBm
| 1:        -20dBm
| 2:        -16dBm
| 3:        -12dBm
| 4:        -8dBm
| 5:        -4dBm
| 6:        0dBm
| 7:        4dBm
| 8:        3dBm
| default:  4dBm
|
--------------------------------------------------------------------------------*/
{
  switch(power_level)
  {
    case 0:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg40dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 1:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg20dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 2:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg16dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 3:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg12dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 4:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg8dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 5:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg4dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 6:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 7:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 8:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos3dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    default:
      NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);
      break;
  }
  
  return(0);
}

/********************************************************************************/
uint32_t radio_mode_set(uint32_t mode)
/*--------------------------------------------------------------------------------
| Radio Tx mode level:
| 0:        250kbps, not for nRF52
| 1:        1Mbps
| 2:        2Mbps
| default: 	1Mbps
|
--------------------------------------------------------------------------------*/
{
  switch(mode)
  {
    case 0:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_250Kbit << RADIO_MODE_MODE_Pos);
      break;
    
    case 1:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_TXPOWER_TXPOWER_Pos);
      break;
    
    case 2:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_2Mbit << RADIO_MODE_MODE_Pos);
      break;
    
    default:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);
      break;
  }
  
  return(0);
}

/********************************************************************************/
uint32_t packet_radio_with_parameters_tx(uint8_t *tx_packet, uint32_t frequency, 
       uint32_t logical_address, uint32_t start_time, uint32_t time_length, uint32_t power_level)
/*--------------------------------------------------------------------------------
| Radio package transmission with configured parameters
| start_time/time_length in us for BLE stack running, TIMER0 used (in time slot), 
|				 in ms for BLE not running, RTC2 used			 
|
--------------------------------------------------------------------------------*/
{
  uint32_t time_out = 0;
  uint32_t radio_mode = RADIO_MODE;  
  volatile uint32_t time_now; 

  /* Set radio mode */
  radio_mode_set(radio_mode);
  /* Set radio Tx power level */
  transmit_power_level_set(power_level);  
  /* Set Frequency bin */
  NRF_RADIO->FREQUENCY = frequency;   
  /* Set device address 0 to use when transmitting */
  NRF_RADIO->TXADDRESS = logical_address;      
  
  /* Set payload pointer */
  NRF_RADIO->PACKETPTR = (uint32_t)tx_packet;  
  
  NRF_RADIO->EVENTS_READY = 0U;
  /* Enable radio and wait for ready */
  NRF_RADIO->TASKS_TXEN = 1U;
  while (NRF_RADIO->EVENTS_READY == 0U)
  {
    if (IF_SOFTDEVICE_RUNNING)
		{			
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0]; 
		}
		else
		{
			time_now = NRF_RTC2->COUNTER;
		}
    /* radio enable time out */  
    if ((time_now - start_time) > time_length)
    {    
      return(1);
    }
  }
  
  /* Start radio Tx task and waiting for complete */
  NRF_RADIO->TASKS_START = 1U;
  NRF_RADIO->EVENTS_END = 0U;  
  /* Wait for end of packet transmitted */
  while(NRF_RADIO->EVENTS_END == 0U)
  {    		
    if (IF_SOFTDEVICE_RUNNING)
		{			
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0]; 
		}
		else
		{
			time_now = NRF_RTC2->COUNTER;
		}
    /* radio transmit time out */  
    if ((time_now - start_time) > time_length)
    {    
      time_out = 1;
      break;
    }
  }
  
  NRF_RADIO->EVENTS_DISABLED = 0U;
  /* Disable radio */
  NRF_RADIO->TASKS_DISABLE = 1U;
  while(NRF_RADIO->EVENTS_DISABLED == 0U)
  {
    if (IF_SOFTDEVICE_RUNNING)
		{			
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0]; 
	    /* radio disable time out */  
	    if ((time_now - start_time) > time_length + TIMER_SLOT_DADIO_DISABLE_MARGIN_US)
	    {    
				break;
	    }
		}
		else
		{
			time_now = NRF_RTC2->COUNTER;
	    /* radio disable time out */  
	    if ((time_now - start_time) > time_length)
	    {    
				break;
	    }
		}
  }
  
  return(time_out);
}

/********************************************************************************/
uint32_t packet_radio_with_parameters_rx(uint8_t *rx_packet, uint32_t frequency, 
       uint32_t logical_address, uint32_t start_time, uint32_t time_length)
/*--------------------------------------------------------------------------------
| radio receiving with time out
| start_time/time_length in us for BLE stack running, TIMER0 used( in time slot), 
|				 in ms for BLE not running, RTC2 used			 
|
--------------------------------------------------------------------------------*/
{
  uint32_t time_out = 0;
  uint32_t radio_mode = RADIO_MODE;
  volatile uint32_t time_now; 
  
  /* Set radio mode */
  radio_mode_set(radio_mode);
  /* Set Frequency bin */
  NRF_RADIO->FREQUENCY = frequency; 
  /* Enable device address to use which receiving */
  NRF_RADIO->RXADDRESSES = logical_address;
  
  /* Set payload pointer */
  NRF_RADIO->PACKETPTR = (uint32_t)rx_packet;
  
  NRF_RADIO->EVENTS_READY = 0U;
  /* Enable radio and wait for ready */
  NRF_RADIO->TASKS_RXEN = 1U;    
  while(NRF_RADIO->EVENTS_READY == 0U)
  {
    if (IF_SOFTDEVICE_RUNNING)
		{			
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0]; 
		}
		else
		{
			time_now = NRF_RTC2->COUNTER;
		}
    /* radio enable time out */  
    if ((time_now - start_time) > time_length)
    {
			return(1);
    }
  }
  	
  NRF_RADIO->EVENTS_END = 0U;
  /* Start listening and wait for address received event */
  NRF_RADIO->TASKS_START = 1U;  
  /* Wait for end of packet recieved */
  while (NRF_RADIO->EVENTS_END == 0U)
  {
    if (IF_SOFTDEVICE_RUNNING)
		{			
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0];
		}
		else
		{
			time_now = NRF_RTC2->COUNTER;
		}
    /* radio receive time out */  
    if ((time_now - start_time) > time_length)
    {    
      time_out = 1;
      break;
    }
  }  
  
  NRF_RADIO->EVENTS_DISABLED = 0U;
  /* Disable radio */
  NRF_RADIO->TASKS_DISABLE = 1U;
  while(NRF_RADIO->EVENTS_DISABLED == 0U)
  {
    if (IF_SOFTDEVICE_RUNNING)
		{			
		  /* get the current timer0 */
	    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	    time_now = NRF_TIMER0->CC[0]; 
	    /* radio disable time out */  
	    if ((time_now - start_time) > time_length + TIMER_SLOT_DADIO_DISABLE_MARGIN_US)
	    {
				break;
	    }
	  }
	  else
	  {
	   	time_now = NRF_RTC2->COUNTER;
	    /* radio disable time out */  
	    if ((time_now - start_time) > time_length)
	    {
				break;
	    }
	  }
  }
  
  /* 1: time out, 0: packet recceived */ 
  return (time_out);
}

/********************************************************************************/
uint32_t packet_wireless_transmit(uint8_t *tx_info_byte)
/*--------------------------------------------------------------------------------
| data packet wireless transmission
|
--------------------------------------------------------------------------------*/
{
  uint32_t initial_timer;
  uint8_t ack_rx[4] = {0, 0, 0, 0};  
   
  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  while (1)
  {
    /* data carrier collision detection */
    radio_carrier_detection(MAX_CD_TIME_MS, TX_RSSI_THRED, DATA_CARRIER_FREQ);
    /* data packet transmit */
    packet_radio_with_parameters_tx(tx_info_byte, DATA_CARRIER_FREQ, USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS, NRF_RTC2->COUNTER, DATA_TX_TIME_INTERVAL_MS, RADIO_TX_POWER_DATA); 
     
    /* ACK receive */                          
    if(!(packet_radio_with_parameters_rx(ack_rx, DATA_CARRIER_FREQ, 1 << USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS, NRF_RTC2->COUNTER, ACK_RX_TIME_MS)))
    {
			rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
      return(0);
    }
    /* data packet transmit time out */
    if ((NRF_RTC2->COUNTER - initial_timer) > DATA_TX_TIME_MS)
    {
			rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
      return(1);
    }
		
		rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
  }  
}

/********************************************************************************/
uint32_t packet_wireless_receive(uint8_t *rx_info_byte, uint32_t start_time_ms, uint32_t time_length_ms)
/*--------------------------------------------------------------------------------
| data packet wireless receiving
|
--------------------------------------------------------------------------------*/
{
  uint32_t logical_address_tx = USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS;
  uint32_t logical_address_rx = (1 << logical_address_tx);
  uint32_t frequency = DATA_CARRIER_FREQ;
  uint32_t tx_power_level_ack = POWER_LEVEL_ACK;
  uint8_t ack_tx[4] = {3, 1, 1, 1};
  
  *rx_info_byte = 0;
  while (1)
	{
	  if (packet_radio_with_parameters_rx(rx_info_byte, frequency, logical_address_rx, start_time_ms, time_length_ms - DATA_TX_TIME_INTERVAL_MS))
	  {
		  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
		  return(1);	
	  }
		
	  /* if CRC check OK, give a ACK feeaback */		
		if (NRF_RADIO->CRCSTATUS == 1U)
		{
		  if (packet_radio_with_parameters_tx(ack_tx, frequency, logical_address_tx, start_time_ms, time_length_ms, tx_power_level_ack))
		  {
			  rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
			  return(1);	
		  }
			else 
			{
				break;
			}
		}
		else
		{
#if (IF_LOG_OUTPUT)
	     printf("++++++++++++++++++++++++++++++++++++ packet received, CRC error ++++++++++++++++++++++++++++++++++++\r\n");
#endif
		}
		if ((NRF_RTC2->COUNTER - start_time_ms) > time_length_ms - DATA_TX_TIME_INTERVAL_MS)
		{
			rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
			return(1);
		}
	}
	
	rtc2_compare0_event_posepone(COMPARE0_EVENT_POSEPONE_USIM_MS);
	return(0);			 
}

/********************************************************************************/
uint32_t packet_wireless_transmit_ble(uint8_t *tx_info_byte, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| data packet wireless transmission
|
--------------------------------------------------------------------------------*/
{
  uint32_t tx_power_level_data = RADIO_TX_POWER_DATA;
  uint32_t frequency = DATA_CARRIER_FREQ;
  uint32_t logical_address_tx = USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS;
  uint32_t logical_address_rx = (1 << logical_address_tx);
  uint8_t  ack_rx[4] = {0, 0, 0, 0};  
 

	if (packet_radio_with_parameters_tx(tx_info_byte, frequency, logical_address_tx, 
	  start_time_us, time_length_us - TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US, tx_power_level_data))
	{
		return(1);
	}
	return (packet_radio_with_parameters_rx(ack_rx, frequency, logical_address_rx, 
	  start_time_us, time_length_us));
}

/********************************************************************************/
uint32_t packet_wireless_receive_ble(uint8_t *rx_info_byte, uint32_t start_time_us, uint32_t time_length_us)
/*--------------------------------------------------------------------------------
| data packet wireless receiving
|
--------------------------------------------------------------------------------*/
{
  uint32_t logical_address_tx = USIM_CLIENT_WIRELESS_LOGICAL_ADDRESS;
  uint32_t logical_address_rx = (1 << logical_address_tx);
  uint32_t frequency = DATA_CARRIER_FREQ;
  uint32_t tx_power_level_ack = POWER_LEVEL_ACK;
  uint8_t ack_tx[4] = {3, 1, 1, 1};
  
  *rx_info_byte = 0;
  
	if (packet_radio_with_parameters_rx(rx_info_byte, frequency, logical_address_rx, 
		  start_time_us, time_length_us - TIMER_SLOT_RADIO_TX_RX_STATE_TRANS_MARGIN_US))
	{
		return(1);	
	}
	
	/* if CRC check OK, give a ACK feeaback */
	if (NRF_RADIO->CRCSTATUS == 1U)
	{
	  return(packet_radio_with_parameters_tx(ack_tx, frequency, logical_address_tx, 
	    start_time_us, time_length_us, tx_power_level_ack)); 
	} 
	else /* no feedback */
	{
		return(1);
	}
}

/********************************************************************************/
uint32_t radio_rssi_measurement(uint32_t frequency)
/*--------------------------------------------------------------------------------
| RSSI measurement 
|
--------------------------------------------------------------------------------*/
{
  uint32_t initial_timer;
  uint32_t radio_time_out_ms = 2;
  uint32_t rssi_value;
   
  /* Set radio mode */
  radio_mode_set(RADIO_MODE);
  /* Set Frequency bin */
  NRF_RADIO->FREQUENCY = frequency; 
  /* Enable device address to use which receiving */
  NRF_RADIO->RXADDRESSES = 0xff;
  
  NRF_RADIO->EVENTS_READY = 0U;
  /* Enable radio in Rx mode and wait for ready */
  NRF_RADIO->TASKS_RXEN = 1U;    
  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  while(NRF_RADIO->EVENTS_READY == 0U)
  {      
    /* radio enable time out */  
    if ((NRF_RTC2->COUNTER - initial_timer) > radio_time_out_ms)
    {    
#if (IF_LOG_OUTPUT)
      printf("..................Radio enable time out in RSSI measurement..................\r\n" );
#endif   
      /* system reset */
      sd_nvic_SystemReset( );
    }
    if (NRF_RTC2->COUNTER < initial_timer)
    {
      initial_timer = NRF_RTC2->COUNTER;
    }
  }

///////////////////////////////////////////////////////////////////  
  /* Start RSSI sample */
  NRF_RADIO->EVENTS_RSSIEND = 0;
  NRF_RADIO->TASKS_RSSISTART = 1U;
  while (NRF_RADIO->EVENTS_RSSIEND == 0)
  {
    /* radio RSSI sampling time out */  
    if ((NRF_RTC2->COUNTER - initial_timer) > radio_time_out_ms)
    {    
#if (IF_LOG_OUTPUT)
      printf("+++++++++++++++++++++++ RSSI value get time out in RSSI measurement +++++++++++++++++++++++\r\n" );
#endif   
      /* system reset */
      sd_nvic_SystemReset( );
    }      
    if (NRF_RTC2->COUNTER < initial_timer)
    {
      initial_timer = NRF_RTC2->COUNTER;
    }
  }

  rssi_value = NRF_RADIO->RSSISAMPLE;
  NRF_RADIO->TASKS_RSSISTOP = 1U;
///////////////////////////////////////////////////////////////////  
  
  NRF_RADIO->EVENTS_DISABLED = 0U;
  /* Disable radio */
  NRF_RADIO->TASKS_DISABLE = 1U;
  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;
  while(NRF_RADIO->EVENTS_DISABLED == 0U)
  {
    /* radio enable time out */  
    if ((NRF_RTC2->COUNTER - initial_timer) > radio_time_out_ms)
    {    
#if (IF_LOG_OUTPUT)
      printf("+++++++++++++++++++++++ Radio disable time out in RSSI measurement +++++++++++++++++++++++\r\n" );
#endif   
      /* system reset */
      sd_nvic_SystemReset( );
    }
    if (NRF_RTC2->COUNTER < initial_timer)
    {
      initial_timer = NRF_RTC2->COUNTER;
    }	
  }  
  
  return (rssi_value);
}

/********************************************************************************/
uint32_t radio_carrier_detection(uint32_t max_CD_ms, uint32_t noise_threshold, uint32_t frequency)
/*--------------------------------------------------------------------------------
| frequency carrier detection, wait for some time if the backgroup noise is larger the
| the threshold
|
--------------------------------------------------------------------------------*/
{
  uint32_t initial_timer;
  
  /* get the initial real time counter */
  initial_timer = NRF_RTC2->COUNTER;

  /* if the carrier frequency quality is not good, wait some time */  
  while (1)
  {
    if (((NRF_RTC2->COUNTER - initial_timer) > max_CD_ms) || (NRF_RTC2->COUNTER < initial_timer) || (radio_rssi_measurement(frequency) > noise_threshold))
    {
      break;
    }
  }  
  
  nrf_delay_us(300);
  
  return(0);  
}

/********************************************************************************/
uint32_t wireless_sim_authentication_algorithm(uint8_t *user_password, uint8_t *system_key, 
         uint8_t *challenge_data, volatile uint8_t *response_data)
/*--------------------------------------------------------------------------------
| wireless SIM interface access channel authentication algorithm example
|
--------------------------------------------------------------------------------*/
{
  uint32_t key_length = KEY_LENGTH;
  uint32_t i;
  
  for (i=0; i<key_length; i++)
  {
    *(response_data + i) = (((*(user_password + i)) ^ (*(system_key + i))) ^ (*(challenge_data + i)));
  }
  
  return(0);
}

/*------------------------------------------------------------------------------*/
/*----------------------- CC channel coding ------------------------------------*/
/*------------------------------------------------------------------------------*/
#if (IF_DATA_CC_CODED || IF_CONNECTION_CC_CODED)
/***************************************************************************/
uint32_t encoder_halfrate (uint8_t *input_bit, uint32_t bit_length, 
             uint8_t initial_state, uint8_t *encoded_bit)
/* -------------------------------------------------------------------------- 
   cc encoder, 1/2 code rate
   memory_length = 3
   encoder_number = 2
   encoder0 {1, 0, 0, 1} 
   encoder1 {1, 1, 0, 1}   
   
	 INPUT --->
   input_bit:         the input bit sequence to be cc encoded, size of bit_length
   bit_length:        the length of bit sequence
   initial_state:     the cc encoder initial state 
   
	 <---OUTPUT
   encoded_bit:       the encoded bits with size of 2 * bit_length
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  uint8_t current_state;
  uint8_t output_bit0[16] = {0,  0,  0,  0,  1,  1,  1,  1,
                            0,  1,  0,  1,  1,  0,  1,  0};
  uint8_t output_bit1[16] = {1,  1,  1,  1,  0,  0,  0,  0,
                            1,  0,  1,  0,  0,  1,  0,  1};
  uint8_t to_state0[8] = {0,  2,  4,  6,  0,  2,  4,  6};
  uint8_t to_state1[8] = {1,  3,  5,  7,  1,  3,  5,  7};

  /** current state is the initial state **/
  current_state = initial_state;
    
  for(i = 0; i < bit_length; i++)
  {
    /** calculate output due to input bit 0 **/
    if(*(input_bit + i) == 0)
    {
      *(encoded_bit + 2 * i) = output_bit0[current_state];
      *(encoded_bit + 2 * i + 1) = output_bit0[8 + current_state];
      /** calculate the new state **/
      current_state = to_state0[current_state];
    }
    /** calculate output due to input bit 1 **/
    else
    {
      *(encoded_bit + 2 * i) = output_bit1[current_state];
      *(encoded_bit + 2 * i + 1) = output_bit1[8 + current_state];
      /** calculate the new state **/
      current_state = to_state1[current_state];
    }
  }
  
  return(0);
}

/***************************************************************************/
uint32_t decoder_hallfrate(uint8_t *input_receivebit, uint32_t bit_length,
       uint8_t initial_state, uint8_t *decoded_bit)
/* -------------------------------------------------------------------------- 
   cc decoder by Viterbi algorithm, 1/2 code rate
   memory_length = 3
   encoder_number = 2
   encoder0 {1, 0, 0, 1} 
   encoder1 {1, 1, 0, 1}   
   
	 INPUT --->
   input_receivebit: the received hard bit sequence to be decoded, size of 2 * bit_length
   bit_length:       the decoding  bit length
   initial_state:    the cc encoder initial state 
   
	 <---OUTPUT
   decoded_bit:       the decoded bits with size of bit_length
 * -------------------------------------------------------------------------- */
{
  /** loop control variable **/	
  uint32_t i;
  uint32_t j;	
  uint32_t k;	
  uint8_t previous_state[16] = 
          {0,  4,  0,  4,  1,  5,  1,  5,  2,  6,  2,  6,  3,  7,  3,  7};
  uint8_t encoding_bit0[16] = 
          {0,  0,  0,  1,  0,  0,  0,  1,  1,  1,  1,  0,  1,  1,  1,  0};
  uint8_t encoding_bit1[16] =
          {1,  1,  1,  0,  1,  1,  1,  0,  0,  0,  0,  1,  0,  0,  0,  1};                              

  uint8_t encoder_number = 2;
  uint8_t memory_length = 3;
  /** the encoder register state number: = 2 ^ memory_length **/    
  uint8_t states_number = 8;
  /** variables for trellis generation **/    
  uint8_t to_state0[8] = {0,  2,  4,  6,  0,  2,  4,  6};
  uint8_t to_state1[8] = {1,  3,  5,  7,  1,  3,  5,  7};
  /** variables for state store **/
  uint8_t *state_history;
  uint8_t *state_sequence;
 
  uint32_t accum_distance_metric[8] = {0, 0, 0, 0, 0, 0, 0, 0};  
  uint32_t accum_distance_metric_history[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint32_t tmp_metric_0;
  uint32_t tmp_metric_1;
  uint8_t tmp_state_0[8];
  uint8_t tmp_state_1[8];
  uint8_t tmp_number;    
    
  uint32_t min_value;
  uint32_t min_index;
  uint8_t depth_of_trellis;
  
  depth_of_trellis = bit_length;
  
  state_history = (uint8_t *)malloc(sizeof(uint8_t) * states_number *
                  (depth_of_trellis + 1));
  if (state_history == NULL)
  {
#if (IF_LOG_OUTPUT)    
    printf("memory allocation error in viterbi_decoding...\r\n");
#endif
    
    return(1); 
  }

  state_sequence = (uint8_t *)malloc(sizeof(uint8_t) * states_number *
                  (depth_of_trellis + 1));
  if (state_sequence == NULL)
  {
#if (IF_LOG_OUTPUT)    
    printf("memory allocation error in viterbi_decoding...\r\n");
#endif
    
    return(1); 
  }  
  	  
  for(i = 0; i < states_number * depth_of_trellis; i++)
  {
	  *(state_history + i) = 0;
	  *(state_sequence + i) = 0;
  }
  for(i = 0; i < states_number; i++)
  {
	  *(state_history + i) = initial_state;
  }
	   
  *(tmp_state_0 + 0) = initial_state; 
  for(i = 0; i < memory_length; i++)
  {
 	  tmp_number = (1 << i);	  	  
	  	  
		for(j = 0; j < tmp_number; j++)
		{ 
		  *(tmp_state_1 + 2 * j + 0) = *(to_state0 + (*(tmp_state_0 + j)));
      *(tmp_state_1 + 2 * j + 1) = * (to_state1 + (*(tmp_state_0 + j)));
		}
			  
		for(j = 0; j < tmp_number; j++)
		{ 
		  *(state_sequence + (i + 1) * states_number + (*(tmp_state_1 + 2 * j + 0))) = 
			  	      (*(tmp_state_1 + 2 * j + 0));
		  *(state_sequence + (i + 1) * states_number + (*(tmp_state_1 + 2 * j + 1))) = 
			  	      (*(tmp_state_1 + 2 * j + 1));
	  	      
	    for ( k = 0; k < i + 1 ; k++ )
	    {
	      *(state_sequence + k * states_number + (*(tmp_state_1 + 2 * j + 0))) = 
	      	   	       *(state_history + k * states_number + (*(tmp_state_0 + j)));
	      *(state_sequence + k * states_number + (*(tmp_state_1 + 2 * j + 1))) = 
	      	   	       *(state_history + k * states_number + (*(tmp_state_0 + j)));
	    }
	      	   
	    (*(accum_distance_metric + (*(tmp_state_1 + 2 * j + 0)))) = 
	      (*(accum_distance_metric_history + (*(tmp_state_0 + j)))) + 
	      distance_between_vector(encoding_bit0 + (*(tmp_state_0 + j)) * encoder_number, 
		    input_receivebit + i * encoder_number, encoder_number);    
			           
	    (*(accum_distance_metric + (*(tmp_state_1 + 2 * j + 1)))) = 
	      (*(accum_distance_metric_history + (*(tmp_state_0 + j)))) + 
	      distance_between_vector(encoding_bit1 + (*(tmp_state_0 + j)) * encoder_number, 
			  input_receivebit + i * encoder_number, encoder_number);         	   
		}
			         	  
		for(j = 0; j < tmp_number; j++)
		{          
		  *(accum_distance_metric_history + (*(tmp_state_1 + 2 * j + 0))) = 
			      *(accum_distance_metric + (*(tmp_state_1 + 2 * j + 0))); 
		  *(accum_distance_metric_history + (*(tmp_state_1 + 2 * j + 1))) = 
			      *(accum_distance_metric + (*(tmp_state_1 + 2 * j + 1))); 
      for(k = 0; k < i + 2 ; k++)
      {
      	*(state_history + k * states_number + (*(tmp_state_1 + 2 * j + 0))) = 
      	    *(state_sequence + k * states_number + (*(tmp_state_1 + 2 * j + 0)));
      	*(state_history + k * states_number + (*(tmp_state_1 + 2 * j + 1))) = 
      	    *(state_sequence + k * states_number + (*(tmp_state_1 + 2 * j + 1)));
      }
	  }
			  
		for(j = 0; j < tmp_number; j++)
		{ 
		  *(tmp_state_0 + 2 * j + 0) = *(tmp_state_1 + 2 * j + 0);
		  *(tmp_state_0 + 2 * j + 1) = *(tmp_state_1 + 2 * j + 1);
		}
  }
		
  for(i = memory_length; i < depth_of_trellis; i++)
  {
		for(j = 0; j < states_number / 2; j++)
		{
		  *(state_sequence + (i + 1) * states_number + 2 * j + 0) =  2 * j + 0;
		  tmp_metric_0 = distance_between_vector(encoding_bit0 + 
		                (*(previous_state + 4 * j + 0)) * encoder_number, 
			              input_receivebit + i * encoder_number, encoder_number) + 
			              (*(accum_distance_metric_history + 
			              (*(previous_state + 4 * j + 0))));
		  tmp_metric_1 = distance_between_vector(encoding_bit0 + 
		                (*(previous_state + 4 * j + 1)) * encoder_number, 
			              input_receivebit + i * encoder_number, encoder_number) +
			              (*(accum_distance_metric_history + 
			              (*(previous_state + 4 * j + 1))));
            
		  if(tmp_metric_0 < tmp_metric_1)
		  {
			  *(accum_distance_metric + 2 * j + 0) = tmp_metric_0;
			  for ( k = 0; k < i + 1 ; k++ )
			  {
			    *(state_sequence + k * states_number + 2 * j + 0) = 
			        *(state_history + k * states_number + (*(previous_state + 4 * j + 0)));
			  }
		  }
		  else
		  {
			  *(accum_distance_metric + 2 * j + 0) = tmp_metric_1;
			  for(k = 0; k < i + 1 ; k++)
			  {
			    *(state_sequence + k * states_number + 2 * j + 0) = 
			        *(state_history + k * states_number + (*(previous_state + 4 * j + 1)));
			  }
		  }        
			              					  
		  *(state_sequence + (i + 1) * states_number + 2 * j + 1) =  2 * j + 1;
      tmp_metric_0 = distance_between_vector(encoding_bit1 + 
              (*(previous_state + 4 * j + 2)) * encoder_number, 
			        input_receivebit + i * encoder_number, encoder_number) + 
			        (*(accum_distance_metric_history + (*(previous_state + 4 * j + 2))));
		  tmp_metric_1 = distance_between_vector(encoding_bit1 + 
		          (*(previous_state + 4 * j + 3)) * encoder_number, 
			        input_receivebit + i * encoder_number, encoder_number) +
			        (*(accum_distance_metric_history + (*(previous_state + 4 * j + 3))));
		  if(tmp_metric_0 < tmp_metric_1)
		  {
			  *(accum_distance_metric + 2 * j + 1) = tmp_metric_0;
			  for ( k = 0; k < i + 1 ; k++ )
			  {
			    *(state_sequence + k * states_number + 2 * j + 1) = 
			        *(state_history + k * states_number + (*(previous_state + 4 * j + 2)));
			  }
		  }
		  else
		  {
			  *(accum_distance_metric + 2 * j + 1) = tmp_metric_1;
			  for(k = 0; k < i + 1 ; k++)
			  {
			    *(state_sequence + k * states_number + 2 * j + 1) = 
			        *(state_history + k * states_number + (*(previous_state + 4 * j + 3)));
			  }
			      
		  }  
		}
			      
		for ( j = 0; j < states_number; j++ )
		{          
		  *(accum_distance_metric_history + j) = *(accum_distance_metric + j); 
      for(k = 0; k < i + 2 ; k++)
      {
      	*(state_history + k * states_number + j) = 
      	    	       *(state_sequence + k * states_number + j);
      }
	  }
  }
	  
  min_index_value(accum_distance_metric_history, states_number, 
	                /**/&min_value, &min_index);
	  	  
  for(i = 0; i < depth_of_trellis; i++)
  {
	  if((*(to_state0 + (*(state_sequence + i * states_number + min_index)))) == 
	  	  	   (*(state_sequence + (i + 1) * states_number + min_index)))
	  {
	    *(decoded_bit + i) = 0;
	  }
	  else if((*(to_state1 + (*(state_sequence + i * states_number + min_index)))) == 
	  	  	   (*(state_sequence + (i + 1) * states_number + min_index)))
	  {
	    *(decoded_bit + i) = 1;
	  }
	  else 
	  {
#if (IF_LOG_OUTPUT)    
      printf("There is something wrong in viterbi_decoding..., i = %ld\r\n", i);
#endif
	    return(1);
	  }
  } 
	  	   
  free(state_history);
  free(state_sequence);

  return(0);
}

/***************************************************************************/
uint32_t int2bit(uint8_t state, uint8_t bit_length, /**/uint8_t *bit_array)
/* -------------------------------------------------------------------------- 
 * Map the state integer number to a bit array
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
	
	for(i = 0; i < bit_length; i++)
  {
    *(bit_array + i) = ((state >> i) & 1);
  }
  
  return(0);
}

/***************************************************************************/
uint32_t bit2int(uint8_t *bit_array, uint8_t bit_length, /**/uint8_t *state)
/* -------------------------------------------------------------------------- 
 * Map a bit array to the corresponding integer state number
 * --------------------------------------------------------------------------
 */
{
  uint32_t i;
    
	*state = 0;
	for (i = 0; i < bit_length; i++)
  {
    (*state) |= (*(bit_array + i) << i);
  }
  
  return(0);
}

/***************************************************************************/
uint32_t char2bit_sequence(uint8_t *char_sequence, uint8_t char_length, 
                    uint8_t *bit_sequence)
/* -------------------------------------------------------------------------- 
 * Map the char (uint8_t) sequence to bit sequence
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  
  for(i = 0; i < char_length; i++)
  {
    int2bit(*(char_sequence + i), 8, bit_sequence + i * 8);
  }
  
  return(0);
}

/***************************************************************************/
uint32_t bit2char_sequence(uint8_t *bit_sequence, uint8_t char_length, 
                    uint8_t *char_sequence)
/* -------------------------------------------------------------------------- 
 * Map the bit sequence to char (uint8_t) sequence
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  
  for(i = 0; i < char_length; i++)
  {
	  bit2int(bit_sequence + i * 8, 8, char_sequence + i);
  }
  
  return(0);
}

/***************************************************************************/
uint32_t distance_between_vector(uint8_t *input1_bit, uint8_t *input2_bit,
                   uint8_t bit_length)
/* -------------------------------------------------------------------------- 
 * Euclid distance between 2 bit vectors
 * -------------------------------------------------------------------------- */
{
  uint32_t tmp = 0;
  uint32_t i;
    
  tmp = 0;
  for(i = 0; i < bit_length; i++)
  {
    if(*(input1_bit + i) != *(input2_bit + i))
    {
      tmp++;
    }
  }     
    
  return(tmp);   
}

/***************************************************************************/
uint32_t min_index_value(uint32_t *vector_in, uint32_t vector_length,
     /**/ uint32_t *min_value, uint32_t *min_index)
/* -------------------------------------------------------------------------- 
 * Minimal value and its index in the input vector  
 *
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
    
  *min_index = 0;
  *min_value = *(vector_in + 0);
    
  for(i = 0; i < vector_length; i++)
  {
    if(*(vector_in + i ) < *min_value)
    {
      *min_value = *(vector_in + i);
      *min_index = i;
    }
  }
  
  return(0);
}

/***************************************************************************/
uint32_t ccencoder_byte_sequence_32(uint8_t *input_bytes, uint8_t encode_block_index,
         uint8_t *encoded_bytes)
/* -------------------------------------------------------------------------- 
 *  CC encoder for length of byte 32	(256 bits)
 *  encode_block_index --> cc encoder byte length for each encoding block = 
 *			            (1 << encode_block_index ) 			 
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  uint8_t bytes_length = 32; 
  uint8_t bit_sequence[256];
  uint8_t bit_coded[512];
  uint8_t bit_coded_interleave[512];
  
  /* byte to bits map */
  char2bit_sequence(input_bytes, bytes_length, bit_sequence);
  /* 1/2 cc encoding for each bytes */
  for (i=0; i<(bytes_length >> encode_block_index); i++)
  {
  	encoder_halfrate (bit_sequence + i * (8 << encode_block_index), 
	        (8 << encode_block_index), 0, bit_coded + i * (16 << encode_block_index));
  }
  /* bit interleave */
  for (i=0; i<256; i++)
  {
  	*(bit_coded_interleave + i      ) = *(bit_coded + INTERLEAVE_PATTERN[i] * 2 + 0);
  	*(bit_coded_interleave + 256 + i) = *(bit_coded + INTERLEAVE_PATTERN[i] * 2 + 1);
  }
  /* bits to byte map */
  bit2char_sequence(bit_coded_interleave, bytes_length << 1, encoded_bytes);
  
  return(0);								 	
}

/***************************************************************************/
uint32_t ccdecoder_byte_sequence_32(uint8_t *input_bytes, uint8_t encode_block_index,
         uint8_t *decoded_bytes)
/* -------------------------------------------------------------------------- 
 *  CC decoder for length of byte 32	(256 bits)
 *  encode_block_index --> cc encoder byte length for each encoding block = 
 *			            (1 << encode_block_index ) 			 
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  uint8_t bytes_length = 32; 
  uint8_t bit_sequence[256];
  uint8_t bit_coded[512];
  uint8_t bit_coded_interleave[512];
   
  /* byte to bit map */
  char2bit_sequence(input_bytes, 64, bit_coded_interleave);
  /* bit de-interleave */
  for (i=0; i<256; i++)
  {
  	*(bit_coded + INTERLEAVE_PATTERN[i] * 2    ) = *(bit_coded_interleave +       i);
  	*(bit_coded + INTERLEAVE_PATTERN[i] * 2 + 1) = *(bit_coded_interleave + 256 + i);
   }
  /* 1/2 cc decode for each bytes */
  for (i=0; i<(bytes_length >> encode_block_index); i++)
  {
 	decoder_hallfrate(bit_coded + i * (16 << encode_block_index), 
	         (8 << encode_block_index), 0, bit_sequence + i * (8 << encode_block_index));
  }
  /* bits to byte map */
  bit2char_sequence(bit_sequence, bytes_length, decoded_bytes);
  
  return(0);								 	
}


/***************************************************************************/
uint32_t ccencoder_byte_sequence_64(uint8_t *input_bytes, uint8_t encode_block_index,
         uint8_t *encoded_bytes)
/* -------------------------------------------------------------------------- 
 *  CC encoder for length of byte 64	(512 bits)
 *  encode_block_index --> cc encoder byte length for each encoding block = 
 *			            (1 << encode_block_index ) 			 
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  uint8_t bytes_length = 64; 
  uint8_t bit_sequence[512];
  uint8_t bit_coded[1024];
  uint8_t bit_coded_interleave[1024];

  /* byte to bits map */
  char2bit_sequence(input_bytes, bytes_length, bit_sequence);
  /* 1/2 cc encoding for each bytes */
  for (i=0; i<(bytes_length >> encode_block_index); i++)
  {
  	encoder_halfrate (bit_sequence + i * (8 << encode_block_index), 
	        (8 << encode_block_index), 0, bit_coded + i * (16 << encode_block_index));
  }
  /* bit interleave */
  for (i=0; i<256; i++)
  {
  	*(bit_coded_interleave + i      ) = *(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 0);
  	*(bit_coded_interleave + 256 + i) = *(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 1);
  	*(bit_coded_interleave + 512 + i) = *(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 2);
  	*(bit_coded_interleave + 768 + i) = *(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 3);
  }
  /* bits to byte map */
  bit2char_sequence(bit_coded_interleave, bytes_length << 1, encoded_bytes);
  
  return(0);								 	
}

/***************************************************************************/
uint32_t ccdecoder_byte_sequence_64(uint8_t *input_bytes, uint8_t encode_block_index,
         uint8_t *decoded_bytes)
/* -------------------------------------------------------------------------- 
 *  CC decoder for length of byte 64	(512 bits)
 *  encode_block_index --> cc encoder byte length for each encoding block = 
 *			            (1 << encode_block_index ) 			 
 * -------------------------------------------------------------------------- */
{
  uint32_t i;
  uint8_t bytes_length = 64; 
  uint8_t bit_sequence[512];
  uint8_t bit_coded[1024];
  uint8_t bit_coded_interleave[1024];
  
  /* byte to bit map */
  char2bit_sequence(input_bytes, bytes_length << 1, bit_coded_interleave);
  /* bit de-interleave */
  for (i=0; i<256; i++)
  {
  	*(bit_coded + INTERLEAVE_PATTERN[i] * 4    ) = *(bit_coded_interleave +       i);
  	*(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 1) = *(bit_coded_interleave + 256 + i);
  	*(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 2) = *(bit_coded_interleave + 512 + i);
  	*(bit_coded + INTERLEAVE_PATTERN[i] * 4 + 3) = *(bit_coded_interleave + 768 + i);
  }
  /* 1/2 cc decode for each bytes */
  for (i=0; i<(bytes_length >> encode_block_index); i++)
  {
 	  decoder_hallfrate(bit_coded + i * (16 << encode_block_index), 
	         (8 << encode_block_index), 0, bit_sequence + i * (8 << encode_block_index));
  }
  /* bits to byte map */
  bit2char_sequence(bit_sequence, bytes_length, decoded_bytes);
  
  return(0);								 	
}
#endif


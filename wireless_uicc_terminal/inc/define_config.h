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
#ifndef DEFINE_CONFIG_H__
#define DEFINE_CONFIG_H__

/* Custom borad pin profile definition */
#define PIN_PROFILE                          1

/* if print out the log information of USIM-PHONE interface, for debug purpose
|  1: log information printed
|  0: not printed */
#define IF_LOG_OUTPUT                        1

/* if USIM binary EF data is updated to USIM card, UICC command 0xd6
|  1: binary EF updated to USIM card
|  0: not writed to USIM card */
#define IF_USIM_BINARY_UPDATE                1

/* if convolutional code is used for data transmission link
|  1: cc coding is used
|  0: cc coding not used */
#define IF_DATA_CC_CODED                     1

/* if convolutional code is used for connection request random access link
|  1: cc coding is used
|  0: cc coding not used */
#define IF_CONNECTION_CC_CODED               0

/* if softdevice BLE stack code is complied in the final target
|  1: softdevice is compiled in the target
|  0: not compiled
|  it should be the same as the value in the Makefile */
#define IF_SOFTDEIVE_USED                    1

/* nRF GIOP pin number for UICC-terminal interface, used for null pin define (1 USIM card case)
|  PIN_VCC_SIM1 == 0xFF: only 1 USIM
|  PIN_VCC_SIM1 != 0xFF: 2 USIMs             */
#define PIN_NULL               0xFF

#if (PIN_PROFILE == 0)
/* wireless USIM card, no LED */
  #define PIN_DATA_PHONE       12
  #define PIN_RESET_PHONE      29
  #define PIN_CLOCK_PHONE      30

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        28
  #define PIN_DATA_SIM0         18
  #define PIN_RESET_SIM0        25
  #define PIN_VCC_SIM0          2

  /* USIM1 card reader PIN define */
  // #define PIN_CLOCK_SIM1        14
  // #define PIN_DATA_SIM1         13
  // #define PIN_RESET_SIM1        17
  // #define PIN_VCC_SIM1          16
  #define PIN_VCC_SIM1          PIN_NULL

#elif (PIN_PROFILE == 1)
/* EV board */
  #define PIN_LED_BLUE          24
  #define PIN_LED_GREEN         23
  #define PIN_LED_YELLOW        22
  #define PIN_LED_ORANGE        18
  #define PIN_LED_RED           20
  #define PIN_LED_WHITE         19

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        28
  #define PIN_DATA_SIM0         29
  #define PIN_RESET_SIM0        25
  #define PIN_VCC_SIM0          2

  /* USIM1 card reader PIN define */
  #define PIN_CLOCK_SIM1        14
  #define PIN_DATA_SIM1         13
  #define PIN_RESET_SIM1        17
  #define PIN_VCC_SIM1          16
  // #define PIN_VCC_SIM1          PIN_NULL

  /* sim-phone interface PIN define*/
  #define PIN_DATA_PHONE        11
  #define PIN_RESET_PHONE       3
  #define PIN_CLOCK_PHONE       4

  /* UART pin define */
  #define RX_PIN_NUMBER        8
  #define TX_PIN_NUMBER        6
  #define CTS_PIN_NUMBER       7
  #define RTS_PIN_NUMBER       5

#elif (PIN_PROFILE == 2)
/* Only 1 blue LED, double SIM connector */
/* 20170620 */
  #define PIN_LED_BLUE          21

  /* SIM-phone interface PIN define, iPhone */
  #define PIN_DATA_PHONE        18
  #define PIN_RESET_PHONE       1
  #define PIN_CLOCK_PHONE       12
  // /* SIM-phone interface PIN define, HWwatch2 */
  // #define PIN_DATA_PHONE        18
  // #define PIN_RESET_PHONE       12
  // #define PIN_CLOCK_PHONE       1

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        15
  #define PIN_DATA_SIM0         16
  #define PIN_RESET_SIM0        8
  #define PIN_VCC_SIM0          6

  /* USIM1 card reader PIN define */
  #define PIN_CLOCK_SIM1        5
  #define PIN_DATA_SIM1         14
  #define PIN_RESET_SIM1        0
  #define PIN_VCC_SIM1          30

#elif (PIN_PROFILE == 3)
/* SIM-phone interface PIN define, iPhone double USIM, 2 LED */
/* 20170722 */
  #define PIN_LED_BLUE          21
  #define PIN_LED_GREEN         18

  #define PIN_DATA_PHONE        16
  #define PIN_RESET_PHONE       5
  #define PIN_CLOCK_PHONE       12

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        6
  #define PIN_DATA_SIM0         29
  #define PIN_RESET_SIM0        0
  #define PIN_VCC_SIM0          1

  /* USIM1 card reader PIN define */
  #define PIN_CLOCK_SIM1        14
  #define PIN_DATA_SIM1         30
  #define PIN_RESET_SIM1        15
  #define PIN_VCC_SIM1          8

#elif (PIN_PROFILE == 4)
/* SIM-phone interface PIN define, iPhone single USIM, 2 LED */
/* 20170722 */
  #define PIN_LED_BLUE          21
  #define PIN_LED_GREEN         18

  #define PIN_DATA_PHONE        14
  #define PIN_RESET_PHONE       5
  #define PIN_CLOCK_PHONE       16

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        1
  #define PIN_DATA_SIM0         0
  #define PIN_RESET_SIM0        8
  #define PIN_VCC_SIM0          15

  /* USIM1 card reader PIN define */
  // #define PIN_CLOCK_SIM1        6
  // #define PIN_DATA_SIM1         30
  // #define PIN_RESET_SIM1        29
  // #define PIN_VCC_SIM1          12
  #define PIN_VCC_SIM1          PIN_NULL

#elif (PIN_PROFILE == 5)
/* PCA100036 board */
  #define PIN_LED_BLUE          24
  #define PIN_LED_GREEN         23
  #define PIN_LED_RED           20
  #define PIN_LED_WHITE         19

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        28
  #define PIN_DATA_SIM0         29
  #define PIN_RESET_SIM0        25
  #define PIN_VCC_SIM0          2

  /* USIM1 card reader PIN define */
  // #define PIN_CLOCK_SIM1        14
  // #define PIN_DATA_SIM1         13
  // #define PIN_RESET_SIM1        17
  // #define PIN_VCC_SIM1          16
  #define PIN_VCC_SIM1          PIN_NULL

  /* sim-phone interface PIN define*/
  #define PIN_DATA_PHONE        11
  #define PIN_RESET_PHONE       3
  #define PIN_CLOCK_PHONE       4

  /* UART pin define */
  #define RX_PIN_NUMBER        8
  #define TX_PIN_NUMBER        6
  #define CTS_PIN_NUMBER       7
  #define RTS_PIN_NUMBER       5

#elif (PIN_PROFILE == 6)
/* SIM-phone interface PIN define, HWwatch single USIM, 2 LED */
/* 20170722 */
  #define PIN_LED_BLUE          21
  #define PIN_LED_GREEN         18

  #define PIN_DATA_PHONE        16
  #define PIN_RESET_PHONE       5
  #define PIN_CLOCK_PHONE       14

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        1
  #define PIN_DATA_SIM0         0
  #define PIN_RESET_SIM0        8
  #define PIN_VCC_SIM0          15

  /* USIM1 card reader PIN define */
  // #define PIN_CLOCK_SIM1        6
  // #define PIN_DATA_SIM1         30
  // #define PIN_RESET_SIM1        29
  // #define PIN_VCC_SIM1          12
  #define PIN_VCC_SIM1          PIN_NULL

#elif (PIN_PROFILE == 7)
/* SIM-phone interface PIN define, iPhone single USIM, 4 LED */
/* 20171227 */
  #define PIN_LED_BLUE          15
  #define PIN_LED_GREEN         16
  #define PIN_LED_WHITE         18
  #define PIN_LED_RED           21

  #define PIN_DATA_PHONE        8
  #define PIN_RESET_PHONE       6
  #define PIN_CLOCK_PHONE       14

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        1
  #define PIN_DATA_SIM0         0
  #define PIN_RESET_SIM0        5
  #define PIN_VCC_SIM0          12

  /* USIM1 card reader PIN define */
  #define PIN_VCC_SIM1          PIN_NULL

#elif (PIN_PROFILE == 8)
/* SIM-phone interface PIN define, HWwatch single USIM, 4 LED */
/* 20171227 */
  #define PIN_LED_BLUE          15
  #define PIN_LED_GREEN         16
  #define PIN_LED_WHITE         18
  #define PIN_LED_RED           21

  #define PIN_DATA_PHONE        5
  #define PIN_RESET_PHONE       8
  #define PIN_CLOCK_PHONE       1

  /* USIM0 card reader PIN define */
  #define PIN_CLOCK_SIM0        0
  #define PIN_DATA_SIM0         14
  #define PIN_RESET_SIM0        6
  #define PIN_VCC_SIM0          12

  /* USIM1 card reader PIN define */
  // #define PIN_CLOCK_SIM1        6
  // #define PIN_DATA_SIM1         30
  // #define PIN_RESET_SIM1        29
  // #define PIN_VCC_SIM1          12
  #define PIN_VCC_SIM1          PIN_NULL

#else
  #error "wrong PIN_PROFILE ......"
#endif

/* SIM card reader ETU define, in 16M timer ticks
** clock signal is 4MHz for SIM interface */
/* 1 / 4 * 372 * 16 = 1488, initial ETU is 372 clock cycle */
#define ETU_TICKS_SIM_INITIAL             1488

/* write data to flash data size, in word. save the data in P_UINT8_FLASH_DATA_RAM_BUFFER global variable */
#define FLASH_WRITE_DATA_SIZE_IN_WORD     256

/* time out for phone command read, in ms */
#define PHONE_COMMAND_TIME_OUT_MS         50

/* watchdog timeout period, in 250ms */
#define WATCH_DOG_TIME_OUT_S              4

/* bytes length for initial USAT menu setting */
#define USAT_BYTE_LENGTH_BACK_INIT        141

/* time for 16 bytes random number generation, in ms */
#define RANDOM_BYTE_NUMBER_TIME_OUT_MS    6

/*--------------------------------------------------------------------------------*/
/* wireless interface protocol related parameter config */
/*--------------------------------------------------------------------------------*/
/* RTC2 interrupt event time interval, in ms */
#define RTC2_COMPARE0_EVENT_INTERVAL_MS           100
#define COMPARE0_EVENT_POSTPONE_USIM_MS           400

/* time interval for phone to transmit a wireless connection state package, in RTC2_COMPARE0_EVENT_INTERVAL_MS */
// #define CONNECT_STATE_TIME_PHONE      40
#define CONNECT_STATE_TIME_PHONE             10

/* time for phone do not receive feedback confirm message for wireless connection request from watch,
*  in CONNECT_STATE_TIME * CONNECT_STATE_TIME_PHONE * 500ms, phone assume there is no connection between SIM-phone */
#define PHONE_DISCONNECT_TIME                30

/* time for SIM do not receive wireless connection request from phone, in CONNECT_STATE_TIME * 200ms,
|  SIM assume there is no connection between SIM-phone */
#define SIM_DISCONNECT_TIME                  300

/* password bytes length for random access request */
#define KEY_LENGTH                           16

/* parameters for access channel, they are pre-defined between Tx and Rx */
#define FREQUENCY_CARRIER_ACCESS             76UL

/* radio transmission mode
|  1: 1Mbps
|  2: 2Mpbs
|  */
#define RADIO_MODE                           1

/* radio transmission power for data transmissiom
|  0: -30dBm for nRF51, -40dBm for nRF52
|  1: -20dBm
|  2: -16dBm
|  3: -12dBm
|  4: -8dBm
|  5: -4dBm
|  6: 0dBm
|  7: 4dBm
|  8: 3dBm, nRF52 only */
#define RADIO_TX_POWER_DATA            7
/* Tx power level for ACK/NACK feedback */
#define POWER_LEVEL_ACK                7
/* Tx power level for phone connection request */
#define POWER_LEVEL_CONNECTION         7

/* time resolution is 1us */
/* time duration for SIM to listen command from phone in us */
#if (IF_CONNECTION_CC_CODED)
  #define SIM_LISTEN_TIME_MS             15
#else
  #define SIM_LISTEN_TIME_MS             5
#endif
/* time duration for phone to send out random access request message */
#if (IF_CONNECTION_CC_CODED)
  #define PHONE_REQUEST_TX_TIME_MS       3
#else
  #define PHONE_REQUEST_TX_TIME_MS       1
#endif
/* time duration for phone to wait confirm message of random access from SIM */
#if (IF_CONNECTION_CC_CODED)
  #define PHONE_REQUEST_WAIT_TIME_MS     8
#else
  #define PHONE_REQUEST_WAIT_TIME_MS     2
#endif
/* time resolution is 1ms */
/* max time for a data package transmission, may be several transmission try */
#define DATA_TX_TIME_MS                980
/* max waiting time for a data package receiving */
#define DATA_RX_TIME_MS                980

/* time interval for each data package transmission try, if no ACK confirm message received, in ms */
#define DATA_TX_TIME_INTERVAL_MS       3
/* ACK receiving time out */
#define ACK_RX_TIME_MS                 3

/* time resolution is 1ms */
/* maximal carrier frequency search time to find out good carrier, in ms */
#define CARRIER_FREQ_SEARCH            10
/* carrier frequency bin search threshold */
#if (RADIO_MODE == 0)
  #define CARRIER_FREQ_THRE            100
#elif (RADIO_MODE == 1)
  #define CARRIER_FREQ_THRE            94
#else
  #define CARRIER_FREQ_THRE            90
#endif

/* maximal carrier frequency colliding detection time, in ms */
#define MAX_CD_TIME_MS                 10
/* carrier colliding detection, background signal level in -dBm */
/* Set the threshold for good carrier frequency search, for radio mode 2Mbps, the minimal measured
|  RSSI is -93dBm, for mode 250Kbps and 1Mbps, the minimal measured RSSI is -102dBm */
#define TX_RSSI_THRED                  CARRIER_FREQ_THRE


/* */
/*--------------------------------------------------------------------------------*/
/* conncetion request information byte number */
#define CONNECTION_REQUEST_BYTE       64
#if (IF_DATA_CC_CODED || IF_CONNECTION_CC_CODED)
  /* cc encoder byte number for each coding block = (1 << ENCODE_BLOCK_INDEX ) */
  #define ENCODE_BLOCK_INDEX          0
#endif

/* interrupt priority for events */
#define GPIOTE_IRQ_PRIORITY         2
#define SWI3_EGU3_IRQ_PRIORITY      5
#define RADIO_LINK_IRQ_PRIORITY     7

#define RTC2          RTC2

#endif


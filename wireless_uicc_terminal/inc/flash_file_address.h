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
#ifndef FLASH_FILE_ADDRESS_H__
#define FLASH_FILE_ADDRESS_H__

#include "define_config.h"
#include "nrf52.h"

/* flash page size: 4K for nRF52 */
#define FLASH_PAGE_SIZE                          (NRF_FICR->CODEPAGESIZE)
/* flash page number: 128 for nRF52_xxaa */
#define FLASH_PAGE_NUMBER                        (NRF_FICR->CODESIZE)

/* start page number for flash user data */
/* 2 pages for FDS defined in sdk_config.h */
/* 2 pages for user config setting and USIM EF data */
#define EF_FILES_START_PAGE                      (FLASH_PAGE_NUMBER - 2 - (FDS_VIRTUAL_PAGES * FDS_VIRTUAL_PAGE_SIZE * 4 / FLASH_PAGE_SIZE))
// #define EF_FILES_START_PAGE                      (FLASH_PAGE_NUMBER - 4)

/*------------------------- page offset 0, data backup page ----------------------------*/
#define BACKUP_DATA_USER_CONFIG_FLASH_ADDR       (FLASH_PAGE_SIZE * EF_FILES_START_PAGE)

/*--------------- page offset 1, user config and USIM file data page -------------------*/
/* byte offset map:
| 0 - 255: user configures
| 256 - 639: USIM0 EF data
| 640 - 1023: USIM1 EF data
*/
#define USER_CONFIG_OFFSET_BYTES             0
#define USIM0_EF_OFFSET_BYTES                256
#define USIM1_EF_OFFSET_BYTES                640
/* offset USER_CONFIG_OFFSET_BYTES: user setting configurations 
| byte 1 ~ 16: user password for wireless SIM connection authentication
| byte 17 ~ 20: wireless SIM authentication flag, 1: authentication on, 0: authentication off
*/
#define USER_CONFIG_FLASH_ADDR       (FLASH_PAGE_SIZE * (EF_FILES_START_PAGE + 1) + USER_CONFIG_OFFSET_BYTES)
/* offset USIM0_EF_OFFSET_BYTES: mandatory EF files data for USIM0
| byte 1 ~ ALL_USIM_EF_SIZE: mandatory USIM0 EF data
*/
#define USIM0_EF_FLASH_ADDR          (FLASH_PAGE_SIZE * (EF_FILES_START_PAGE + 1) + USIM0_EF_OFFSET_BYTES)
/* offset USIM1_EF_OFFSET_BYTES: mandatory EF files data for USIM1
| byte 1 ~ ALL_USIM_EF_SIZE: mandatory USIM1 EF data
*/
#define USIM1_EF_FLASH_ADDR          (FLASH_PAGE_SIZE * (EF_FILES_START_PAGE + 1) + USIM1_EF_OFFSET_BYTES)

/* user configures byte settings offset map:
|
*/
/* size: 16 bytes */
#define SMART_USIM_DEVICE_NAME_OFFSET_IN_CONFIG                 (USER_CONFIG_OFFSET_BYTES + 0)
/* size: 16 bytes */
#define USER_PASSWORD_OFFSET_IN_CONFIG                          (USER_CONFIG_OFFSET_BYTES + 16)
/* size: 16 bytes */
#define SYSTEM_KEY_OFFSET_IN_CONFIG                             (USER_CONFIG_OFFSET_BYTES + 32)
/* size: 16 bytes */
#define AUTH_CHALL_DATA_OFFSET_IN_CONFIG                        (USER_CONFIG_OFFSET_BYTES + 48)
/* size: 16 bytes */
#define LED_PATTERN_OFFSET_IN_CONFIG                            (USER_CONFIG_OFFSET_BYTES + 64)
/* size: 1 byte */
#define DEVICE_LOG_ADD_OFFSET_IN_CONFIG                         (USER_CONFIG_OFFSET_BYTES + 80)
/* size: 1 byte */
#define WIRELESS_SIM_ON_OFF_OFFSET_IN_CONFIG                    (USER_CONFIG_OFFSET_BYTES + 81)
/* size: 1 byte */
#define USIM_SERVER_CALL_OFFSET_IN_CONFIG                       (USER_CONFIG_OFFSET_BYTES + 82)
/* size: 1 byte */
#define BLE_ON_OFF_OFFSET_IN_CONFIG                             (USER_CONFIG_OFFSET_BYTES + 83)
/* size: 1 byte */
#define BLE_BOND_DELETE_OFFSET_IN_CONFIG                        (USER_CONFIG_OFFSET_BYTES + 84)
/* size: 1 byte */
#define WIRELESS_USIM_WORK_MODE_OFFSET_IN_CONFIG                (USER_CONFIG_OFFSET_BYTES + 85)
/* size: 3 byte */
#define USIM_CLK_FREQ_OFFSET_IN_CONFIG                          (USER_CONFIG_OFFSET_BYTES + 86)
/* size: 1 byte */
#define DEFAULT_USED_USIM_OFFSET_IN_CONFIG                      (USER_CONFIG_OFFSET_BYTES + 89)
/* size: 1 byte */
#define USIM_DATA_MODE_OFFSET_IN_CONFIG                         (USER_CONFIG_OFFSET_BYTES + 90)
/* size: 1 byte */
#define ICCID_2FE2_DATA_MODE_OFFSET_IN_CONFIG                   (USER_CONFIG_OFFSET_BYTES + 91)
/* size: 1 byte */
#define AUTHEN_0x88_BLE_ON_OFF_OFFSET_IN_CONFIG                 (USER_CONFIG_OFFSET_BYTES + 92)

/* total bytes number of user config */
#define USER_CONFIG_ALL_BYTES_LENGTH                            93                        

/*---------------------- user config flash data address -----------------------*/
/* smart wireless USIM name for BLE device: 4 words = 16 bytes */
#define SMART_USIM_DEVICE_NAME_FLASH_ADDR      (USER_CONFIG_FLASH_ADDR + SMART_USIM_DEVICE_NAME_OFFSET_IN_CONFIG)
/* user password: 4 words = 16 bytes*/
#define USER_PASSWORD_FLASH_ADDR               (USER_CONFIG_FLASH_ADDR + USER_PASSWORD_OFFSET_IN_CONFIG)
/* system key: 4 words = 16 bytes */
#define SYSTEM_KEY_FLASH_ADDR                  (USER_CONFIG_FLASH_ADDR + SYSTEM_KEY_OFFSET_IN_CONFIG)
/* LED indicator, 16bytes. 1: LED on, 0: LED off */
#define LED_PATTERN_FLASH_ADDR                 (USER_CONFIG_FLASH_ADDR + LED_PATTERN_OFFSET_IN_CONFIG)
/* logical address: 1 byte */
#define DEVICE_LOG_ADD_FLASH_ADDR              (USER_CONFIG_FLASH_ADDR + DEVICE_LOG_ADD_OFFSET_IN_CONFIG)
/* wireless SIM air interface on/off 
|  0: wireless SIM off 
|  1: wireless SIM on */       
#define WIRELESS_SIM_ON_OFF_FLASH_ADDR          (USER_CONFIG_FLASH_ADDR + WIRELESS_SIM_ON_OFF_OFFSET_IN_CONFIG)
/* USIM server side call On/Off flag: 1 word = 4 bytes */
/* 0: USIM server side call off */
/* 1: USIM server side call on */
#define USIM_SERVER_CALL_ON_OFF_FLASH_ADDR      (USER_CONFIG_FLASH_ADDR + USIM_SERVER_CALL_OFFSET_IN_CONFIG)
/* BLE on/off indicator, 1: BLE on, 0: BLE off */
#define BLE_STATUS_ON_OFF_FLASH_ADDR            (USER_CONFIG_FLASH_ADDR + BLE_ON_OFF_OFFSET_IN_CONFIG)
/* BLE bond information erase flag, 1: delete, 0: not delete */
#define BLE_BOND_DEL_STATUS_FLASH_ADDR          (USER_CONFIG_FLASH_ADDR + BLE_BOND_DELETE_OFFSET_IN_CONFIG)
/* wireless USIM mode
0: fixed wireless USIM client mode
1: wireless USIM auto mode, the USIM card presnece detection is performed, 
   if there ther are USIM cards, working at server mode, 
   if there is no USIM card, swith to client mode */
#define WIRELESS_USIM_WORK_MODE_FLASH_ADDR      (USER_CONFIG_FLASH_ADDR + WIRELESS_USIM_WORK_MODE_OFFSET_IN_CONFIG)
/* USIM clock frequency flag, 
|  byte 0, 1: 3.25MHz, 2: 3.84MHz, others: random selection between 3.25MHz and 3.84MHz
|  byte 1, 0: no updated, 1: updated
|  byte 2, ETU ticks byte, 0x94/0x95/0x96 */
#define PHONE_USIM_CLK_FREQ_FLASH_ADDR          (USER_CONFIG_FLASH_ADDR + USIM_CLK_FREQ_OFFSET_IN_CONFIG)
/* the default used USIM, 0: USIM0, 1: USIM1 */
#define DEFAULT_USED_USIM_FLASH_ADDR            (USER_CONFIG_FLASH_ADDR + DEFAULT_USED_USIM_OFFSET_IN_CONFIG)
/* USIM data read mode when phone power on, 4bytes. 
1: read ICCID data and check it with the data in flash, 
0: skip read ICCID, only check if the flach data is available */
#define USIM_DATA_MODE_FLASH_ADDR                 (USER_CONFIG_FLASH_ADDR + USIM_DATA_MODE_OFFSET_IN_CONFIG)
/* ICCID data mode
0: read the 2FE2 data from USIM card, from USIM0 and USIM1 respectively
1: both USIM0 and USIM1 use the same 2FE2 data, read from USIM0
2: both USIM0 and USIM1 use the same fixed 2FE2 data */
#define ICCID_2FE2_DATA_MODE_FLASH_ADDR           (USER_CONFIG_FLASH_ADDR + ICCID_2FE2_DATA_MODE_OFFSET_IN_CONFIG)
/* BEL stack softdevice on/off when phone command authentication 0x88
0: BLE off when 0x88 command
1: BLE off when 0x88 command */
#define AUTHEN_0x88_BLE_ON_OFF_FLASH_ADDR         (USER_CONFIG_FLASH_ADDR + AUTHEN_0x88_BLE_ON_OFF_OFFSET_IN_CONFIG)
 

/*--------------------- USIM card EF total file size ----------------------*/
/* total size of EF data save to flash */
#define ALL_USIM_EF_SIZE         247
/* total size of EF data + flags save to flash, multiple of 4 */
#define ALL_USIM_EF_FLAG_SIZE    268

/*------------------------- USIM0 EF file size ----------------------------*/
#define FILE_SIZE_2FE2           10
#define FILE_SIZE_2F05           4
#define FILE_SIZE_6F07           9
#define FILE_SIZE_6F08           33
#define FILE_SIZE_6F09           33
#define FILE_SIZE_6F31           1
#define FILE_SIZE_6F78           2
#define FILE_SIZE_6F7E           11
#define FILE_SIZE_6F73           14
#define FILE_SIZE_6F7B           12
#define FILE_SIZE_6FAD           4
#define FILE_SIZE_6F5C           3
#define FILE_SIZE_6F5B           6
#define FILE_SIZE_6FC4           46
#define FILE_SIZE_6F56           1
#define FILE_SIZE_4F20           9
#define FILE_SIZE_4F52           9
#define FILE_SIZE_6F42           40
/*------------------------- the offset for each EF data ----------------------------*/
/* USIM file 2fe2: 10 bytes */
#define EF_2FE2_OFFSET           (1)
/* USIM file 2f05: 4 bytes */
#define EF_2F05_OFFSET           (11)
/* USIM file 6f07: 9 bytes */
#define EF_6F07_OFFSET           (15)
/* USIM file 6f08: 33 bytes */
#define EF_6F08_OFFSET           (24)
/* USIM file 6f09: 33 bytes */
#define EF_6F09_OFFSET           (57)
/* USIM file 6f31: 1 bytes */
#define EF_6F31_OFFSET           (90)
/* USIM file 6f78: 2 bytes */
#define EF_6F78_OFFSET           (91)
/* USIM file 6f7e: 11 bytes */
#define EF_6F7E_OFFSET           (93)
/* USIM file 6f73: 14 bytes */
#define EF_6F73_OFFSET           (104)
/* USIM file 6f7b: 12 bytes */
#define EF_6F7B_OFFSET           (118)
/* USIM file 6fad: 4 bytes */
#define EF_6FAD_OFFSET           (130)
/* USIM file 6f5c: 3 bytes */
#define EF_6F5C_OFFSET           (134)
/* USIM file 6f5b: 6 bytes */
#define EF_6F5B_OFFSET           (137)
/* USIM file 6fc4: 46 bytes */
#define EF_6FC4_OFFSET           (143)
/* USIM file 6f56: 1 bytes */
#define EF_6F56_OFFSET           (189)
/* USIM file 4f20: 9 bytes */
#define EF_4F20_OFFSET           (190)
/* USIM file 4f52: 9 bytes */
#define EF_4F52_OFFSET           (199)
/* USIM file 6f42: 40 bytes */
#define EF_6F42_OFFSET           (208)

/* EF data updated from USIM flag */
#define FLAG_2FE2_OFFSET         (ALL_USIM_EF_SIZE + 1)
#define FLAG_2F05_OFFSET         (ALL_USIM_EF_SIZE + 2)
#define FLAG_6F07_OFFSET         (ALL_USIM_EF_SIZE + 3)
#define FLAG_6F08_OFFSET         (ALL_USIM_EF_SIZE + 4)
#define FLAG_6F09_OFFSET         (ALL_USIM_EF_SIZE + 5)
#define FLAG_6F31_OFFSET         (ALL_USIM_EF_SIZE + 6)
#define FLAG_6F78_OFFSET         (ALL_USIM_EF_SIZE + 7)
#define FLAG_6F7E_OFFSET         (ALL_USIM_EF_SIZE + 8)
#define FLAG_6F73_OFFSET         (ALL_USIM_EF_SIZE + 9)
#define FLAG_6F7B_OFFSET         (ALL_USIM_EF_SIZE + 10)
#define FLAG_6FAD_OFFSET         (ALL_USIM_EF_SIZE + 11)
#define FLAG_6F5C_OFFSET         (ALL_USIM_EF_SIZE + 12)
#define FLAG_6F5B_OFFSET         (ALL_USIM_EF_SIZE + 13)
#define FLAG_6FC4_OFFSET         (ALL_USIM_EF_SIZE + 14)
#define FLAG_6F56_OFFSET         (ALL_USIM_EF_SIZE + 15)
#define FLAG_4F20_OFFSET         (ALL_USIM_EF_SIZE + 16)
#define FLAG_4F52_OFFSET         (ALL_USIM_EF_SIZE + 17)
#define FLAG_6F42_OFFSET         (ALL_USIM_EF_SIZE + 18)

/* USIM file 2fe2: 10 bytes */
#define USIM0_EF_2FE2_FLASH_ADDR (USIM0_EF_FLASH_ADDR + 1)
/* USIM file 2f05: 4 bytes */
#define USIM0_EF_2F05_FLASH_ADDR (USIM0_EF_2FE2_FLASH_ADDR + FILE_SIZE_2FE2)
/* USIM file 6f07: 9 bytes */
#define USIM0_EF_6F07_FLASH_ADDR (USIM0_EF_2F05_FLASH_ADDR + FILE_SIZE_2F05)
/* USIM file 6f08: 33 bytes */
#define USIM0_EF_6F08_FLASH_ADDR (USIM0_EF_6F07_FLASH_ADDR + FILE_SIZE_6F07)
/* USIM file 6f09: 33 bytes */
#define USIM0_EF_6F09_FLASH_ADDR (USIM0_EF_6F08_FLASH_ADDR + FILE_SIZE_6F08)
/* USIM file 6f31: 1 bytes */
#define USIM0_EF_6F31_FLASH_ADDR (USIM0_EF_6F09_FLASH_ADDR + FILE_SIZE_6F09)
/* USIM file 6f78: 2 bytes */
#define USIM0_EF_6F78_FLASH_ADDR (USIM0_EF_6F31_FLASH_ADDR + FILE_SIZE_6F31)
/* USIM file 6f7e: 11 bytes */
#define USIM0_EF_6F7E_FLASH_ADDR (USIM0_EF_6F78_FLASH_ADDR + FILE_SIZE_6F78)
/* USIM file 6f73: 14 bytes */
#define USIM0_EF_6F73_FLASH_ADDR (USIM0_EF_6F7E_FLASH_ADDR + FILE_SIZE_6F7E)
/* USIM file 6f7b: 12 bytes */
#define USIM0_EF_6F7B_FLASH_ADDR (USIM0_EF_6F73_FLASH_ADDR + FILE_SIZE_6F73)
/* USIM file 6fad: 4 bytes */
#define USIM0_EF_6FAD_FLASH_ADDR (USIM0_EF_6F7B_FLASH_ADDR + FILE_SIZE_6F7B)
/* USIM file 6f5c: 3 bytes */
#define USIM0_EF_6F5C_FLASH_ADDR (USIM0_EF_6FAD_FLASH_ADDR + FILE_SIZE_6FAD)
/* USIM file 6f5b: 6 bytes */
#define USIM0_EF_6F5B_FLASH_ADDR (USIM0_EF_6F5C_FLASH_ADDR + FILE_SIZE_6F5C)
/* USIM file 6fc4: 46 bytes */
#define USIM0_EF_6FC4_FLASH_ADDR (USIM0_EF_6F5B_FLASH_ADDR + FILE_SIZE_6F5B)
/* USIM file 6f56: 1 bytes */
#define USIM0_EF_6F56_FLASH_ADDR (USIM0_EF_6FC4_FLASH_ADDR + FILE_SIZE_6FC4)
/* USIM file 4f20: 9 bytes */
#define USIM0_EF_4F20_FLASH_ADDR (USIM0_EF_6F56_FLASH_ADDR + FILE_SIZE_6F56)
/* USIM file 4f52: 9 bytes */
#define USIM0_EF_4F52_FLASH_ADDR (USIM0_EF_4F20_FLASH_ADDR + FILE_SIZE_4F20)
/* USIM file 6f42: 40 bytes */
#define USIM0_EF_6F42_FLASH_ADDR (USIM0_EF_4F52_FLASH_ADDR + FILE_SIZE_4F52)

/* flag if the USIM file 2fe2 data in flash is updated from USIM card: 1 bye */
#define FLAG_USIM0_EF_2FE2_FLASH_ADDR (USIM0_EF_FLASH_ADDR + ALL_USIM_EF_SIZE + 1)
/* flag if the USIM file 2f05 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_2F05_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 1)
/* flag if the USIM file 6f07 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F07_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 2)
/* flag if the USIM file 6f08 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F08_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 3)
/* flag if the USIM file 6f09 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F09_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 4)
/* flag if the USIM file 6f31 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F31_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 5)
/* flag if the USIM file 6f78 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F78_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 6)
/* flag if the USIM file 6f7e data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F7E_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 7)
/* flag if the USIM file 6f73 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F73_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 8)
/* flag if the USIM file 6f7b data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F7B_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 9)
/* flag if the USIM file 6fad data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6FAD_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 10)
/* flag if the USIM file 6f5c data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F5C_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 11)
/* flag if the USIM file 6f5b data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F5B_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 12)
/* flag if the USIM file 6fc4 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6FC4_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 13)
/* flag if the USIM file 6f56 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F56_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 14)
/* flag if the USIM file 6f07 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_4F20_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 15)
/* flag if the USIM file 6f07 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_4F52_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 16)
/* flag if the USIM file 6f42 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM0_EF_6F42_FLASH_ADDR (FLAG_USIM0_EF_2FE2_FLASH_ADDR + 17)

/*----------------------- USIM1 EF file flash address ---------------------*/
/* USIM file 2fe2: 10 bytes */
#define USIM1_EF_2FE2_FLASH_ADDR (USIM1_EF_FLASH_ADDR + 1)
/* USIM file 2f05: 4 bytes */
#define USIM1_EF_2F05_FLASH_ADDR (USIM1_EF_2FE2_FLASH_ADDR + 0xa)
/* USIM file 6f07: 9 bytes */
#define USIM1_EF_6F07_FLASH_ADDR (USIM1_EF_2F05_FLASH_ADDR + 0x4)
/* USIM file 6f08: 33 bytes */
#define USIM1_EF_6F08_FLASH_ADDR (USIM1_EF_6F07_FLASH_ADDR + 0x9)
/* USIM file 6f09: 33 bytes */
#define USIM1_EF_6F09_FLASH_ADDR (USIM1_EF_6F08_FLASH_ADDR + 0x21)
/* USIM file 6f31: 1 bytes */
#define USIM1_EF_6F31_FLASH_ADDR (USIM1_EF_6F09_FLASH_ADDR + 0x21)
/* USIM file 6f78: 2 bytes */
#define USIM1_EF_6F78_FLASH_ADDR (USIM1_EF_6F31_FLASH_ADDR + 0x1)
/* USIM file 6f7e: 11 bytes */
#define USIM1_EF_6F7E_FLASH_ADDR (USIM1_EF_6F78_FLASH_ADDR + 0x2)
/* USIM file 6f73: 14 bytes */
#define USIM1_EF_6F73_FLASH_ADDR (USIM1_EF_6F7E_FLASH_ADDR + 0xb)
/* USIM file 6f7b: 12 bytes */
#define USIM1_EF_6F7B_FLASH_ADDR (USIM1_EF_6F73_FLASH_ADDR + 0xe)
/* USIM file 6fad: 4 bytes */
#define USIM1_EF_6FAD_FLASH_ADDR (USIM1_EF_6F7B_FLASH_ADDR + 0xc)
/* USIM file 6f5c: 3 bytes */
#define USIM1_EF_6F5C_FLASH_ADDR (USIM1_EF_6FAD_FLASH_ADDR + 0x4)
/* USIM file 6f5b: 6 bytes */
#define USIM1_EF_6F5B_FLASH_ADDR (USIM1_EF_6F5C_FLASH_ADDR + 0x3)
/* USIM file 6fc4: 46 bytes */
#define USIM1_EF_6FC4_FLASH_ADDR (USIM1_EF_6F5B_FLASH_ADDR + 0x6)
/* USIM file 6f56: 1 bytes */
#define USIM1_EF_6F56_FLASH_ADDR (USIM1_EF_6FC4_FLASH_ADDR + 0x2e)
/* USIM file 4f20: 9 bytes */
#define USIM1_EF_4F20_FLASH_ADDR (USIM1_EF_6F56_FLASH_ADDR + 0x1)
/* USIM file 4f52: 9 bytes */
#define USIM1_EF_4F52_FLASH_ADDR (USIM1_EF_4F20_FLASH_ADDR + 0x9)
/* USIM file 6f42: 40 bytes */
#define USIM1_EF_6F42_FLASH_ADDR (USIM1_EF_4F52_FLASH_ADDR + 0x9)


/* flag if the USIM file 2fe2 data in flash is updated from USIM card: 1 bye */
#define FLAG_USIM1_EF_2FE2_FLASH_ADDR (USIM1_EF_FLASH_ADDR + ALL_USIM_EF_SIZE + 1)
/* flag if the USIM file 2f05 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_2F05_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 1)
/* flag if the USIM file 6f07 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F07_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 2)
/* flag if the USIM file 6f08 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F08_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 3)
/* flag if the USIM file 6f09 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F09_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 4)
/* flag if the USIM file 6f31 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F31_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 5)
/* flag if the USIM file 6f78 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F78_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 6)
/* flag if the USIM file 6f7e data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F7E_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 7)
/* flag if the USIM file 6f73 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F73_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 8)
/* flag if the USIM file 6f7b data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F7B_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 9)
/* flag if the USIM file 6fad data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6FAD_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 10)
/* flag if the USIM file 6f5c data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F5C_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 11)
/* flag if the USIM file 6f5b data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F5B_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 12)
/* flag if the USIM file 6fc4 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6FC4_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 13)
/* flag if the USIM file 6f56 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F56_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 14)
/* flag if the USIM file 6f07 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_4F20_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 15)
/* flag if the USIM file 6f07 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_4F52_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 16)
/* flag if the USIM file 6f42 data in flash is updated from USIM card: 1 byte */
#define FLAG_USIM1_EF_6F42_FLASH_ADDR (FLAG_USIM1_EF_2FE2_FLASH_ADDR + 17)

/* RAM buffer for data store 
|  byte 0 ~ 255:     user config settings
|  byte 256 ~ 639:   USIM0 EF data 
|  byte 640 ~ 1023:  USIM1 EF data
----------------------------------------------------------------------*/			
uint8_t P_UINT8_FLASH_DATA_RAM_BUFFER[FLASH_WRITE_DATA_SIZE_IN_WORD << 2];
// uint8_t P_UINT8_FLASH_DATA_RAM_BUFFER[4096];

/*----------------------- user config settings in RAM buffer ------------------------*/
/* smart USIM device name for BLE briadcast */
#define SMART_USIM_DEVICE_NAME_RAM           (P_UINT8_FLASH_DATA_RAM_BUFFER + SMART_USIM_DEVICE_NAME_OFFSET_IN_CONFIG)
/* user passwoord for wireless SIM air interface authentication, 16 bytes = 128 bits, 
|  will be transmitted in the data packet */
#define USER_PASSWORD_RAM                    (P_UINT8_FLASH_DATA_RAM_BUFFER + USER_PASSWORD_OFFSET_IN_CONFIG)
/* system for wireless SIM air interface authentication, 16 bytes = 128 bits,
|  never transmitted in the data packet, but stored in the flash at both side */
#define SYSTEM_KEY_RAM                       (P_UINT8_FLASH_DATA_RAM_BUFFER + SYSTEM_KEY_OFFSET_IN_CONFIG)
/* random authentication challenge or wireless SIM air interface authentication, 16 bytes = 128 bits,
|  will be transmitted in the data packet */
#define AUTH_CHALL_DATA_RAM                  (P_UINT8_FLASH_DATA_RAM_BUFFER + AUTH_CHALL_DATA_OFFSET_IN_CONFIG)

// #define LED_PATTERN_RAM                      (P_UINT8_FLASH_DATA_RAM_BUFFER + LED_PATTERN_OFFSET_IN_CONFIG)
/* device logical address saved in RAM: 1 ~ 7 */
#define DEVICE_LOGICAL_ADDRESS_RAM           (*(P_UINT8_FLASH_DATA_RAM_BUFFER + DEVICE_LOG_ADD_OFFSET_IN_CONFIG))
/* wireless SIM air interface on/off 
|  0: wireless SIM off 
|  1: wireless SIM on */       
#define WIRELESS_SIM_ON_OFF_RAM              (*(P_UINT8_FLASH_DATA_RAM_BUFFER + WIRELESS_SIM_ON_OFF_OFFSET_IN_CONFIG))
/* USIM server side call On/Off flag: 1 word = 4 bytes */
/* 0: USIM server side call off */
/* 1: USIM server side call on */
#define USIM_SERVER_CALL_ON_OFF_RAM          (*(P_UINT8_FLASH_DATA_RAM_BUFFER + USIM_SERVER_CALL_OFFSET_IN_CONFIG))
/* BLE on/off indicator, 1: BLE on, 0: BLE off */
#define BLE_STATUS_ON_OFF_RAM                (*(P_UINT8_FLASH_DATA_RAM_BUFFER + BLE_ON_OFF_OFFSET_IN_CONFIG))
/* BLE bond information erase flag, 1: delete, 0: not delete */
#define BLE_BOND_DEL_STATUS_RAM              (*(P_UINT8_FLASH_DATA_RAM_BUFFER + BLE_BOND_DELETE_OFFSET_IN_CONFIG))
/* wireless USIM mode
0: fixed wireless USIM client mode
1: wireless USIM auto mode, the USIM card presnece detection is performed, 
   if there ther are USIM cards, working at server mode, 
   if there is no USIM card, swith to client mode */
#define WIRELESS_USIM_WORK_MODE_RAM          (*(P_UINT8_FLASH_DATA_RAM_BUFFER + WIRELESS_USIM_WORK_MODE_OFFSET_IN_CONFIG))
/* USIM clock frequency flag, 
|  byte 0, 1: 3.25MHz, 2: 3.84MHz, others: random selection between 3.25MHz and 3.84MHz
|  byte 1, 0: no updated, 1: updated
|  byte 2, ETU ticks byte, 0x94/0x95/0x96 */
#define PHONE_USIM_CLK_FREQ_RAM              (P_UINT8_FLASH_DATA_RAM_BUFFER + USIM_CLK_FREQ_OFFSET_IN_CONFIG)
/* the default used USIM, 0: USIM0, 1: USIM1 */
#define DEFAULT_USED_USIM_RAM				         (*(P_UINT8_FLASH_DATA_RAM_BUFFER + DEFAULT_USED_USIM_OFFSET_IN_CONFIG))
/* USIM data read mode when phone power on, 4bytes. 
1: read ICCID data and check it with the data in flash, 
0: skip read ICCID, only check if the flach data is available */
#define USIM_DATA_MODE_RAM                   (*(P_UINT8_FLASH_DATA_RAM_BUFFER + USIM_DATA_MODE_OFFSET_IN_CONFIG))   
/* ICCID data mode
0: read the 2FE2 data from USIM card, from USIM0 and USIM1 respectively
1: both USIM0 and USIM1 use the same 2FE2 data, read from USIM0
2: both USIM0 and USIM1 use the same fixed 2FE2 data */
#define ICCID_2FE2_DATA_MODE_RAM             (*(P_UINT8_FLASH_DATA_RAM_BUFFER + ICCID_2FE2_DATA_MODE_OFFSET_IN_CONFIG)) 
/* BEL stack softdevice on/off when phone command authentication 0x88
0: BLE off when 0x88 command
1: BLE off when 0x88 command */
#define AUTHEN_0x88_BLE_ON_OFF_RAM           (*(P_UINT8_FLASH_DATA_RAM_BUFFER + AUTHEN_0x88_BLE_ON_OFF_OFFSET_IN_CONFIG)) 


/*------------------------- USIM card EF data in RAM buffer ----------------------------*/
#define USIM0_EF_DATA_RAM                    (P_UINT8_FLASH_DATA_RAM_BUFFER + USIM0_EF_OFFSET_BYTES)
#define USIM1_EF_DATA_RAM                    (P_UINT8_FLASH_DATA_RAM_BUFFER + USIM1_EF_OFFSET_BYTES)
	
#endif


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
#ifndef FILE_DATA_USIM_H__
#define FILE_DATA_USIM_H__

#include "stdint.h"
#include "define_config.h"

/* FCP file data size */
#define FCP_SIZE_3F00 0x2b
#define FCP_SIZE_2FE2 0x25
#define FCP_SIZE_2F05 0x25
#define FCP_SIZE_2F00 0x28
#define FCP_SIZE_ADF  0x39
#define FCP_SIZE_6F38 0x25
#define FCP_SIZE_6F07 0x25
#define FCP_SIZE_6F08 0x25
#define FCP_SIZE_6F09 0x25
#define FCP_SIZE_6F31 0x25
#define FCP_SIZE_6F78 0x25
#define FCP_SIZE_6F7E 0x25
#define FCP_SIZE_6F73 0x25
#define FCP_SIZE_6F7B 0x25
#define FCP_SIZE_6F5B 0x25
#define FCP_SIZE_6F5C 0x25
#define FCP_SIZE_6FC4 0x24
#define FCP_SIZE_6FAD 0x25
#define FCP_SIZE_6FB7 0x28
#define FCP_SIZE_6F06 0x28
#define FCP_SIZE_4F20 0x25
#define FCP_SIZE_4F52 0x25
#define FCP_SIZE_6F56 0x25
#define FCP_SIZE_6F4B 0x27
#define FCP_SIZE_6F49 0x27
#define FCP_SIZE_6F3C 0x27
#define FCP_SIZE_6F43 0x24
#define FCP_SIZE_6F42 0x1e
#define FCP_SIZE_6F46 0x18
/* FCP file data offset */
#define FCP_OFFSET_3F00 0x00
#define FCP_OFFSET_2FE2 (FCP_OFFSET_3F00 + FCP_SIZE_3F00)
#define FCP_OFFSET_2F05 (FCP_OFFSET_2FE2 + FCP_SIZE_2FE2)
#define FCP_OFFSET_2F00 (FCP_OFFSET_2F05 + FCP_SIZE_2F05)
#define FCP_OFFSET_ADF  (FCP_OFFSET_2F00 + FCP_SIZE_2F00)
#define FCP_OFFSET_6F38 (FCP_OFFSET_ADF + FCP_SIZE_ADF)
#define FCP_OFFSET_6F07 (FCP_OFFSET_6F38 + FCP_SIZE_6F38)
#define FCP_OFFSET_6F08 (FCP_OFFSET_6F07 + FCP_SIZE_6F07)
#define FCP_OFFSET_6F09 (FCP_OFFSET_6F08 + FCP_SIZE_6F08)
#define FCP_OFFSET_6F31 (FCP_OFFSET_6F09 + FCP_SIZE_6F09)
#define FCP_OFFSET_6F78 (FCP_OFFSET_6F31 + FCP_SIZE_6F31)
#define FCP_OFFSET_6F7E (FCP_OFFSET_6F78 + FCP_SIZE_6F78)
#define FCP_OFFSET_6F73 (FCP_OFFSET_6F7E + FCP_SIZE_6F7E)
#define FCP_OFFSET_6F7B (FCP_OFFSET_6F73 + FCP_SIZE_6F73)
#define FCP_OFFSET_6F5B (FCP_OFFSET_6F7B + FCP_SIZE_6F7B)
#define FCP_OFFSET_6F5C (FCP_OFFSET_6F5B + FCP_SIZE_6F5B)
#define FCP_OFFSET_6FC4 (FCP_OFFSET_6F5C + FCP_SIZE_6F5C)
#define FCP_OFFSET_6FAD (FCP_OFFSET_6FC4 + FCP_SIZE_6FC4)
#define FCP_OFFSET_6FB7 (FCP_OFFSET_6FAD + FCP_SIZE_6FAD)
#define FCP_OFFSET_6F06 (FCP_OFFSET_6FB7 + FCP_SIZE_6FB7)
#define FCP_OFFSET_4F20 (FCP_OFFSET_6F06 + FCP_SIZE_6F06)
#define FCP_OFFSET_4F52 (FCP_OFFSET_4F20 + FCP_SIZE_4F20)
#define FCP_OFFSET_6F56 (FCP_OFFSET_4F52 + FCP_SIZE_4F52)
#define FCP_OFFSET_6F4B (FCP_OFFSET_6F56 + FCP_SIZE_6F56)
#define FCP_OFFSET_6F49 (FCP_OFFSET_6F4B + FCP_SIZE_6F4B)
#define FCP_OFFSET_6F3C (FCP_OFFSET_6F49 + FCP_SIZE_6F49)
#define FCP_OFFSET_6F43 (FCP_OFFSET_6F3C + FCP_SIZE_6F3C)
#define FCP_OFFSET_6F42 (FCP_OFFSET_6F43 + FCP_SIZE_6F43)
#define FCP_OFFSET_6F46 (FCP_OFFSET_6F42 + FCP_SIZE_6F42)

/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
uint8_t UPDATED_BYTES_2F00[0x20] =
        {0x61,  0x1d,  0x4f,  0x10,  0xa0,  0x0,  0x0,  0x0,  0x87,  0x10,  0x2,  0xff,
         0x86,  0xff,  0xff,  0x89,  0xff,  0xff,  0xff,  0xff,  0x50,  0x9,  0x55,  0x6e,
         0x69,  0x76,  0x65,  0x72,  0x53,  0x49,  0x4d,  0xff}; 
uint8_t UPDATED_BYTES_7FFF_6F38[0xa] = {0x9e,  0x08,  0x4,  0x4,  0x00,  0x0,  0x0,  0x0,  0x0,  0x0};
uint8_t UPDATED_BYTES_7FFF_6F43[0x2] = {0xa,  0xff};
uint8_t UPDATED_BYTES_7FFF_6F46[0x11] =
        {0x0,  'W',  'i',  'r',  'e',  'l',  'e',  's',  's',  ' ',  'U',  'S',  'I',  'M',
         0xff,  0xff,  0xff};
/* record size: 0x2c=44, record number: 0x9 */
uint8_t UPDATED_BYTES_7FFF_6F06[0x2c] =
        {0x80,  0x1,  0x18,  0xa4,  0x6,  0x83,  0x1,  0xa,  0x95,  0x1,  0x8,  0x80,  0x1,
         0x3,  0xa4,  0x6,  0x83,  0x1,  0x1,  0x95,  0x1,  0x8,  0xff,  0xff,  0xff,  0xff,
         0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,
         0xff,  0xff,  0xff,  0xff,  0xff,  0xff};
/************************************************************************************************/

#endif

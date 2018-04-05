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

#include "nrf_soc.h"
#include "sd_ble_ancs_nus.h"
#include "define_config.h"
#include "app_scheduler.h"
#include "nrf_sdh.h"

/* smart USIM connector initialization */
void smart_usim_initialization(void);

/********************************************************************************/
int main(void)
/*--------------------------------------------------------------------------------
| Application main function
|
--------------------------------------------------------------------------------*/
{
  /* smart USIM connector initialization */
  smart_usim_initialization( );

  /*----------------------------------------------------------------------------*/
  /* Enter main loop with power management */
  while (true)
  {
#if (IF_SOFTDEIVE_USED)
    app_sched_execute();
    if(nrf_sdh_is_enabled())
    {
      power_manage();
    }
    else
#endif
    {
      __WFE();
      __SEV();
      __WFE();
    }
  }
}


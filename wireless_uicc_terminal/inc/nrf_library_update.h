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
#ifndef NRF_LIBRARY_UPDATE_H__
#define NRF_LIBRARY_UPDATE_H__

#include "nrf_gpio.h"
#include "define_config.h"

/* input/ouput PIN configuration, used for UICC interface IO */

__STATIC_INLINE void nrf_gpio_cfg_output_S0D1(uint32_t pin_number)
{
    nrf_gpio_cfg(
            pin_number,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_CONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0D1,
            NRF_GPIO_PIN_NOSENSE);
}

__STATIC_INLINE void nrf_gpio_cfg_output_H0D1(uint32_t pin_number)
{
    nrf_gpio_cfg(
            pin_number,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_CONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_H0D1,
            NRF_GPIO_PIN_NOSENSE);
}

__STATIC_INLINE void nrf_gpio_cfg_output_H0H1(uint32_t pin_number)
{
    nrf_gpio_cfg(
            pin_number,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);
}

__STATIC_INLINE void nrf_gpio_cfg_input_H0H1(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config)
{
    nrf_gpio_cfg(
            pin_number,
            NRF_GPIO_PIN_DIR_INPUT,
            NRF_GPIO_PIN_INPUT_CONNECT,
            pull_config,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);
}

/**
  \brief   Get Interrupt Enable status
  \details Returns a device specific interrupt enable status from the NVIC interrupt controller.
  \param [in]      IRQn  Device specific interrupt number.
  \return             0  Interrupt is not enabled.
  \return             1  Interrupt is enabled.
  \note    IRQn must not be negative.
 */
__STATIC_INLINE uint32_t __NVIC_GetEnableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)(IRQn) >= 0)
  {
    return((uint32_t)(((NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] & (1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));
  }
  else
  {
    return(0U);
  }
}

#endif 

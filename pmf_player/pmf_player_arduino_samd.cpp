//============================================================================
// PMF Player
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Profoundic Technologies nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL PROFOUNDIC TECHNOLOGIES BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//============================================================================

#include "pmf_player.h"
#if defined(ARDUINO_ARCH_SAMD)
#include "pmf_data.h"
//---------------------------------------------------------------------------


//===========================================================================
// audio buffer
//===========================================================================
static pmf_audio_buffer<int16_t, 2048> s_audio_buffer;
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
void setup_timer4(uint16_t clk_div_, uint8_t count_)
{
   // Set up the generic clock (GCLK4) used to clock timers
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |          // Divide the 48MHz clock source by divisor 3: 48MHz/1=48MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Feed GCLK4 to TC4 and TC5
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_TC4_CTRLA |= TC_CTRLA_MODE_COUNT8;          // Set the counter to 8-bit mode
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  REG_TC4_COUNT8_CC0 = count_;                      // Set the TC4 CC0 register to some arbitary value
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  REG_TC4_INTFLAG |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENSET_OVF;             // Enable TC4 interrupts

  uint16_t prescale=0;
  switch(clk_div_)
  {
    case 1:    prescale=TC_CTRLA_PRESCALER(0); break;
    case 2:    prescale=TC_CTRLA_PRESCALER(1); break;
    case 4:    prescale=TC_CTRLA_PRESCALER(2); break;
    case 8:    prescale=TC_CTRLA_PRESCALER(3); break;
    case 16:   prescale=TC_CTRLA_PRESCALER(4); break;
    case 64:   prescale=TC_CTRLA_PRESCALER(5); break;
    case 256:  prescale=TC_CTRLA_PRESCALER(6); break;
    case 1024: prescale=TC_CTRLA_PRESCALER(7); break;
  }
  REG_TC4_CTRLA |= prescale | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_ENABLE;    // Enable TC4
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization
}
//----

void TC4_Handler()
{
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)             
  {
    // write 10-bit value from audio buffer to the DAC
    uint16_t smp=s_audio_buffer.read_sample<uint16_t, 10>();
    analogWrite(A0, smp);
/*#if defined(__SAMD51__)
    while(DAC->SYNCBUSY.bit.DATA0);
    DAC->DATA[0].reg=smp;
#else
    while(DAC->STATUS.reg&DAC_STATUS_SYNCBUSY);
    // and write the data
    DAC->DATA.reg=smp;
#endif*/
    REG_TC4_INTFLAG = TC_INTFLAG_OVF;
  }
}
//----

uint16_t next_pow2(uint16_t v_)
{
  // the next power-of-2 of the value (if v_ is pow-of-2 returns v_)
  --v_;
  v_|=v_>>1;
  v_|=v_>>2;
  v_|=v_>>4;
  v_|=v_>>8;
  return v_+1;
}
//----

uint16_t get_clk_div(uint32_t sampling_freq_)
{
  float ideal_clk_div=48000000.0f/(256.0f*float(sampling_freq_));
  uint16_t clk_div=next_pow2(uint16_t(ceil(ideal_clk_div)));
  switch(clk_div)
  {
    case 32: clk_div=64; break;
    case 128: clk_div=256; break;
    case 512: clk_div=1024; break;
  }
  return clk_div;
}
//---------------------------------------------------------------------------

uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_)
{
  // return closest frequency representable with the timer
  uint16_t clk_div=get_clk_div(sampling_freq_);
  uint32_t timer_clk=48000000/clk_div;
  uint8_t timer_count=timer_clk/sampling_freq_;
  return timer_clk/timer_count;
}
//----

void pmf_player::start_playback(uint32_t sampling_freq_)
{
  // enable playback interrupt at given playback frequency
  analogWriteResolution(10);
  s_audio_buffer.reset();
  uint16_t clk_div=get_clk_div(sampling_freq_);
  uint8_t clk_cnt=(48000000/clk_div)/sampling_freq_;
  setup_timer4(clk_div, clk_cnt);
}
//----

void pmf_player::stop_playback()
{
  /*todo*/
}
//----

void pmf_player::mix_buffer(pmf_mixer_buffer &buf_, unsigned num_samples_)
{
  mix_buffer_impl(buf_, num_samples_);
}
//----

pmf_mixer_buffer pmf_player::get_mixer_buffer()
{
  return s_audio_buffer.get_mixer_buffer();
}
//---------------------------------------------------------------------------

//===========================================================================
#endif // ARDUINO_ARCH_SAMD

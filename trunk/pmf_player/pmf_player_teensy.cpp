//============================================================================
// PMF Player v0.3
//
// Copyright (c) 2013, Profoundic Technologies, Inc.
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
#if defined(CORE_TEENSY)
#include "pmf_data.h"
//---------------------------------------------------------------------------


//===========================================================================
// audio buffer
//===========================================================================
enum {pmfplayer_audio_buffer_size=400};  // number of 16-bit samples in the buffer
enum {audio_subbuffer_size=pmfplayer_audio_buffer_size/2};
static int16_t s_buffer[pmfplayer_audio_buffer_size];
static int16_t *volatile s_buffer_playback_pos;
static uint8_t s_subbuffer_write_idx=0;
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
static IntervalTimer s_int_timer;
//----

void playback_interrupt()
{
  int16_t *smp_addr=s_buffer_playback_pos;
  int16_t smp=*smp_addr;
  *smp_addr=0x80<<PMF_AUDIO_LEVEL;
  smp>>=PMF_AUDIO_LEVEL;
  smp=smp<0?0:smp>255?255:smp;
  GPIOD_PSOR=smp;
  GPIOD_PCOR=~smp;
  if(++s_buffer_playback_pos==s_buffer+pmfplayer_audio_buffer_size)
    s_buffer_playback_pos=s_buffer;
}
//----

void pmf_player::start_playback()
{
  // setup pins
  #define GPIO_BITBAND(reg, bit) (*(uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)))
  #define GPIO_BITBAND_ADDR(reg, bit) (((uint32_t)&(reg) - 0x40000000) * 32 + (bit) * 4 + 0x42000000)
  #define CONFIG_PULLUP (PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PS)
  GPIO_BITBAND(GPIOD_PDDR, 0) = 1;
  PORTD_PCR0 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 1) = 1;
  PORTD_PCR1 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 2) = 1;
  PORTD_PCR2 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 3) = 1;
  PORTD_PCR3 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 4) = 1;
  PORTD_PCR4 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 5) = 1;
  PORTD_PCR5 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 6) = 1;
  PORTD_PCR6 = CONFIG_PULLUP;
  GPIO_BITBAND(GPIOD_PDDR, 7) = 1;
  PORTD_PCR7 = CONFIG_PULLUP;

  // clear audio buffer
  for(unsigned i=0; i<pmfplayer_audio_buffer_size; ++i)
    s_buffer[i]=0x80<<PMF_AUDIO_LEVEL;
  s_buffer_playback_pos=s_buffer;
  s_subbuffer_write_idx=1;

  // enable playback interrupt at given playback frequency
  s_int_timer.begin(playback_interrupt, 1000000.0f/pmfplayer_sampling_rate);
}
//----

void pmf_player::stop_playback()
{
  s_int_timer.end();
}
//----

void pmf_player::mix_buffer(mixer_buffer &buf_, unsigned num_samples_)
{
  audio_channel *channel=m_channels, *channel_end=channel+m_num_playback_channels;
  do
  {
    // check for active channel
    if(!channel->sample_speed)
      continue;

    // get channel attributes
    size_t sample_addr=(size_t)(m_pmf_file+pgm_read_word(channel->inst_metadata+pmfcfg_offset_inst_offset));
    uint32_t sample_pos=(long(sample_addr)<<8)+channel->sample_pos;
    uint16_t sample_speed=channel->sample_speed;
    uint16_t sample_end=sample_addr+pgm_read_word(channel->inst_metadata+pmfcfg_offset_inst_length);
    uint16_t sample_loop_len=pgm_read_word(channel->inst_metadata+pmfcfg_offset_inst_loop_length);
    uint8_t sample_volume=(uint16_t(channel->sample_volume)*channel->vol_env_value)>>8;

    // mix channel to the buffer
    int16_t *buf=(int16_t*)buf_.begin, *buffer_end=buf+buf_.num_samples;
    do
    {
      // add channel sample to buffer
      int8_t smp=(int8_t)pgm_read_byte(sample_pos>>8);
      *buf+=int16_t(sample_volume*int16_t(smp))>>8;

      // advance sample position
      sample_pos+=sample_speed;
      if(uint16_t(sample_pos>>8)>=sample_end)
      {
        sample_pos-=long(sample_loop_len)<<8;
        if(!sample_loop_len)
        {
          channel->sample_speed=0;
          break;
        }
      }
    } while(++buf<buffer_end);
    channel->sample_pos=sample_pos-(long(sample_addr)<<8);
  } while(++channel!=channel_end);

  // advance buffer
  ((int16_t*&)buf_.begin)+=num_samples_;
  buf_.num_samples-=num_samples_;
}
//----

pmf_player::mixer_buffer pmf_player::get_mixer_buffer()
{
  /*todo: disable interrupts*/
  const int16_t *playback_pos=s_buffer_playback_pos;
  /*todo: enable interrupts*/
  mixer_buffer buf={0, 0};
  if(   (s_subbuffer_write_idx && playback_pos>=s_buffer+audio_subbuffer_size)
     || (!s_subbuffer_write_idx && playback_pos<s_buffer+audio_subbuffer_size))
    return buf;
  buf.begin=s_buffer+s_subbuffer_write_idx*audio_subbuffer_size;
  buf.num_samples=audio_subbuffer_size;
  s_subbuffer_write_idx^=1;
  return buf;
}
//----

void pmf_player::visualize_pattern_frame()
{
}
//---------------------------------------------------------------------------

//===========================================================================
#endif // CORE_TEENSY

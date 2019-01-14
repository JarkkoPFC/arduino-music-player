//============================================================================
// PMF Player v0.4
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
  *smp_addr=0;
  smp=2048+(smp<<PMF_AUDIO_LEVEL);
  smp=smp<0?0:smp>4095?4095:smp;
  analogWriteDAC0(smp);
  if(++s_buffer_playback_pos==s_buffer+pmfplayer_audio_buffer_size)
    s_buffer_playback_pos=s_buffer;
}
//----

void pmf_player::start_playback()
{
  // setup pins
  DDRB=0x3f;
  DDRC=0x3f;

  // clear audio buffer
  for(unsigned i=0; i<pmfplayer_audio_buffer_size; ++i)
    s_buffer[i]=0;
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
    size_t sample_addr=(size_t)(m_pmf_file+pgm_read_dword(channel->inst_metadata+pmfcfg_offset_inst_offset));
    uint32_t sample_pos=channel->sample_pos;
    int16_t sample_speed=channel->sample_speed;
    uint32_t sample_end=uint32_t(pgm_read_dword(channel->inst_metadata+pmfcfg_offset_inst_length))<<8;
    uint32_t sample_loop_len=pgm_read_dword(channel->inst_metadata+pmfcfg_offset_inst_loop_length)<<8;
    uint8_t sample_volume=(uint16_t(channel->sample_volume)*channel->vol_env_value)>>8;
    uint32_t sample_pos_offs=sample_end-sample_loop_len;
    if(sample_pos<sample_pos_offs)
      sample_pos_offs=0;
    sample_addr+=sample_pos_offs>>8;
    sample_pos-=sample_pos_offs;
    sample_end-=sample_pos_offs;

    // mix channel to the buffer
    int16_t *buf=(int16_t*)buf_.begin, *buffer_end=buf+num_samples_;
    do
    {
      // add channel sample to buffer
#ifdef PMF_LINEAR_INTERPOLATION
      uint16_t smp=((uint16_t)pgm_read_word(sample_addr+(sample_pos>>8)));
      uint8_t sample_pos_frc=sample_pos&255;
      int16_t interp_smp=((int16_t(int8_t(smp&255))*(256-sample_pos_frc))>>8)+((int16_t(int8_t(smp>>8))*sample_pos_frc)>>8);
      *buf+=int16_t(sample_volume*interp_smp)>>8;
#else
      int8_t smp=(int8_t)pgm_read_byte(sample_addr+(sample_pos>>8));
      *buf+=int16_t(sample_volume*int16_t(smp))>>8;
#endif

      // advance sample position
      sample_pos+=sample_speed;
      if(sample_pos>=sample_end)
      {
        // check for loop
        if(!sample_loop_len)
        {
          channel->sample_speed=0;
          break;
        }

        // apply normal/bidi loop
        if(pgm_read_byte(channel->inst_metadata+pmfcfg_offset_inst_flags)&pmfinstflag_bidi_loop)
        {
          sample_pos-=sample_speed*2;
          channel->sample_speed=sample_speed=-sample_speed;
        }
        else
          sample_pos-=sample_loop_len;
      }
    } while(++buf<buffer_end);
    channel->sample_pos=sample_pos+sample_pos_offs;
  } while(++channel!=channel_end);

  // advance buffer
  ((int16_t*&)buf_.begin)+=num_samples_;
  buf_.num_samples-=num_samples_;
}
//----

pmf_player::mixer_buffer pmf_player::get_mixer_buffer()
{
  cli();
  const int16_t *playback_pos=s_buffer_playback_pos;
  sei();
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
  // get LED states
  uint16_t led_bits=0;
  for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];
    if(chl.note_hit)
    {
      led_bits|=1<<ci;
      chl.note_hit=0;
    }
  }

  // set LEDs
  PORTB=led_bits;
  PORTC=led_bits>>6;
}
//---------------------------------------------------------------------------

//===========================================================================
#endif // CORE_TEENSY

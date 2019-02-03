//============================================================================
// PMF Player v0.5
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
#if defined(__AVR_ATmega328P__)
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
ISR(TIMER1_COMPA_vect)
{
  static const int8_t s_mid_buffer_value_hi=1<<(PMF_AUDIO_LEVEL-1);
  int16_t smp;
  asm volatile
  (
    "ld %A[smp], %a[buffer_pos] \n\t"
    "st %a[buffer_pos]+, __zero_reg__ \n\t"
    "ld %B[smp], %a[buffer_pos] \n\t"
    "lds __tmp_reg__, %[mid_buffer_value_hi] \n\t"
    "st %a[buffer_pos]+, __tmp_reg__ \n\t"

#if PMF_AUDIO_LEVEL>1
    "asr %B[smp] \n\t"
    "ror %A[smp] \n\t"
#endif
#if PMF_AUDIO_LEVEL>2
    "asr %B[smp] \n\t"
    "ror %A[smp] \n\t"
#endif
#if PMF_AUDIO_LEVEL>3
    "asr %B[smp] \n\t"
    "ror %A[smp] \n\t"
#endif
    "asr %B[smp] \n\t"
    "breq no_sample_clamp_%= \n\t"
    "lsl %B[smp] \n\t"
    "sbc %B[smp], %B[smp] \n\t "
    "com %B[smp] \n\t"
    "out %[port_d], %B[smp] \n\t"
    "rjmp check_buffer_restart_%= \n\t"

    "restart_buffer_%=: \n\t"
    "ldi %A[buffer_pos], lo8(%[buffer_begin]) \n\t"
    "ldi %B[buffer_pos], hi8(%[buffer_begin]) \n\t"
    "rjmp done_%= \n\t\n\t"

    "no_sample_clamp_%=: \n\t"
    "ror %A[smp] \n\t"
    "out %[port_d], %A[smp] \n\t"

    "check_buffer_restart_%=: \n\t"
    "cpi %A[buffer_pos], lo8(%[buffer_end]) \n\t"
    "ldi %A[smp], hi8(%[buffer_end]) \n\t"
    "cpc %B[buffer_pos], %A[smp] \n\t"
    "breq restart_buffer_%= \n\t"

    "done_%=: \n\t"

    :[buffer_pos] "+e" (s_buffer_playback_pos)
    ,[smp] "=&r" (smp)
    :[buffer_begin] "p" (s_buffer)
    ,[buffer_end] "p" (s_buffer+pmfplayer_audio_buffer_size)
    ,[mid_buffer_value_hi] "X" (&s_mid_buffer_value_hi)
    ,[port_d] "I" (_SFR_IO_ADDR(PORTD))
  );
}
//----

void pmf_player::start_playback()
{
  // setup pins
  DDRD=0xff;
  DDRB=0x3f;
  DDRC=0x3f;

  // clear audio buffer
  for(unsigned i=0; i<pmfplayer_audio_buffer_size; ++i)
    s_buffer[i]=0x80<<PMF_AUDIO_LEVEL;
  s_buffer_playback_pos=s_buffer;
  s_subbuffer_write_idx=1;

  // enable playback interrupt at given playback frequency
  TCCR1A=0;
  TCCR1B=_BV(CS10)|_BV(WGM12); // CTC mode 4 (OCR1A)
  TCCR1C=0;
  TIMSK1=_BV(OCIE1A);          // enable timer 1 counter A
  OCR1A=(16000000+pmfplayer_sampling_rate/2)/pmfplayer_sampling_rate;
}
//----

void pmf_player::stop_playback()
{
  TIMSK1=0;
}
//----

void pmf_player::mix_buffer(mixer_buffer &buf_, unsigned num_samples_)
{
  int16_t *buffer_begin=(int16_t*)buf_.begin, *buffer_end=buffer_begin+num_samples_;
  audio_channel *channel=m_channels, *channel_end=channel+m_num_playback_channels;
  do
  {
    // check for active channel
    if(!channel->sample_speed)
      continue;

    // get channel attributes
    size_t sample_addr=(size_t)(m_pmf_file+pgm_read_dword(channel->smp_metadata+pmfcfg_offset_smp_data));
    uint16_t sample_len=pgm_read_word(channel->smp_metadata+pmfcfg_offset_smp_length);/*todo: should be dword*/
    uint16_t loop_len=pgm_read_word(channel->smp_metadata+pmfcfg_offset_smp_loop_length);/*todo: should be dword*/
    uint8_t volume=(uint16_t(channel->sample_volume)*channel->vol_env.value>>8)>>8;
    register uint8_t sample_pos_frc=channel->sample_pos;
    register uint16_t sample_pos_int=sample_addr+(channel->sample_pos>>8);
    register uint16_t sample_speed=channel->sample_speed;
    register uint16_t sample_end=sample_addr+sample_len;
    register uint16_t sample_loop_len=loop_len;
    register uint8_t sample_volume=volume;
    register uint8_t zero=0, upper_tmp;

    asm volatile
    (
      "push %A[buffer_pos] \n\t"
      "push %B[buffer_pos] \n\t"

      "mix_samples_%=: \n\t"
      "lpm %[upper_tmp], %a[sample_pos_int] \n\t"
      "mulsu %[upper_tmp], %[sample_volume] \n\t"
      "mov %[upper_tmp], r1 \n\t"
      "lsl %[upper_tmp] \n\t"
      "sbc %[upper_tmp], %[upper_tmp] \n\t"
      "ld __tmp_reg__, %a[buffer_pos] \n\t"
      "add __tmp_reg__, r1 \n\t"
      "st %a[buffer_pos]+, __tmp_reg__ \n\t"
      "ld __tmp_reg__, %a[buffer_pos] \n\t"
      "adc __tmp_reg__, %[upper_tmp] \n\t"
      "st %a[buffer_pos]+, __tmp_reg__ \n\t"
      "add %[sample_pos_frc], %A[sample_speed] \n\t"
      "adc %A[sample_pos_int], %B[sample_speed] \n\t"
      "adc %B[sample_pos_int], %[zero] \n\t"
      "cp %A[sample_pos_int], %A[sample_end] \n\t"
      "cpc %B[sample_pos_int], %B[sample_end] \n\t"
      "brcc sample_end_%= \n\t"
      "next_sample_%=: \n\t"
      "cp %A[buffer_pos], %A[buffer_end] \n\t"
      "cpc %B[buffer_pos], %B[buffer_end] \n\t"
      "brne mix_samples_%= \n\t"
      "rjmp done_%= \n\t"

      "sample_end_%=: \n\t"
      /*todo: implement bidi loop support*/
      "sub %A[sample_pos_int], %A[sample_loop_len] \n\t"
      "sbc %B[sample_pos_int], %B[sample_loop_len] \n\t"
      "mov __tmp_reg__, %A[sample_loop_len] \n\t"
      "or __tmp_reg__, %B[sample_loop_len] \n\t"
      "brne next_sample_%= \n\t"
      "clr %A[sample_speed] \n\t"
      "clr %B[sample_speed] \n\t"

      "done_%=: \n\t"
      "clr r1 \n\t"
      "pop %B[buffer_pos] \n\t"
      "pop %A[buffer_pos] \n\t"

      :[sample_volume] "+d" (sample_volume)
      ,[sample_speed] "+l" (sample_speed)
      ,[sample_pos_frc] "+l" (sample_pos_frc)
      ,[sample_pos_int] "+z" (sample_pos_int)

      :[sample_end] "r" (sample_end)
      ,[upper_tmp] "d" (upper_tmp)
      ,[zero] "r" (zero)
      ,[sample_loop_len] "l" (sample_loop_len)
      ,[buffer_pos] "e" (buffer_begin)
      ,[buffer_end] "l" (buffer_end)
    );

    // store values back to the channel data
    channel->sample_pos=(long(sample_pos_int-sample_addr)<<8)+sample_pos_frc;
    channel->sample_speed=sample_speed;
  } while(++channel!=channel_end);

  // advance buffer
  ((int16_t*&)buf_.begin)+=num_samples_;
  buf_.num_samples-=num_samples_;
}
//----

pmf_player::mixer_buffer pmf_player::get_mixer_buffer()
{
  asm volatile("cli"::);
  const int16_t *playback_pos=s_buffer_playback_pos;
  asm volatile("sei"::);
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
#endif // __AVR_ATmega328P__

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
#if defined(CORE_TEENSY)
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
static IntervalTimer s_int_timer;
//----

void playback_interrupt()
{
  uint16_t smp=s_audio_buffer.read_sample<uint16_t, 12>();
  analogWriteDAC0(smp);
}
//----

uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_)
{
  // round to the closest frequency representable by the bus
  float us=1000000.0f/sampling_freq_;
  uint32_t cycles=us*(F_BUS/1000000)-0.5f;
  float freq=1000000.0f*(F_BUS/1000000)/float(cycles);
  return uint32_t(freq+0.5f);
}
//----

void pmf_player::start_playback(uint32_t sampling_freq_)
{
  // enable playback interrupt at given frequency
  s_audio_buffer.reset();
  s_int_timer.begin(playback_interrupt, 1000000.0f/sampling_freq_);
}
//----

void pmf_player::stop_playback()
{
  s_int_timer.end();
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
#endif // CORE_TEENSY

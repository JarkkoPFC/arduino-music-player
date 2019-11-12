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

#if PFC_USE_SGTL5000_AUDIO_SHIELD==1
#include "output_i2s.h"
#include "control_sgtl5000.h"
//---------------------------------------------------------------------------

//===========================================================================
// mod_audio_stream
//===========================================================================
class mod_audio_stream: public AudioStream
{
public:
  // construction
  mod_audio_stream();
  ~mod_audio_stream();
  void init(AudioStream &output_, bool stereo_=true);
  pmf_mixer_buffer get_mixer_buffer();
  //-------------------------------------------------------------------------

private:
  virtual void update();
  //-------------------------------------------------------------------------
  
  pmf_audio_buffer<int32_t, 2048> m_audio_buffer;
  AudioConnection *m_connection_l, *m_connection_r;
  bool m_stereo;
};
//---------------------------------------------------------------------------

mod_audio_stream::mod_audio_stream()
  :AudioStream(0, 0)
  ,m_connection_l(0)
  ,m_connection_r(0)
  ,m_stereo(false)
{
}
//----

mod_audio_stream::~mod_audio_stream()
{
  delete m_connection_l;
  delete m_connection_r;
}
//----

void mod_audio_stream::init(AudioStream &output_, bool stereo_)
{
  m_audio_buffer.reset();
  m_connection_l=new AudioConnection(*this, 0, output_, 0);
  m_connection_r=new AudioConnection(*this, stereo_?1:0, output_, 1);
  m_stereo=stereo_;
}
//----

pmf_mixer_buffer mod_audio_stream::get_mixer_buffer()
{
  pmf_mixer_buffer buf=m_audio_buffer.get_mixer_buffer();
  if(m_stereo)
    buf.num_samples/=2;
  return buf;
}
//---------------------------------------------------------------------------

void mod_audio_stream::update()
{
  if(m_stereo)
  {
    audio_block_t *block_l=allocate();
    audio_block_t *block_r=allocate();
    int16_t *data_l=block_l->data, *data_r=block_r->data;
    for(unsigned i=0; i<AUDIO_BLOCK_SAMPLES; ++i)
    {
      uint16_t vl=m_audio_buffer.read_sample<uint16_t, 16>();
      data_l[i]=vl-32768;
      uint16_t vr=m_audio_buffer.read_sample<uint16_t, 16>();
      data_r[i]=vr-32768;
    }
    transmit(block_l, 0);
    transmit(block_r, 1);
    release(block_l);
    release(block_r);
  }
  else
  {
    audio_block_t *block=allocate();
    int16_t *data=block->data;
    for(unsigned i=0; i<AUDIO_BLOCK_SAMPLES; ++i)
    {
      uint16_t v=m_audio_buffer.read_sample<uint16_t, 16>();
      data[i]=v-32768;
    }
    transmit(block, 0);
    release(block);
  }
}
//---------------------------------------------------------------------------

static mod_audio_stream s_mod_stream;
static AudioOutputI2S s_dac;
static AudioControlSGTL5000 s_sgtl5000;
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_) const
{
  return uint32_t(AUDIO_SAMPLE_RATE_EXACT+0.5f);
}
//----

void pmf_player::start_playback(uint32_t sampling_freq_)
{
  // setup
  AudioMemory(2);
  s_sgtl5000.enable();
  s_sgtl5000.volume(0.5);
  s_mod_stream.init(s_dac, PMF_USE_STEREO_MIXING?true:false);
}
//----

void pmf_player::stop_playback()
{
  /*todo*/
}
//----

void pmf_player::mix_buffer(pmf_mixer_buffer &buf_, unsigned num_samples_)
{
  mix_buffer_impl<int32_t, PMF_USE_STEREO_MIXING?true:false, 13>(buf_, num_samples_);
}
//----

pmf_mixer_buffer pmf_player::get_mixer_buffer()
{
  return s_mod_stream.get_mixer_buffer();
}
//---------------------------------------------------------------------------

#else // PFC_USE_SGTL5000_AUDIO_SHIELD
//===========================================================================
// pmf_player
//===========================================================================
static pmf_audio_buffer<int16_t, 2048> s_audio_buffer;
static IntervalTimer s_int_timer;
//----

void playback_interrupt()
{
  uint16_t smp=s_audio_buffer.read_sample<uint16_t, 12>();
  analogWriteDAC0(smp);
}
//----

uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_) const
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
  mix_buffer_impl<int16_t>(buf_, num_samples_);
}
//----

pmf_mixer_buffer pmf_player::get_mixer_buffer()
{
  return s_audio_buffer.get_mixer_buffer();
}
//---------------------------------------------------------------------------
#endif

//===========================================================================
#endif // CORE_TEENSY

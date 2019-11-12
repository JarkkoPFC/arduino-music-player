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

#ifndef PFC_PMF_PLAYER_H
#define PFC_PMF_PLAYER_H
//---------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include <Arduino.h>
#include "pmf_data.h"

// new
struct pmf_channel_info;
struct pmf_mixer_buffer;
class pmf_player;
template<typename T, unsigned buffer_size> struct pmf_audio_buffer;
typedef void(*pmf_row_callback_t)(void *custom_data_, uint8_t channel_idx_, uint8_t &note_idx_, uint8_t &inst_idx_, uint8_t &volume_, uint8_t &effect_, uint8_t &effect_data_);
typedef void(*pmf_tick_callback_t)(void *custom_data_);
//---------------------------------------------------------------------------


//===========================================================================
// PMF player config
//===========================================================================
enum {pmfplayer_max_channels=12};        // maximum number of audio playback channels (reduce to save dynamic memory)
#define PMF_USE_STEREO_MIXING 1          // use stereo mixing if supported (interleaved in the audio output buffer)
#define PMF_USE_LINEAR_INTERPOLATION 0   // interpolate samples linearly for better sound quality (more performanmce intensive)
#define PFC_USE_SGTL5000_AUDIO_SHIELD 0  // enable playback through SGTL5000-based audio shield (Teensy)
#define PMF_USE_SERIAL_LOGS 0            // enable logging to serial output (disable to save memory)
//---------------------------------------------------------------------------


//===========================================================================
// logging
//===========================================================================
#if PMF_USE_SERIAL_LOGS==1
#define PMF_SERIAL_LOG(...) {char buf[64]; sprintf(buf, __VA_ARGS__); Serial.print(buf);}
#else
#define PMF_SERIAL_LOG(...)
#endif
//---------------------------------------------------------------------------


//===========================================================================
// e_pmf_effect/e_pmf_subfx
//===========================================================================
enum e_pmf_effect
{
  // global control
  pmffx_set_speed_tempo,   // [1, 255], [1, 32]=speed, [33, 255]=tempo
  pmffx_position_jump,     // [0, song_len-1]
  pmffx_pattern_break,     // [0, 255]
  // channel effects
  pmffx_volume_slide,      // [00xxyyyy], x=slide type (0=slide down, 1=slide up, 2=fine slide down, 3=fine slide up), y=slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_note_slide_down,   // [1, 0xdf] = normal slide, [0xe0, 0xef] = extra fine slide, [0xf0, 0xff] = fine slide, 0=use previous slide value
  pmffx_note_slide_up,     // [1, 0xdf] = normal slide, [0xe0, 0xef] = extra fine slide, [0xf0, 0xff] = fine slide, 0=use previous slide value
  pmffx_note_slide,        // [1, 0xdf] = slide, 0=use previous slide value, [0xe0, 0xff]=unused
  pmffx_arpeggio,          // x=[0, 15], y=[0, 15]
  pmffx_vibrato,           // [xxxxyyyy], x=vibrato speed, y=vibrato depth
  pmffx_tremolo,           // [xxxxyyyy], x=tremolo speed, y=tremolo depth
  pmffx_note_vol_slide,    // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_vibrato_vol_slide, // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_retrig_vol_slide,  // [xxxxyyyy], x=volume slide param, y=sample retrigger frequency
  pmffx_set_sample_offset, // [xxxxxxxx], offset=x*256
  pmffx_subfx,             // [xxxxyyyy], x=sub-effect, y=sub-effect value
  pmffx_panning,           // [xyzwwwww], x=type (0=set, 1=slide), y=precision (0=normal, 1=fine), z=direction (0=left, 1=right), w=value (if x=0, pan value is [yzwwwww])
};
//----

enum e_pmf_subfx
{
  pmfsubfx_set_glissando,    // 0=off, 1=on (when enabled "note slide" slides half a not at a time)
  pmfsubfx_set_finetune,     // [-8, 7]
  pmfsubfx_set_vibrato_wave, // [0xyy], x=[0=retrigger, 1=no retrigger], yy=vibrato wave=[0=sine, 1=ramp down, 2=square, 3=random]
  pmfsubfx_set_tremolo_wave, // [0xyy], x=[0=retrigger, 1=no retrigger], yy=tremolo wave=[0=sine, 1=ramp down, 2=square, 3=random]
  pmfsubfx_pattern_delay,    // [1, 15]
  pmfsubfx_pattern_loop,     // [0, 15], 0=set loop start, >0 = loop N times from loop start
  pmfsubfx_note_cut,         // [0, 15], cut on X tick
  pmfsubfx_note_delay,       // [0, 15], delay X ticks
};
//---------------------------------------------------------------------------


//===========================================================================
// pmf_channel_info
//===========================================================================
struct pmf_channel_info
{
  uint8_t base_note;
  uint8_t volume;
  uint8_t effect;
  uint8_t effect_data;
  uint8_t note_hit;
};
//---------------------------------------------------------------------------


//===========================================================================
// pmf_mixer_buffer
//===========================================================================
struct pmf_mixer_buffer
{
  void *begin;
  unsigned num_samples;
};
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
class pmf_player
{
public:
  // construction and playback setup
  pmf_player();
  ~pmf_player();
  void load(const void *pmem_pmf_file_);
  void enable_playback_channels(uint8_t num_channels_);
  void set_row_callback(pmf_row_callback_t, void *custom_data_=0);
  void set_tick_callback(pmf_tick_callback_t, void *custom_data_=0);
  //-------------------------------------------------------------------------

  // PMF accessors
  uint8_t num_pattern_channels() const;
  uint8_t num_playback_channels() const;
  uint16_t playlist_length() const;
  //-------------------------------------------------------------------------

  // player control
  void start(uint32_t sampling_freq_=22050, uint16_t playlist_pos_=0);
  void stop();
  void update();
  //-------------------------------------------------------------------------

  // playback state accessors
  bool is_playing() const;
  uint8_t playlist_pos() const;
  uint8_t pattern_row() const;
  uint8_t pattern_speed() const;
  pmf_channel_info channel_info(uint8_t channel_idx_) const;
  //-------------------------------------------------------------------------

private:
  struct envelope_state;
  struct audio_channel;
  // platform specific functions (implemented in platform specific files)
  uint32_t get_sampling_freq(uint32_t sampling_freq_) const;
  void start_playback(uint32_t sampling_freq_);
  void stop_playback();
  void mix_buffer(pmf_mixer_buffer&, unsigned num_samples_);
  pmf_mixer_buffer get_mixer_buffer();
  // platform agnostic reference functions
  template<typename T, bool stereo=false, unsigned channel_bits=8> void mix_buffer_impl(pmf_mixer_buffer&, unsigned num_samples_);
  // audio effects
  void apply_channel_effect_volume_slide(audio_channel&);
  void apply_channel_effect_note_slide(audio_channel&);
  void apply_channel_effect_vibrato(audio_channel&);
  void apply_channel_effects();
  bool init_effect_volume_slide(audio_channel&, uint8_t effect_data_);
  bool init_effect_note_slide(audio_channel&, uint8_t slide_speed_, uint16_t target_note_pediod_);
  void init_effect_vibrato(audio_channel&, uint8_t vibrato_depth_, uint8_t vibrato_speed_);
  void evaluate_envelope(envelope_state&, uint16_t env_data_offs_, bool is_note_off_);
  void evaluate_envelopes();
  // pattern playback
  uint16_t get_note_period(uint8_t note_idx_, int16_t finetune_);
  int16_t get_sample_speed(uint16_t note_period_, bool forward_);
  void set_instrument(audio_channel&, uint8_t inst_idx_, uint8_t note_idx_);
  void hit_note(audio_channel&, uint8_t note_idx_, uint8_t sample_start_pos_, bool reset_sample_pos_);
  void process_pattern_row();
  void process_track_row(audio_channel&, uint8_t &note_idx_, uint8_t &inst_idx_, uint8_t &volume_, uint8_t &effect_, uint8_t &effect_data_);
  void init_pattern(uint8_t playlist_pos_, uint8_t row_=0);
  //-------------------------------------------------------------------------

  //=========================================================================
  // envelope_state
  //=========================================================================
  struct envelope_state
  {
    uint16_t tick;
    int8_t pos;
    uint16_t value;
  };
  //-------------------------------------------------------------------------

  //=========================================================================
  // audio_channel
  //=========================================================================
  struct audio_channel
  {
    // track state
    const uint8_t *track_pos;
    const uint8_t *track_loop_pos;
    uint8_t track_bit_pos;
    uint8_t track_loop_bit_pos;
    uint8_t decomp_type;
    uint8_t decomp_buf[6][2];
    uint8_t track_loop_decomp_buf[6][2];
    // visualization
    uint8_t note_hit;              // note hit
    // sample playback
    const uint8_t *inst_metadata;
    const uint8_t *smp_metadata;
    uint32_t sample_pos;           // sample position (24.8 fp)
    int16_t sample_speed;          // sample speed (8.8 fp)
    int16_t sample_finetune;       // sample finetune (9.7 fp)
    uint16_t note_period;          // current note period
    uint8_t sample_volume;         // sample volume (0.8 fp)
    int8_t sample_panning;         // sample panning (-127=left, 0=center, 127=right, -128=surround)
    uint8_t base_note_idx;         // base note index
    int8_t inst_note_idx_offs;     // instrument note offset
    // sound effects
    uint8_t effect;                // current effect
    uint8_t effect_data;           // current effect data
    uint8_t vol_effect;            // current volume effect
    int8_t fxmem_panning_spd;      // panning
    uint8_t fxmem_arpeggio;        // arpeggio
    uint8_t fxmem_note_slide_spd;  // note slide speed
    uint16_t fxmem_note_slide_prd; // note slide target period
    uint8_t fxmem_vol_slide_spd;   // volume slide speed & type
    uint8_t fxmem_vibrato_spd;     // vibrato speed
    uint8_t fxmem_vibrato_depth;   // vibrato depth
    uint8_t fxmem_vibrato_wave;    // vibrato waveform index & retrigger bit
    int8_t fxmem_vibrato_pos;      // vibrato wave pos
    uint8_t fxmem_retrig_count;    // sample retrigger count
    uint8_t fxmem_note_delay_idx;  // note delay note index
    uint16_t vol_fadeout;          // fadeout volume
    envelope_state vol_env;        // volume envelope
    envelope_state pitch_env;      // pitch envelope
  };
  //-------------------------------------------------------------------------

  // PMF info
  const uint8_t *m_pmf_file;
  uint32_t m_sampling_freq;
  pmf_row_callback_t m_row_callback;
  void *m_row_callback_custom_data;
  pmf_tick_callback_t m_tick_callback;
  void *m_tick_callback_custom_data;
  uint16_t m_pmf_flags;  // e_pmf_flags
  uint16_t m_note_period_min;
  uint16_t m_note_period_max;
  uint8_t m_note_slide_speed;
  uint8_t m_num_pattern_channels;
  uint8_t m_num_instruments;
  uint8_t m_num_samples;
  // audio channel states
  uint8_t m_num_playback_channels;
  uint8_t m_num_processed_pattern_channels;
  audio_channel m_channels[pmfplayer_max_channels];
  // audio buffer state
  uint16_t m_num_batch_samples;
  uint16_t m_batch_pos;
  // pattern playback state
  uint16_t m_current_pattern_playlist_pos;
  uint8_t m_current_pattern_last_row;
  uint8_t m_current_pattern_row_idx;
  uint8_t m_current_row_tick;
  uint8_t m_speed;
  uint8_t m_arpeggio_counter;
  uint8_t m_pattern_delay;
  uint8_t m_pattern_loop_cnt;
  uint8_t m_pattern_loop_row_idx;
};
//---------------------------------------------------------------------------

template<typename T, bool stereo, unsigned channel_bits>
void pmf_player::mix_buffer_impl(pmf_mixer_buffer &buf_, unsigned num_samples_)
{
  audio_channel *channel=m_channels, *channel_end=channel+m_num_playback_channels;
  do
  {
    // check for active channel
    if(!channel->sample_speed)
      continue;

    // get channel attributes
    size_t sample_addr=(size_t)(m_pmf_file+pgm_read_dword(channel->smp_metadata+pmfcfg_offset_smp_data));
    uint32_t sample_pos=channel->sample_pos;
    int16_t sample_speed=channel->sample_speed;
    uint32_t sample_end=uint32_t(pgm_read_dword(channel->smp_metadata+pmfcfg_offset_smp_length))<<8;
    uint32_t sample_loop_len=(pgm_read_dword(channel->smp_metadata+pmfcfg_offset_smp_loop_length_and_panning)&0xffffff)<<8;
    uint8_t sample_volume=(channel->sample_volume*(channel->vol_env.value>>8))>>8;
    uint32_t sample_pos_offs=sample_end-sample_loop_len;
    if(sample_pos<sample_pos_offs)
      sample_pos_offs=0;
    sample_addr+=sample_pos_offs>>8;
    sample_pos-=sample_pos_offs;
    sample_end-=sample_pos_offs;

    // setup panning
    int8_t panning=channel->sample_panning;
    int16_t sample_phase_shift=panning==-128?0xffff:0;
    panning&=~int8_t(sample_phase_shift);
    uint8_t sample_volume_l=uint8_t((uint16_t(sample_volume)*uint8_t(128-panning))>>8);
    uint8_t sample_volume_r=uint8_t((uint16_t(sample_volume)*uint8_t(128+panning))>>8);

    // mix channel to the buffer
    T *buf=(T*)buf_.begin, *buffer_end=buf+num_samples_*(stereo?2:1);
    do
    {
      // get sample data and adjust volume
#if PMF_USE_LINEAR_INTERPOLATION==1
      uint16_t smp_data=((uint16_t)pgm_read_word(sample_addr+(sample_pos>>8)));
      uint8_t sample_pos_frc=sample_pos&255;
      int16_t smp=((int16_t(int8_t(smp_data&255))*(256-sample_pos_frc))>>8)+((int16_t(int8_t(smp_data>>8))*sample_pos_frc)>>8);
#else
      int16_t smp=(int8_t)pgm_read_byte(sample_addr+(sample_pos>>8));
#endif

      // mix the result to the audio buffer (the if-branch with compile-time constant will be optimized out)
      if(stereo)
      {
        (*buf++)+=T(sample_volume_l*smp)>>(16-channel_bits);
        (*buf++)+=T(sample_volume_r*(smp^sample_phase_shift))>>(16-channel_bits);
      }
      else
        (*buf++)+=T(sample_volume*smp)>>(16-channel_bits);

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
        if(pgm_read_byte(channel->smp_metadata+pmfcfg_offset_smp_flags)&pmfsmpflag_bidi_loop)
        {
          sample_pos-=sample_speed*2;
          channel->sample_speed=sample_speed=-sample_speed;
        }
        else
          sample_pos-=sample_loop_len;
      }
    } while(buf<buffer_end);
    channel->sample_pos=sample_pos+sample_pos_offs;
  } while(++channel!=channel_end);

  // advance buffer
  ((T*&)buf_.begin)+=num_samples_*(stereo?2:1);
  buf_.num_samples-=num_samples_;
}
//---------------------------------------------------------------------------


//===========================================================================
// pmf_audio_buffer
//===========================================================================
template<typename T, unsigned buffer_size>
struct pmf_audio_buffer
{
  // construction & accessors
  pmf_audio_buffer();
  void reset();
  template<typename U, unsigned sample_bits> U read_sample();
  pmf_mixer_buffer get_mixer_buffer();
  //-------------------------------------------------------------------------

  enum {buf_size=buffer_size};
  enum {subbuf_size=buffer_size/2};
  volatile uint16_t playback_pos;
  uint8_t subbuf_write_idx;
  T buffer[buffer_size];
};
//---------------------------------------------------------------------------

template<typename T, unsigned buffer_size>
pmf_audio_buffer<T, buffer_size>::pmf_audio_buffer()
{
  reset();
}
//----

template<typename T, unsigned buffer_size>
void pmf_audio_buffer<T, buffer_size>::reset()
{
  playback_pos=0;
  subbuf_write_idx=1;
  memset(buffer, 0, sizeof(buffer));
}
//----

template<typename T, unsigned buffer_size>
template<typename U, unsigned sample_bits>
U pmf_audio_buffer<T, buffer_size>::read_sample()
{
  // read sample from the buffer and clip to given number of bits
  enum {sample_range=1<<sample_bits};
  enum {max_sample_val=sample_range-1};
  uint16_t pbpos=playback_pos;
  T *smp_addr=buffer+pbpos;
  U smp=U(*smp_addr+(sample_range>>1));
  *smp_addr=0;
  if(smp>sample_range-1)
    smp=smp>((U(-1)>>1)+(sample_range>>1))?0:max_sample_val;
  if(++pbpos==buffer_size)
    pbpos=0;
  playback_pos=pbpos;
  return smp;
}
//----

template<typename T, unsigned buffer_size>
pmf_mixer_buffer pmf_audio_buffer<T, buffer_size>::get_mixer_buffer()
{
  // return buffer for mixing if available (i.e. not playing the one for writing)
  uint16_t pbpos=playback_pos; // note: atomic read thus no need to disable interrupts
  pmf_mixer_buffer buf={0, 0};
  if(subbuf_write_idx^(pbpos<subbuf_size))
    return buf;
  buf.begin=buffer+subbuf_write_idx*subbuf_size;
  buf.num_samples=subbuf_size;
  subbuf_write_idx^=1;
  return buf;
}
//---------------------------------------------------------------------------

//============================================================================
#endif

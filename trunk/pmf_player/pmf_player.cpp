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
#include "pmf_data.h"
//---------------------------------------------------------------------------


//============================================================================
// pmf_header
//============================================================================
struct pmf_header
{
  char signature[4];
  uint16_t version;
  uint16_t flags; // e_pmf_flag
  uint32_t file_size;
  uint8_t initial_speed;
  uint8_t initial_tempo;
  uint16_t playlist_length;
  uint8_t num_channels;
  uint8_t num_patterns;
  uint8_t num_instruments;
  uint8_t first_playlist_entry;
};
//----------------------------------------------------------------------------


//===========================================================================
// PMF format config
//===========================================================================
// PMF config
// PMF file structure
enum {pmfcfg_offset_flags=PFC_OFFSETOF(pmf_header, flags)};
enum {pmfcfg_offset_init_speed=PFC_OFFSETOF(pmf_header, initial_speed)};
enum {pmfcfg_offset_init_tempo=PFC_OFFSETOF(pmf_header, initial_tempo)};
enum {pmfcfg_offset_playlist_length=PFC_OFFSETOF(pmf_header, playlist_length)};
enum {pmfcfg_offset_num_channels=PFC_OFFSETOF(pmf_header, num_channels)};
enum {pmfcfg_offset_num_patterns=PFC_OFFSETOF(pmf_header, num_patterns)};
enum {pmfcfg_offset_num_instruments=PFC_OFFSETOF(pmf_header, num_instruments)};
enum {pmfcfg_offset_playlist=PFC_OFFSETOF(pmf_header, first_playlist_entry)};
enum {pmfcfg_pattern_metadata_header_size=2};
enum {pmfcfg_pattern_metadata_track_offset_size=2};
enum {pmfcfg_offset_pattern_metadata_last_row=0};
enum {pmfcfg_offset_pattern_metadata_track_offsets=2};
// envelope configs
enum {pmfcfg_offset_env_num_points=0};
enum {pmfcfg_offset_env_sustain_loop_start=1};
enum {pmfcfg_offset_env_sustain_loop_end=2};
enum {pmfcfg_offset_env_loop_start=3};
enum {pmfcfg_offset_env_loop_end=4};
enum {pmfcfg_offset_env_points=5};
enum {pmfcfg_offset_env_point_pos=0};
enum {pmfcfg_offset_env_point_val=1};
//enum {pmfcfg_envelope_header_size=5};
enum {pmfcfg_envelope_point_size=2};
// bit-compression settings
enum {pmfcfg_num_data_mask_bits=4};
enum {pmfcfg_num_note_bits=7};       // max 10 octaves (0-9) (12*10=120)
enum {pmfcfg_num_instrument_bits=5}; // max 32 instruments
enum {pmfcfg_num_volume_bits=6};     // volume range [0, 63]
enum {pmfcfg_num_effect_bits=4};     // effects 0-15
enum {pmfcfg_num_effect_data_bits=8};
// PMF flags
enum e_pmf_flags
{
  pmfflag_fast_note_slides   =0x01,  // regular note slide speeds are multiplied by 4
  pmfflag_linear_freq_table  =0x02,  // 0=Amiga, 1=linear
};
// PMF special notes
enum {pmfcfg_note_cut=120};
enum {pmfcfg_note_off=121};
// PMF effects
enum {num_subfx_value_bits=4};
enum {subfx_value_mask=~(-1<<num_subfx_value_bits)};
enum e_pmf_effect
{
  // global control
  pmffx_set_speed_tempo,   // [1, 255]
  pmffx_position_jump,     // [0, song_len-1]
  pmffx_pattern_break,     // [0, 255]
  // channel effects
  pmffx_volume_slide,      // [00xxyyyy], x=slide type (0=slide down, 1=slide up, 2=fine slide down, 3=fine slide up), y=slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_note_slide_down,   // [1, 0xdf] = normal slide, [0xe0, 0xef] = extra fine slide, [0xf0, 0xff] = fine slide, 0=use previous slide value
  pmffx_note_slide_up,     // [1, 0xdf] = normal slide, [0xe0, 0xef] = extra fine slide, [0xf0, 0xff] = fine slide, 0=use previous slide value
  pmffx_note_slide,        // [1, 0xdf] = slide, 0=use previous slide value, [0xe0, 0xff]=unused
  pmffx_arpeggio,          // x=[0, 15], y=[0, 15]
  pmffx_vibrato,           // [xxxxyyyy], x=vibrato speed, y=vibrato depth
  pmffx_note_vol_slide,    // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_vibrato_vol_slide, // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_retrig_vol_slide,  // [xxxxyyyy], x=volume slide param, y=sample retrigger frequency
  pmffx_set_sample_offset, // [xxxxxxxx], offset=x*256
  pmffx_subfx,             // [xxxxyyyy], x=sub-effect, y=sub-effect value
};
enum e_pmf_subfx
{
  pmfsubfx_set_finetune,     // [-8, 7]
  pmfsubfx_set_vibrato_wave, // [0xyy], x=[0=retrigger, 1=no retrigger], yy=vibrato wave=[0=sine, 1=ramp down, 2=square, 3=random]
  pmfsubfx_pattern_delay,    // [1, 15]
  pmfsubfx_loop_pattern,     // [0, 15], 0=set loop start
};
enum e_pmfx_vslide_type
{
  pmffx_vslidetype_down      =0x00,
  pmffx_vslidetype_up        =0x10,
  pmffx_vslidetype_fine_down =0x20,
  pmffx_vslidetype_fine_up   =0x30,
  //----
  pmffx_vslidetype_mask      =0x30
};
// waveform tables
static const uint8_t PROGMEM s_waveforms[3][32]=
{
  {0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126, 127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12}, // sine-wave
  {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124}, // saw-wave
  {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127} // square-wave
};
//---------------------------------------------------------------------------


//===========================================================================
// PMF note periods
//===========================================================================
enum {min_period=28, max_period=27392};
//---------------------------------------------------------------------------


//===========================================================================
// local helper functions
//===========================================================================
namespace
{
  //=========================================================================
  // read bits
  //=========================================================================
  uint8_t read_bits(const uint8_t *&ptr_, uint8_t &bit_pos_, uint8_t num_bits_)
  {
    // read bits from the bit stream
    uint8_t v=pgm_read_word(ptr_)>>bit_pos_;
    bit_pos_+=num_bits_;
    if(bit_pos_>7)
    {
      ++ptr_;
      bit_pos_-=8;
    }
    return v;
  }
  //-------------------------------------------------------------------------

  //=========================================================================
  // exp2
  //=========================================================================
  float exp2(float x_)
  {
    // map x_ to range [0, 0.5]
    int adjustment=0;
    uint8_t int_arg=uint8_t(x_);
    x_-=int_arg;
    if(x_>0.5f)
    {
      adjustment=1;
      x_-=0.5f;
    }

    // calculate 2^x_ approximation
    float x2=x_*x_;
    float Q=20.8189237930062f+x2;
    float x_P=x_*(7.2152891521493f+0.0576900723731f*x2);
    float res=(1<<int_arg)*(Q+x_P)/(Q-x_P);
    if(adjustment)
      res*=1.4142135623730950488f;
    return res;
  }
  //-------------------------------------------------------------------------

  //=========================================================================
  // get_note_period
  //=========================================================================
  uint16_t get_note_period(uint8_t note_idx_, uint8_t flags_)
  {
    if(flags_&pmfflag_linear_freq_table)
      return 7680-note_idx_*64;
    return uint16_t(27392.0f/exp2(note_idx_/12.0f)+0.5f);
  }
  //-------------------------------------------------------------------------

  //=========================================================================
  // get_sample_speed
  //=========================================================================
  uint16_t get_sample_speed(uint16_t note_period_, uint16_t c4hz_, uint8_t flags_)
  {
    if(flags_&pmfflag_linear_freq_table)
      return uint16_t((c4hz_*32.0f/pmfplayer_sampling_rate)*exp2(float(7680-note_period_)/768.0f)+0.5f);
    return uint16_t((c4hz_*1024.0f*7093789.2/pmfplayer_sampling_rate)/(8363.0f*note_period_)+0.5f);
  }
} // namespace <anonymous>
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
pmf_player::pmf_player()
{
}
//----

pmf_player::~pmf_player()
{
  stop();
}
//---------------------------------------------------------------------------

void pmf_player::start(const void *pmem_pmf_file_)
{
  // read PMF properties
  m_pmf_file=static_cast<const uint8_t*>(pmem_pmf_file_);
  m_num_pattern_channels=pgm_read_byte(m_pmf_file+pmfcfg_offset_num_channels);
  m_num_playback_channels=m_num_pattern_channels<pmfplayer_max_channels?m_num_pattern_channels:pmfplayer_max_channels;
  m_flags=pgm_read_word(m_pmf_file+pmfcfg_offset_flags);
  m_pmf_instrument_meta=m_pmf_file+((pmfcfg_offset_playlist+pgm_read_word(m_pmf_file+pmfcfg_offset_playlist_length)+3)&-4);
  m_pmf_pattern_meta=m_pmf_instrument_meta+sizeof(pmf_instrument_header)*pgm_read_byte(m_pmf_file+pmfcfg_offset_num_instruments);
  m_note_slide_speed=m_flags&pmfflag_fast_note_slides?4:2;

  // initialize channels
  memset(m_channels, 0, sizeof(m_channels));
  for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];
    chl.fxmem_vibrato_wave=0x04;
  }

  // init playback state
  init_pattern(0);
  m_speed=pgm_read_byte(m_pmf_file+pmfcfg_offset_init_speed);
  m_num_batch_samples=(long(pmfplayer_sampling_rate)*125)/long(pgm_read_byte(m_pmf_file+pmfcfg_offset_init_tempo)*50);
  m_current_row_tick=m_speed-1;
  m_arpeggio_counter=0;
  m_pattern_delay=1;

  // start playback
  m_batch_pos=0;
  start_playback();
}
//----

void pmf_player::stop()
{
  stop_playback();
}
//----

void pmf_player::update()
{
  // check if audio buffer should be updated
  mixer_buffer subbuffer=get_mixer_buffer();
  if(!subbuffer.num_samples)
    return;

  // update audio buffer
  do
  {
    // mix batch of samples
    uint16_t batch_left=m_num_batch_samples-m_batch_pos;
    unsigned num_samples=min(subbuffer.num_samples, batch_left);
    mix_buffer(subbuffer, num_samples);
    m_batch_pos+=num_samples;

    // check for new batch
    if(m_batch_pos==m_num_batch_samples)
    {
      if(++m_current_row_tick==m_speed)
      {
        if(!--m_pattern_delay)
        {
          m_pattern_delay=1;
          process_pattern_row();
        }
        m_current_row_tick=0;
      }
      else
        apply_channel_effects();
      evaluate_envelopes();
      visualize_pattern_frame();
      m_batch_pos=0;
    }
  } while(subbuffer.num_samples);
}
//---------------------------------------------------------------------------

void pmf_player::apply_channel_effects()
{
  m_arpeggio_counter=(m_arpeggio_counter+1)%3;
  for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
  {
    // apply active effect
    audio_channel &chl=m_channels[ci];
    switch(chl.effect)
    {
      case pmffx_arpeggio:
      {
        // alternate between 3 periods defined by arpeggio parameters
        uint16_t note_period=get_note_period((chl.base_note_idx&127)+((chl.effect_data>>(4*m_arpeggio_counter))&0xf), m_flags);
        chl.sample_speed=get_sample_speed(note_period, chl.sample_c4hz, m_flags);
      } break;

      case pmffx_note_slide_up:
      {
        // slide note period down to minimum period
        chl.note_period-=int(chl.effect_data)*m_note_slide_speed;
        if(int16_t(chl.note_period)<min_period)
          chl.note_period=min_period;
        chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_c4hz, m_flags);
      } break;

      case pmffx_note_slide_down:
      {
        // slide note period up to max period
        chl.note_period+=int(chl.effect_data)*m_note_slide_speed;
        if(chl.note_period>max_period)
          chl.note_period=max_period;
        chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_c4hz, m_flags);
      } break;

      case pmffx_note_slide:
      case pmffx_note_vol_slide:
      {
        // slide note period towards target period and clamp the period to target
        int16_t note_period=chl.note_period;
        int16_t note_target_prd=chl.fxmem_note_slide_prd;
        int16_t slide_spd=(note_period<note_target_prd?chl.fxmem_note_slide_spd:-chl.fxmem_note_slide_spd)*m_note_slide_speed;
        note_period+=slide_spd;
        chl.note_period=((slide_spd>0)^(note_period<note_target_prd))?note_target_prd:note_period;
        chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_c4hz, m_flags);

        // check for additional volume slide
        if(chl.effect==pmffx_note_vol_slide)
          goto pmffx_volume_slide;
      } break;

      case pmffx_volume_slide:
      {
        // slide volume either up/down defined by effect
        pmffx_volume_slide:
        int8_t vdelta=(chl.fxmem_vol_slide_spd&0xf)<<2;
        int16_t v=int16_t(chl.sample_volume)+((chl.fxmem_vol_slide_spd&pmffx_vslidetype_mask)==pmffx_vslidetype_down?-vdelta:vdelta);
        chl.sample_volume=v<0?0:v>255?255:v;
      } break;

      case pmffx_vibrato:
      case pmffx_vibrato_vol_slide:
      {
        uint8_t wave_idx=chl.fxmem_vibrato_wave&3;
        int8_t vibrato_pos=chl.fxmem_vibrato_pos;
        int8_t wave_sample=vibrato_pos<0?-int8_t(pgm_read_byte(&s_waveforms[wave_idx][32-vibrato_pos])):pgm_read_byte(&s_waveforms[wave_idx][vibrato_pos]);
        chl.sample_speed=get_sample_speed(chl.note_period+(int16_t(wave_sample*chl.fxmem_vibrato_depth)>>8), chl.sample_c4hz, m_flags);
        if((chl.fxmem_vibrato_pos+=chl.fxmem_vibrato_spd)>31)
          chl.fxmem_vibrato_pos-=64;

        // check for additional volume slide
        if(chl.effect==pmffx_vibrato_vol_slide)
          goto pmffx_volume_slide;
      } break;

      case pmffx_retrig_vol_slide:
      {
        if(!--chl.fxmem_retrig_count)
        {
          uint8_t effect_data=chl.effect_data;
          chl.fxmem_retrig_count=effect_data&0xf;
          int vol=chl.sample_volume;
          effect_data>>=4;
          switch(effect_data)
          {
            case  6: vol=(vol+vol)/3; break;
            case  7: vol>>=1; break;
            case 14: vol=(vol*3)/2; break;
            case 15: vol+=vol; break;
            default:
            {
              uint8_t delta=2<<(effect_data&7);
              vol+=effect_data&8?+delta:-delta;
            }
          }
          chl.sample_volume=vol<0?0:vol>255?255:vol;
          chl.sample_pos=0;
          chl.note_hit=1;
        }
      } break;
    }
  }
}
//----

void pmf_player::evaluate_envelopes()
{
  // evaluate channel envelopes
  for(uint8_t ci=0; ci<m_num_playback_channels; ++ci)
  {
    // check fadeout
    audio_channel &chl=m_channels[ci];
    uint8_t volume=255;
    if(uint16_t env_offset=pgm_read_word(chl.inst_metadata+pmfcfg_offset_inst_vol_env))
    {
      // advance envelope
      const uint8_t *envelope=m_pmf_file+env_offset;
      const uint8_t *env_span_data=envelope+pmfcfg_offset_env_points+chl.vol_env_pos*pmfcfg_envelope_point_size;
      uint8_t env_span_pos_end=pgm_read_byte(env_span_data+pmfcfg_offset_env_point_pos+pmfcfg_envelope_point_size);
      if(++chl.vol_env_tick>env_span_pos_end)
      {
        // get envelope start and end points (sustain/loop/none)
        uint8_t env_last_pnt=pgm_read_byte(envelope+pmfcfg_offset_env_num_points)-1;
        uint8_t loop_pnt_start=pgm_read_byte(envelope+pmfcfg_offset_env_loop_start);
        uint8_t env_pnt_start=min(env_last_pnt, loop_pnt_start);
        uint8_t loop_pnt_end=pgm_read_byte(envelope+pmfcfg_offset_env_loop_end);
        uint8_t env_pnt_end=min(env_last_pnt, loop_pnt_end);
        if(!(chl.base_note_idx&0x80))
        {
          uint8_t sustain_loop_pnt_start=pgm_read_byte(envelope+pmfcfg_offset_env_sustain_loop_start);
          uint8_t sustain_loop_pnt_end=pgm_read_byte(envelope+pmfcfg_offset_env_sustain_loop_end);
          env_pnt_start=min(env_pnt_start, sustain_loop_pnt_start);
          env_pnt_end=min(env_pnt_end, sustain_loop_pnt_end);
        }

        // check for envelope end/loop-end
        if(++chl.vol_env_pos==env_pnt_end)
        {
          if(env_pnt_start<env_pnt_end)
          {
            chl.vol_env_pos=env_pnt_start;
            chl.vol_env_tick=pgm_read_byte(envelope+pmfcfg_offset_env_points+pmfcfg_offset_env_point_pos+env_pnt_start*pmfcfg_envelope_point_size);
          }
          else
          {
            chl.vol_env_pos=env_pnt_start-1;
            chl.vol_env_tick=pgm_read_byte(envelope+pmfcfg_offset_env_points+pmfcfg_offset_env_point_pos+env_pnt_start*pmfcfg_envelope_point_size);
          }
        }
        env_span_data=envelope+pmfcfg_offset_env_points+chl.vol_env_pos*pmfcfg_envelope_point_size;
      }

      // linearly interpolate envelope value
      uint8_t env_span_pos_start=pgm_read_byte(env_span_data+pmfcfg_offset_env_point_pos);
      uint8_t env_span_val_start=pgm_read_byte(env_span_data+pmfcfg_offset_env_point_val);
      uint8_t env_span_val_end=pgm_read_byte(env_span_data+pmfcfg_offset_env_point_val+pmfcfg_envelope_point_size);
      float span_pos=float(chl.vol_env_tick-env_span_pos_start)/float(env_span_pos_end-env_span_pos_start);
      volume=env_span_val_start+int16_t(span_pos*(int16_t(env_span_val_end)-int16_t(env_span_val_start)));
    }

    // check volume fadeout
    if(chl.base_note_idx&0x80)
    {
      uint16_t fadeout_speed=pgm_read_word(chl.inst_metadata+pmfcfg_offset_inst_fadeout_speed);
      volume=(volume*(chl.vol_fadeout>>8))>>8;
      chl.vol_fadeout=chl.vol_fadeout>fadeout_speed?chl.vol_fadeout-fadeout_speed:0;
    }

    chl.vol_env_value=volume;
  }
}
//----

void pmf_player::process_pattern_row()
{
  // store current track positions
  const uint8_t *current_track_poss[pmfplayer_max_channels];
  uint8_t current_track_bit_poss[pmfplayer_max_channels];
  for(uint8_t ci=0; ci<m_num_playback_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];
    current_track_poss[ci]=chl.track_pos;
    current_track_bit_poss[ci]=chl.track_bit_pos;
  }

  // parse row in the music pattern
  bool loop_pattern=false;
  uint8_t num_skip_rows=0;
  for(uint8_t ci=0; ci<m_num_playback_channels; ++ci)
  {
    // get note, instrument, volume and effect for the channel
    audio_channel &chl=m_channels[ci];
    uint8_t note_idx=0xff, inst_idx=0xff, volume=0xff, effect=0xff, effect_data, sample_start_pos=0;
    process_track_row(chl, note_idx, inst_idx, volume, effect, effect_data);

    // check for note cut/off
    if(note_idx==pmfcfg_note_cut)
    {
      // stop sample playback
      chl.sample_speed=0;
      note_idx=0xff;
    }
    else if(note_idx==pmfcfg_note_off)
    {
      // release note
      chl.base_note_idx|=128;
      chl.vol_fadeout=65535;
      note_idx=0xff;
    }

    // check for instrument
    if(inst_idx!=0xff)
    {
      // setup channel instrument
      const uint8_t *inst_metadata=m_pmf_instrument_meta+inst_idx*pmfcfg_instrument_metadata_size;
      chl.inst_metadata=inst_metadata;
      chl.sample_volume=pgm_read_byte(inst_metadata+pmfcfg_offset_inst_volume);
      chl.sample_c4hz=pgm_read_word(inst_metadata+pmfcfg_offset_inst_c4hz);
    }

    // check for volume
    if(volume!=0xff)
      chl.sample_volume=(volume<<2)|(volume>>4);

    // get effect
    chl.effect=0xff;
    if(effect!=0xff)
    {
      // setup effect
      switch(effect)
      {
        case pmffx_set_speed_tempo:
        {
          if(effect_data<=32)
            m_speed=effect_data;
          else
            m_num_batch_samples=(long(pmfplayer_sampling_rate)*125)/long(effect_data*50);
        } break;

        case pmffx_position_jump:
        {
          m_current_pattern_playlist_pos=effect_data-1;
          m_current_pattern_row_idx=m_current_pattern_last_row;
        } break;

        case pmffx_pattern_break:
        {
          m_current_pattern_row_idx=m_current_pattern_last_row;
          num_skip_rows=effect_data;
        } break;

        case pmffx_arpeggio:
        {
          chl.effect=pmffx_arpeggio;
          chl.effect_data=effect_data;
        } break;

        case pmffx_note_slide_down:
        {
          // check for slide data or read from effect memory
          if(effect_data)
            chl.effect_data=effect_data;
          else
            effect_data=chl.effect_data;

          // check for extra-fine/fine slide
          if(effect_data>=0xe0)
          {
            chl.note_period+=effect_data>=0xf0?(effect_data-0xf0)*4:(effect_data-0xe0);
            if(chl.note_period>max_period)
              chl.note_period=max_period;
            if(chl.sample_speed)
              chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_c4hz, m_flags);
          }
          else
          {
            // regular note slide down
            chl.effect=pmffx_note_slide_down;
          }
        } break;

        case pmffx_note_slide_up:
        {
          // check for slide data or read from effect memory
          if(effect_data)
            chl.effect_data=effect_data;
          else
            effect_data=chl.effect_data;

          // check for extra-fine/fine slide
          if(effect_data>=0xe0)
          {
            chl.note_period-=effect_data>=0xf0?(effect_data-0xf0)*4:(effect_data-0xe0);
            if(int16_t(chl.note_period)<min_period)
              chl.note_period=min_period;
            if(chl.sample_speed)
              chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_c4hz, m_flags);
          }
          else
          {
            // regular note slide up
            chl.effect=pmffx_note_slide_up;
          }
        } break;

        case pmffx_note_slide:
        {
          // update note slide effect memory
          if(effect_data)
            chl.fxmem_note_slide_spd=effect_data;
          if(note_idx!=0xff)
            chl.fxmem_note_slide_prd=get_note_period(note_idx, m_flags);

          // disable note retrigger
          chl.effect=pmffx_note_slide;
          note_idx=0xff;
        } break;

        case pmffx_note_vol_slide:
        case pmffx_vibrato_vol_slide:
        case pmffx_volume_slide:
        {
          // check for volume slide data or read from effect memory
          if(effect_data&0xf)
            chl.fxmem_vol_slide_spd=effect_data;
          effect_data=chl.fxmem_vol_slide_spd;

          // init proper slide
          uint8_t fx_type=effect_data&pmffx_vslidetype_mask;
          if(fx_type==pmffx_vslidetype_fine_down || fx_type==pmffx_vslidetype_fine_up)
          {
            // fine slide
            int8_t vdelta=(effect_data&0xf)<<2;
            int16_t v=int16_t(chl.sample_volume)+(fx_type==pmffx_vslidetype_fine_down?-vdelta:vdelta);
            chl.sample_volume=v<0?0:v>255?255:v;
          }
          else
          {
            // regular volume slide
            chl.effect=effect;
          }
        } break;

        case pmffx_vibrato:
        {
          // update vibrato attributes
          uint8_t vdepth=effect_data&0x0f;
          if(vdepth)
            chl.fxmem_vibrato_depth=vdepth<<3;
          if(effect_data&0xf0)
            chl.fxmem_vibrato_spd=effect_data>>4;

          // check for vibrato retrigger
          if(chl.fxmem_vibrato_wave&0x4)
          {
            uint8_t wave_idx=chl.fxmem_vibrato_wave&3;
            int8_t vibrato_pos=chl.fxmem_vibrato_pos;
            int8_t wave_sample=vibrato_pos<0?-int8_t(pgm_read_byte(&s_waveforms[wave_idx][32-vibrato_pos])):pgm_read_byte(&s_waveforms[wave_idx][vibrato_pos]);
            if(chl.sample_speed)
              chl.sample_speed=get_sample_speed(chl.note_period+(int16_t(wave_sample*chl.fxmem_vibrato_depth)>>8), chl.sample_c4hz, m_flags);
            if((chl.fxmem_vibrato_pos+=chl.fxmem_vibrato_spd)>31)
              chl.fxmem_vibrato_pos-=64;
          }
          else
            chl.fxmem_vibrato_pos=0;
          chl.effect=pmffx_vibrato;
        } break;

        case pmffx_retrig_vol_slide:
        {
          chl.sample_pos=0;
          chl.fxmem_retrig_count=effect_data&0xf;
          chl.effect=pmffx_retrig_vol_slide;
          chl.effect_data=effect_data;
          chl.note_hit=1;
        } break;

        case pmffx_set_sample_offset:
        {
          sample_start_pos=effect_data;
          chl.sample_pos=effect_data*long(65536);
        } break;

        case pmffx_subfx:
        {
          switch(effect_data>>4)
          {
            case pmfsubfx_set_finetune:
            {
              if(inst_idx!=0xff)
              {
                //todo: should move C4hz values of instruments to RAM
              }
            } break;

            case pmfsubfx_set_vibrato_wave:
            {
              uint8_t wave=effect_data&3;
              chl.fxmem_vibrato_wave=(wave<3?wave:m_batch_pos%3)|(effect_data&4);
            } break;

            case pmfsubfx_pattern_delay:
            {
              m_pattern_delay=(effect_data&0xf)+1;
            } break;

            case pmfsubfx_loop_pattern:
            {
              effect_data&=0xf;
              if(effect_data)
              {
                if(m_pattern_loop_cnt)
                  --m_pattern_loop_cnt;
                else
                  m_pattern_loop_cnt=effect_data;
                if(m_pattern_loop_cnt)
                  loop_pattern=true;
              }
              else
              {
                // set loop start
                m_pattern_loop_row_idx=m_current_pattern_row_idx;
                for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
                {
                  audio_channel &chl=m_channels[ci];
                  chl.track_loop_pos=current_track_poss[ci];
                  chl.track_loop_bit_pos=current_track_bit_poss[ci];
                  memcpy(chl.track_loop_decomp_buf, chl.decomp_buf, sizeof(chl.track_loop_decomp_buf));
                }
              }
            } break;
          }
        } break;
      }
    }

    // check for note hit
    if(note_idx!=0xff)
    {
      chl.note_period=get_note_period(note_idx, m_flags);
      chl.base_note_idx=note_idx;
      chl.sample_pos=sample_start_pos*long(65536);
      chl.sample_speed=get_sample_speed(chl.note_period, chl.sample_c4hz, m_flags);
      chl.note_hit=1;
      chl.vol_env_tick=0;
      chl.vol_env_pos=-1;
    }
  }

  // check for pattern loop
  if(loop_pattern)
  {
    for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
    {
      audio_channel &chl=m_channels[ci];
      chl.track_pos=chl.track_loop_pos;
      chl.track_bit_pos=chl.track_loop_bit_pos;
      memcpy(chl.decomp_buf, chl.track_loop_decomp_buf, sizeof(chl.decomp_buf));
    }
    m_current_pattern_row_idx=m_pattern_loop_row_idx-1;
  }

  // advance pattern
  if(m_current_pattern_row_idx++==m_current_pattern_last_row)
  {
    // proceed to the next pattern
    if(++m_current_pattern_playlist_pos==pgm_read_word(m_pmf_file+pmfcfg_offset_playlist_length))
      m_current_pattern_playlist_pos=0;
    init_pattern(m_current_pattern_playlist_pos, num_skip_rows);
  }
}
//----

void pmf_player::process_track_row(audio_channel &chl_, uint8_t &note_idx_, uint8_t &inst_idx_, uint8_t &volume_, uint8_t &effect_, uint8_t &effect_data_)
{
  // get data mask
  uint8_t data_mask=0;
  bool read_dmask=false;
  switch(chl_.decomp_type&0x03)
  {
    case 0x0: read_dmask=true; break;
    case 0x1: read_dmask=read_bits(chl_.track_pos, chl_.track_bit_pos, 1)&1; break;
    case 0x2:
    {
      switch(read_bits(chl_.track_pos, chl_.track_bit_pos, 2)&3)
      {
        case 0x1: read_dmask=true; break;
        case 0x2: data_mask=chl_.decomp_buf[5][0]; break;
        case 0x3: data_mask=chl_.decomp_buf[5][1]; break;
      }
    } break;
  }
  if(read_dmask)
  {
    data_mask=read_bits(chl_.track_pos, chl_.track_bit_pos, chl_.decomp_type&4?8:4)&(chl_.decomp_type&4?0xff:0x0f);
    chl_.decomp_buf[5][1]=chl_.decomp_buf[5][0];
    chl_.decomp_buf[5][0]=data_mask;
  }

  // get note
  switch(data_mask&0x11)
  {
    case 0x01:
    {
      note_idx_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_note_bits)&((1<<pmfcfg_num_note_bits)-1);
      chl_.decomp_buf[0][1]=chl_.decomp_buf[0][0];
      chl_.decomp_buf[0][0]=note_idx_;
    } break;
    case 0x10: note_idx_=chl_.decomp_buf[0][0]; break;
    case 0x11: note_idx_=chl_.decomp_buf[0][1]; break;
  }

  // get instrument
  switch(data_mask&0x22)
  {
    case 0x02:
    {
      inst_idx_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_instrument_bits)&((1<<pmfcfg_num_instrument_bits)-1);
      chl_.decomp_buf[1][1]=chl_.decomp_buf[1][0];
      chl_.decomp_buf[1][0]=inst_idx_;
    } break;
    case 0x20: inst_idx_=chl_.decomp_buf[1][0]; break;
    case 0x22: inst_idx_=chl_.decomp_buf[1][1]; break;
  }

  // get volume
  switch(data_mask&0x44)
  {
    case 0x04:
    {
      volume_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_volume_bits)&((1<<pmfcfg_num_volume_bits)-1);
      chl_.decomp_buf[2][1]=chl_.decomp_buf[2][0];
      chl_.decomp_buf[2][0]=volume_;
    } break;
    case 0x40: volume_=chl_.decomp_buf[2][0]; break;
    case 0x44: volume_=chl_.decomp_buf[2][1]; break;
  }

  // get effect
  switch(data_mask&0x88)
  {
    case 0x08:
    {
      effect_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_effect_bits)&((1<<pmfcfg_num_effect_bits)-1);
      effect_data_=read_bits(chl_.track_pos, chl_.track_bit_pos, pmfcfg_num_effect_data_bits)&((1<<pmfcfg_num_effect_data_bits)-1);
      chl_.decomp_buf[3][1]=chl_.decomp_buf[3][0];
      chl_.decomp_buf[3][0]=effect_;
      chl_.decomp_buf[4][1]=chl_.decomp_buf[4][0];
      chl_.decomp_buf[4][0]=effect_data_;
    } break;
    case 0x80: effect_=chl_.decomp_buf[3][0]; effect_data_=chl_.decomp_buf[4][0]; break;
    case 0x88: effect_=chl_.decomp_buf[3][1]; effect_data_=chl_.decomp_buf[4][1]; break;
  }
}
//----

void pmf_player::init_pattern(uint8_t playlist_pos_, uint8_t row_)
{
  // set state
  m_current_pattern_playlist_pos=playlist_pos_;
  m_current_pattern_row_idx=row_;
  m_pattern_loop_cnt=0;
  m_pattern_loop_row_idx=0;

  // initialize pattern at given playlist location and pattern row
  const uint8_t *pattern=m_pmf_pattern_meta+pgm_read_byte(m_pmf_file+pmfcfg_offset_playlist+playlist_pos_)*(pmfcfg_pattern_metadata_header_size+pmfcfg_pattern_metadata_track_offset_size*m_num_pattern_channels);
  m_current_pattern_last_row=pgm_read_byte(pattern+pmfcfg_offset_pattern_metadata_last_row);
  for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
  {
    // init audio track
    audio_channel &chl=m_channels[ci];
    chl.track_pos=m_pmf_file+pgm_read_word(pattern+pmfcfg_offset_pattern_metadata_track_offsets+ci*pmfcfg_pattern_metadata_track_offset_size);
    chl.track_bit_pos=0;
    chl.decomp_type=read_bits(chl.track_pos, chl.track_bit_pos, 3)&7;
    chl.track_loop_pos=chl.track_pos;
    chl.track_loop_bit_pos=chl.track_bit_pos;

    // skip to given row
    uint8_t note_idx, inst_idx, volume, effect, effect_data;
    for(unsigned ri=0; ri<row_; ++ri)
      process_track_row(chl, note_idx, inst_idx, volume, effect, effect_data);
  }
}
//---------------------------------------------------------------------------

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
  uint16_t note_period_min;
  uint16_t note_period_max;
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
enum {pmf_file_version=0x1210};      // v1.21
// PMF file structure
enum {pmfcfg_offset_signature=PFC_OFFSETOF(pmf_header, signature)};
enum {pmfcfg_offset_version=PFC_OFFSETOF(pmf_header, version)};
enum {pmfcfg_offset_flags=PFC_OFFSETOF(pmf_header, flags)};
enum {pmfcfg_offset_init_speed=PFC_OFFSETOF(pmf_header, initial_speed)};
enum {pmfcfg_offset_init_tempo=PFC_OFFSETOF(pmf_header, initial_tempo)};
enum {pmfcfg_offset_note_period_min=PFC_OFFSETOF(pmf_header, note_period_min)};
enum {pmfcfg_offset_note_period_max=PFC_OFFSETOF(pmf_header, note_period_max)};
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
enum {pmfcfg_num_instrument_bits=6}; // max 64 instruments
enum {pmfcfg_num_volume_bits=6};     // volume range [0, 63]
enum {pmfcfg_num_effect_bits=4};     // effects 0-15
enum {pmfcfg_num_effect_data_bits=8};
// PMF flags
enum e_pmf_flags
{
  pmfflag_linear_freq_table  =0x01,  // 0=Amiga, 1=linear
};
// PMF special notes
enum {pmfcfg_note_cut=120};
enum {pmfcfg_note_off=121};
// PMF effects
enum {num_subfx_value_bits=4};
enum {subfx_value_mask=~(-1<<num_subfx_value_bits)};
enum e_pmf_volfx
{
  pmfvolfx_volume_slide,
  pmfvolfx_note_slide,
  pmfvolfx_vibrato,
};
enum e_pmfx_vslide_type
{
  pmffx_vslidetype_down       =0x00,
  pmffx_vslidetype_up         =0x10,
  pmffx_vslidetype_fine_down  =0x20,
  pmffx_vslidetype_fine_up    =0x30,
  //----
  pmffx_vslidetype_mask       =0x30,
  pmffx_vsslidetype_fine_mask =0x20
};
// waveform tables
static const int8_t PROGMEM s_waveforms[3][32]=
{
  {6, 19, 31, 43, 54, 65, 76, 85, 94, 102, 109, 115, 120, 123, 126, 127, 127, 126, 123, 120, 115, 109, 102, 94, 85, 76, 65, 54, 43, 31, 19, 6}, // sine-wave
  {-2, -6, -10, -14, -18, -22, -26, -30, -34, -38, -42, -46, -50, -54, -58, -62, -66, -70, -74, -78, -82, -86, -90, -94, -98, -102, -106, -110, -114, -118, -122, -126}, // ramp down-wave
  {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127}, // square-wave
};
// logging
#ifdef PMF_SERIAL_LOGS
void logf(const char *fs_, ...)
{
  char buf[64];
  va_list args;
  va_start(args, fs_);
  sprintf(buf, fs_, args);
  va_end(args);
  Serial.print(buf);
}
#define PMF_SERIAL_LOG(x__) logf x__
#else
#define PMF_SERIAL_LOG(x__)
#endif
//---------------------------------------------------------------------------


//===========================================================================
// PMF note periods
//===========================================================================
enum {note_slide_down_target_period=32767};
enum {note_slide_up_target_period=1};
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
  uint16_t get_note_period(uint8_t note_idx_, int16_t finetune_, uint8_t flags_)
  {
    if(flags_&pmfflag_linear_freq_table)
      return 7680-note_idx_*64-finetune_/2;
    return uint16_t(27392.0f/exp2((note_idx_*128+finetune_)/(12.0f*128.0f))+0.5f);
  }
  //-------------------------------------------------------------------------

  //=========================================================================
  // get_sample_speed
  //=========================================================================
  int16_t get_sample_speed(uint16_t note_period_, uint8_t pmf_flags_, bool forward_)
  {
    int16_t speed;
    if(pmf_flags_&pmfflag_linear_freq_table)
      speed=int16_t((8363.0f*8.0f/pmfplayer_sampling_rate)*exp2(float(7680-note_period_)/768.0f)+0.5f);
    else
      speed=int16_t((7093789.2f*256.0f/pmfplayer_sampling_rate)/note_period_+0.5f);
    return forward_?speed:-speed;
  }
} // namespace <anonymous>
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
pmf_player::pmf_player()
{
  m_pmf_file=0;
  m_row_callback=0;
  m_speed=0;
}
//----

pmf_player::~pmf_player()
{
  stop();
}
//----

void pmf_player::load(const void *pmem_pmf_file_)
{
  // check for valid PMF file
  const uint8_t *pmf_file=static_cast<const uint8_t*>(pmem_pmf_file_);
  if(pgm_read_dword(pmf_file+pmfcfg_offset_signature)!=0x78666d70)
  {
    PMF_SERIAL_LOG(("Error: Invalid PMF file. Please use pmf_converter to generate the file.\r\n"));
    return;
  }
  if((pgm_read_word(pmf_file+pmfcfg_offset_version)&0xfff0)!=pmf_file_version)
  {
    PMF_SERIAL_LOG(("Error: PMF file version mismatch. Please use matching pmf_converter to generate the file.\r\n"));
    return;
  }

  // read PMF properties
  m_pmf_file=pmf_file;
  m_num_pattern_channels=pgm_read_byte(m_pmf_file+pmfcfg_offset_num_channels);
  enable_playback_channels(m_num_pattern_channels);
  m_flags=pgm_read_word(m_pmf_file+pmfcfg_offset_flags);
  m_pmf_instrument_meta=m_pmf_file+((pmfcfg_offset_playlist+pgm_read_word(m_pmf_file+pmfcfg_offset_playlist_length)+3)&-4);
  m_pmf_pattern_meta=m_pmf_instrument_meta+sizeof(pmf_instrument_header)*pgm_read_byte(m_pmf_file+pmfcfg_offset_num_instruments);
  m_note_slide_speed=m_flags&pmfflag_linear_freq_table?4:2;
  PMF_SERIAL_LOG(("PMF file loaded (%i channels)\r\n", m_num_pattern_channels));
}
//----

void pmf_player::enable_playback_channels(uint8_t num_channels_)
{
  if(m_pmf_file)
    m_num_playback_channels=num_channels_<pmfplayer_max_channels?num_channels_:pmfplayer_max_channels;
}
//----

void pmf_player::set_row_callback(pmf_row_callback_t callback_, void *custom_data_)
{
  m_row_callback=callback_;
  m_row_callback_custom_data=custom_data_;
}
//---------------------------------------------------------------------------

uint8_t pmf_player::num_pattern_channels() const
{
  return m_pmf_file?m_num_pattern_channels:0;
}
//----

uint8_t pmf_player::num_playback_channels() const
{
  return m_pmf_file?m_num_playback_channels:0;
}
//----

uint16_t pmf_player::playlist_length() const
{
  return m_pmf_file?pgm_read_word(m_pmf_file+pmfcfg_offset_playlist_length):0;
}
//---------------------------------------------------------------------------

void pmf_player::start(uint16_t playlist_pos_)
{
  // initialize channels
  if(!m_pmf_file)
    return;
  memset(m_channels, 0, sizeof(m_channels));
  for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];
    chl.fxmem_vol_slide_spd=pmffx_vslidetype_down|0x01;
  }

  // init playback state
  m_num_processed_pattern_channels=min(m_num_pattern_channels, m_num_playback_channels);
  init_pattern(playlist_pos_<pgm_read_word(m_pmf_file+pmfcfg_offset_playlist_length)?playlist_pos_:0);
  m_speed=pgm_read_byte(m_pmf_file+pmfcfg_offset_init_speed);
  m_note_period_min=pgm_read_word(m_pmf_file+pmfcfg_offset_note_period_min);
  m_note_period_max=pgm_read_word(m_pmf_file+pmfcfg_offset_note_period_max);
  m_num_batch_samples=(long(pmfplayer_sampling_rate)*125)/long(pgm_read_byte(m_pmf_file+pmfcfg_offset_init_tempo)*50);
  m_current_row_tick=m_speed-1;
  m_arpeggio_counter=0;
  m_pattern_delay=1;

  // start playback
  m_batch_pos=0;
  start_playback();
  PMF_SERIAL_LOG(("PMF playback started (%i channels)\r\n", m_num_playback_channels));
}
//----

void pmf_player::stop()
{
  if(m_speed)
    stop_playback();
  m_speed=0;
}
//----

void pmf_player::update()
{
  // check if audio buffer should be updated
  if(!m_note_slide_speed)
    return;
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

bool pmf_player::is_playing() const
{
  return m_speed!=0;
}
//----

uint8_t pmf_player::playlist_pos() const
{
  return m_speed?m_current_pattern_playlist_pos:0;
}  
//----

uint8_t pmf_player::pattern_row() const
{
  return m_speed?m_current_pattern_row_idx:0;
}  
//----

uint8_t pmf_player::pattern_speed() const
{
  return m_speed?m_speed:0;
}
//----

pmf_channel_info pmf_player::channel_info(uint8_t channel_idx_) const
{
  pmf_channel_info info;
  if(channel_idx_<m_num_playback_channels)
  {
    // collect channel info
    const audio_channel &chl=m_channels[channel_idx_];
    info.base_note=chl.base_note_idx;
    info.volume=chl.sample_volume;
    info.effect=chl.effect;
    info.effect_data=chl.effect_data;
  }
  else
  {
    // setup no-info
    info.base_note=0xff;
    info.volume=0;
    info.effect=0xff;
    info.effect_data=0;
  }
  return info;
}
//---------------------------------------------------------------------------

void pmf_player::apply_channel_effect_volume_slide(audio_channel &chl_)
{
  // slide volume either up/down defined by effect
  int8_t vdelta=(chl_.fxmem_vol_slide_spd&0xf)<<2;
  int16_t v=int16_t(chl_.sample_volume)+((chl_.fxmem_vol_slide_spd&pmffx_vslidetype_mask)==pmffx_vslidetype_down?-vdelta:vdelta);
  chl_.sample_volume=v<0?0:v>255?255:v;
}
//----

void pmf_player::apply_channel_effect_note_slide(audio_channel &chl_)
{
  // slide note period towards target period and clamp the period to target
  int16_t note_period=chl_.note_period;
  int16_t note_target_prd=chl_.fxmem_note_slide_prd;
  int16_t slide_spd=(note_period<note_target_prd?chl_.fxmem_note_slide_spd:-chl_.fxmem_note_slide_spd)*m_note_slide_speed;
  note_period+=slide_spd;
  note_period=((slide_spd>0)^(note_period<note_target_prd))?note_target_prd:note_period;
  chl_.note_period=note_period;
  chl_.sample_speed=note_period<m_note_period_min || note_period>m_note_period_max?0:get_sample_speed(chl_.note_period, m_flags, chl_.sample_speed>=0);
}
//----

void pmf_player::apply_channel_effect_vibrato(audio_channel &chl_)
{
  uint8_t wave_idx=chl_.fxmem_vibrato_wave&3;
  int8_t vibrato_pos=chl_.fxmem_vibrato_pos;
  int8_t wave_sample=vibrato_pos<0?-int8_t(pgm_read_byte(&s_waveforms[wave_idx][~vibrato_pos])):pgm_read_byte(&s_waveforms[wave_idx][vibrato_pos]);
  chl_.sample_speed=get_sample_speed(chl_.note_period+(int16_t(wave_sample*chl_.fxmem_vibrato_depth)>>8), m_flags, chl_.sample_speed>=0);
  if((chl_.fxmem_vibrato_pos+=chl_.fxmem_vibrato_spd)>31)
    chl_.fxmem_vibrato_pos-=64;
}
//----

void pmf_player::apply_channel_effects()
{
  m_arpeggio_counter=(m_arpeggio_counter+1)%3;
  for(unsigned ci=0; ci<m_num_playback_channels; ++ci)
  {
    // apply active volume effect
    audio_channel &chl=m_channels[ci];
    switch(chl.vol_effect)
    {
      case pmfvolfx_volume_slide: apply_channel_effect_volume_slide(chl); break;
      case pmfvolfx_note_slide: apply_channel_effect_note_slide(chl); break;
      case pmfvolfx_vibrato: apply_channel_effect_vibrato(chl); break;
    }

    // apply active effect
    switch(chl.effect)
    {
      case pmffx_arpeggio:
      {
        // alternate between 3 periods defined by arpeggio parameters
        if(uint8_t base_note_idx=chl.base_note_idx&127)
        {
          uint16_t note_period=get_note_period(base_note_idx+((chl.fxmem_arpeggio>>(4*m_arpeggio_counter))&0xf), chl.sample_finetune, m_flags);
          chl.sample_speed=get_sample_speed(note_period, m_flags, chl.sample_speed>=0);
        }
      } break;

      case pmffx_note_slide: apply_channel_effect_note_slide(chl); break;

      case pmffx_note_vol_slide:
      {
        if(chl.fxmem_note_slide_spd<0xe0)
          apply_channel_effect_note_slide(chl);
        if(!(chl.fxmem_vol_slide_spd&pmffx_vsslidetype_fine_mask))
          apply_channel_effect_volume_slide(chl);
      } break;

      case pmffx_volume_slide: apply_channel_effect_volume_slide(chl); break;

      case pmffx_vibrato: apply_channel_effect_vibrato(chl); break;

      case pmffx_vibrato_vol_slide:
      {
        apply_channel_effect_vibrato(chl);
        if(!(chl.fxmem_vol_slide_spd&pmffx_vsslidetype_fine_mask))
          apply_channel_effect_volume_slide(chl);
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

      case pmffx_subfx|(pmfsubfx_note_cut<<pmfcfg_num_effect_bits):
      {
        // cut note after given number of ticks
        if(!--chl.effect_data)
        {
          chl.sample_speed=0;
          chl.effect=0xff;
        }
      } break;

      case pmffx_subfx|(pmfsubfx_note_delay<<pmfcfg_num_effect_bits):
      {
        // hit note after given number of ticks
        if(!--chl.effect_data)
        {
          hit_note(chl, chl.fxmem_note_delay_idx, 0);
          chl.effect=0xff;
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
    // check for envelope
    audio_channel &chl=m_channels[ci];
    uint8_t volume=255;
    if(uint16_t env_offset=pgm_read_word(chl.inst_metadata+pmfcfg_offset_inst_vol_env))
    {
      // advance envelope (check if passes the current span end point)
      const uint8_t *envelope=m_pmf_file+env_offset;
      const uint8_t *env_span_data=envelope+pmfcfg_offset_env_points+chl.vol_env_pos*pmfcfg_envelope_point_size;
      uint8_t env_span_pos_end=pgm_read_byte(env_span_data+pmfcfg_offset_env_point_pos+pmfcfg_envelope_point_size);
      if(++chl.vol_env_tick>=env_span_pos_end)
      {
        // get envelope start and end points (sustain/loop/none)
        uint8_t env_pnt_start_idx, env_pnt_end_idx;
        if(chl.base_note_idx&0x80)
        {
          env_pnt_start_idx=pgm_read_byte(envelope+pmfcfg_offset_env_loop_start);
          env_pnt_end_idx=pgm_read_byte(envelope+pmfcfg_offset_env_loop_end);
        }
        else
        {
          env_pnt_start_idx=pgm_read_byte(envelope+pmfcfg_offset_env_sustain_loop_start);
          env_pnt_end_idx=pgm_read_byte(envelope+pmfcfg_offset_env_sustain_loop_end);
        }
        uint8_t env_last_pnt_idx=pgm_read_byte(envelope+pmfcfg_offset_env_num_points)-1;
        env_pnt_start_idx=min(env_pnt_start_idx, env_last_pnt_idx);
        env_pnt_end_idx=min(env_pnt_end_idx, env_last_pnt_idx);

        // check for envelope end/loop-end
        if(++chl.vol_env_pos==env_pnt_end_idx)
        {
          if(env_pnt_start_idx<env_pnt_end_idx)
            chl.vol_env_pos=env_pnt_start_idx;
          else
            chl.vol_env_pos=env_pnt_start_idx-1;
          chl.vol_env_tick=pgm_read_byte(envelope+pmfcfg_offset_env_points+pmfcfg_offset_env_point_pos+env_pnt_start_idx*pmfcfg_envelope_point_size);
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

    // check volume fadeout (note off)
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

bool pmf_player::init_effect_volume_slide(audio_channel &chl_, uint8_t effect_data_)
{
  // check for volume slide data or read from effect memory
  if(effect_data_&0xf)
    chl_.fxmem_vol_slide_spd=effect_data_;
  effect_data_=chl_.fxmem_vol_slide_spd;

  if(effect_data_&pmffx_vsslidetype_fine_mask)
  {
    // fine slide
    uint8_t fx_type=effect_data_&pmffx_vslidetype_mask;
    int8_t vdelta=(effect_data_&0xf)<<2;
    int16_t v=int16_t(chl_.sample_volume)+(fx_type==pmffx_vslidetype_fine_down?-vdelta:vdelta);
    chl_.sample_volume=v<0?0:v>255?255:v;
    return false;
  }
  return true;
}
//----

bool pmf_player::init_effect_note_slide(audio_channel &chl_, uint8_t slide_speed_, uint16_t target_note_period_)
{
  // update note slide effect memory and check for regular note slide
  if(slide_speed_)
    chl_.fxmem_note_slide_spd=slide_speed_;
  else
    slide_speed_=chl_.fxmem_note_slide_spd;
  if(target_note_period_)
    chl_.fxmem_note_slide_prd=target_note_period_;
  if(slide_speed_<0xe0)
    return true;

  // apply fine/extra-fine note slide
  if(!chl_.sample_speed)
    return false;
  int16_t note_period=chl_.note_period;
  int16_t slide_spd=slide_speed_>=0xf0?(slide_speed_-0xf0)*4:(slide_speed_-0xe0);
  if(note_period>target_note_period_)
    slide_spd=-slide_spd;
  note_period+=slide_spd;
  if((slide_spd>0)^(note_period<target_note_period_))
    note_period=target_note_period_;
  chl_.note_period=note_period;
  if(note_period<m_note_period_min || note_period>m_note_period_max)
    chl_.sample_speed=0;
  return false;
}
//----

void pmf_player::init_effect_vibrato(audio_channel &chl_, uint8_t vibrato_depth_, uint8_t vibrato_speed_)
{
  // update vibrato attributes
  if(vibrato_depth_)
    chl_.fxmem_vibrato_depth=vibrato_depth_<<3;
  if(vibrato_speed_)
    chl_.fxmem_vibrato_spd=vibrato_speed_;
}
//----

void pmf_player::hit_note(audio_channel &chl_, uint8_t note_idx_, uint32_t sample_start_pos_)
{
  if(!chl_.inst_metadata)
    return;
  chl_.note_period=get_note_period(note_idx_, chl_.sample_finetune, m_flags);
  chl_.base_note_idx=note_idx_;
  chl_.sample_pos=sample_start_pos_;
  chl_.sample_speed=get_sample_speed(chl_.note_period, m_flags, true);
  chl_.note_hit=1;
  chl_.vol_env_tick=uint8_t(-1);
  chl_.vol_env_pos=-1;
  if(!(chl_.fxmem_vibrato_wave&0x4))
    chl_.fxmem_vibrato_pos=0;
}
//----

void pmf_player::process_pattern_row()
{
  // store current track positions
  const uint8_t *current_track_poss[pmfplayer_max_channels];
  uint8_t current_track_bit_poss[pmfplayer_max_channels];
  for(uint8_t ci=0; ci<m_num_processed_pattern_channels; ++ci)
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
    if(ci<m_num_processed_pattern_channels)
      process_track_row(chl, note_idx, inst_idx, volume, effect, effect_data);
    if(m_row_callback)
    {
      // apply custom track data
      uint8_t custom_note_idx=0xff, custom_inst_idx=0xff, custom_volume=0xff, custom_effect=0xff, custom_effect_data;
      (*m_row_callback)(m_row_callback_custom_data, ci, custom_note_idx, custom_inst_idx, custom_volume, custom_effect, custom_effect_data);
      if(custom_note_idx<12*10 || custom_note_idx==pmfcfg_note_cut || custom_note_idx==pmfcfg_note_off)
        note_idx=custom_note_idx;
      if(custom_inst_idx<pgm_read_byte(m_pmf_file+pmfcfg_offset_num_instruments))
        inst_idx=custom_inst_idx;
      if(custom_volume!=0xff)
        volume=custom_volume;
      if(custom_effect!=0xff)
      {
        effect=custom_effect;
        effect_data=custom_effect_data;
      }
    }

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
      if(!(chl.base_note_idx&128))
      {
        chl.base_note_idx|=128;
        chl.vol_fadeout=65535;
      }
      note_idx=0xff;
    }

    // check for instrument
    if(inst_idx!=0xff)
    {
      // setup channel instrument
      const uint8_t *inst_metadata=m_pmf_instrument_meta+inst_idx*pmfcfg_instrument_metadata_size;
      chl.inst_metadata=inst_metadata;
      chl.sample_volume=pgm_read_byte(inst_metadata+pmfcfg_offset_inst_volume);
      chl.sample_finetune=pgm_read_word(inst_metadata+pmfcfg_offset_inst_finetune);
    }

    // check for volume or volume effect
    chl.vol_effect=0xff;
    bool update_sample_speed=true;
    if(volume!=0xff)
    {
      if(volume<(1<<pmfcfg_num_volume_bits))
        chl.sample_volume=(volume<<2)|(volume>>4);
      else
      {
        // initialize volume effect
        uint8_t volfx_data=volume&0xf;
        switch(volume&0xf0)
        {
          // volume slide
          case 0x40:
          case 0x50:
          case 0x60:
          case 0x70:
          {
            if(init_effect_volume_slide(chl, volume&0x3f))
              chl.vol_effect=pmfvolfx_volume_slide;
          } break;

          // note slide down
          case 0x80:
          {
            // init note slide down and disable note retrigger
            if(init_effect_note_slide(chl, volfx_data, note_slide_down_target_period))
              chl.vol_effect=pmfvolfx_note_slide;
          } break;
          
          // note slide up
          case 0x90:
          {
            // init note slide up and disable note retrigger
            if(init_effect_note_slide(chl, volfx_data, note_slide_up_target_period))
              chl.vol_effect=pmfvolfx_note_slide;
          } break;

          // note slide
          case 0xa0:
          {
            // init note slide and disable note retrigger
            if(init_effect_note_slide(chl, volfx_data, note_idx!=0xff?get_note_period(note_idx, chl.sample_finetune, m_flags):0))
              chl.vol_effect=pmfvolfx_note_slide;
            note_idx=0xff;
          } break;

          // set vibrato speed
          case 0xb0:
          {
            if(volfx_data)
              chl.fxmem_vibrato_spd=volfx_data;
          } break;

          // vibrato
          case 0xc0:
          {
            init_effect_vibrato(chl, volfx_data, 0);
            chl.vol_effect=pmfvolfx_vibrato;
            update_sample_speed=false;
          } break;
        }
      }
    }

    // get effect
    chl.effect=0xff;
    if(effect!=0xff)
    {
      // setup effect
      switch(effect)
      {
        case pmffx_set_speed_tempo:
        {
          if(effect_data<32)
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

        case pmffx_volume_slide:
        {
          if(init_effect_volume_slide(chl, effect_data))
            chl.effect=pmffx_volume_slide;
        } break;

        case pmffx_note_slide_down:
        {
          if(init_effect_note_slide(chl, effect_data, note_slide_down_target_period))
            chl.effect=pmffx_note_slide;
        } break;

        case pmffx_note_slide_up:
        {
          if(init_effect_note_slide(chl, effect_data, note_slide_up_target_period))
            chl.effect=pmffx_note_slide;
        } break;

        case pmffx_note_slide:
        {
          // init note slide and disable retrigger
          if(init_effect_note_slide(chl, effect_data, note_idx!=0xff?get_note_period(note_idx, chl.sample_finetune, m_flags):0))
            chl.effect=pmffx_note_slide;
          note_idx=0xff;
        } break;

        case pmffx_arpeggio:
        {
          chl.effect=pmffx_arpeggio;
          if(effect_data)
            chl.fxmem_arpeggio=effect_data;
        } break;

        case pmffx_vibrato:
        {
          // update vibrato attributes
          init_effect_vibrato(chl, effect_data&0x0f, effect_data>>4);
          chl.effect=pmffx_vibrato;
          update_sample_speed=false;
        } break;

        case pmffx_tremolo:
        {
          /*todo*/
        } break;

        case pmffx_note_vol_slide:
        {
          init_effect_note_slide(chl, 0, note_idx!=0xff?get_note_period(note_idx, chl.sample_finetune, m_flags):0);
          init_effect_volume_slide(chl, effect_data);
          chl.effect=pmffx_note_vol_slide;
          note_idx=0xff;
        } break;

        case pmffx_vibrato_vol_slide:
        {
          init_effect_vibrato(chl, 0, 0);
          init_effect_volume_slide(chl, effect_data);
          chl.effect=pmffx_vibrato_vol_slide;
          update_sample_speed=false;
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
            case pmfsubfx_set_glissando:
            {
              /*todo*/
            } break;

            case pmfsubfx_set_finetune:
            {
              if(inst_idx!=0xff)
              {
                /*todo: should move C4hz values of instruments to RAM*/
              }
            } break;

            case pmfsubfx_set_vibrato_wave:
            {
              uint8_t wave=effect_data&3;
              chl.fxmem_vibrato_wave=(wave<3?wave:m_batch_pos%3)|(effect_data&4);
            } break;

            case pmfsubfx_set_tremolo_wave:
            {
              /*todo*/
            } break;

            case pmfsubfx_pattern_delay:
            {
              m_pattern_delay=(effect_data&0xf)+1;
            } break;

            case pmfsubfx_pattern_loop:
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
                for(unsigned ci=0; ci<m_num_processed_pattern_channels; ++ci)
                {
                  audio_channel &chl=m_channels[ci];
                  chl.track_loop_pos=current_track_poss[ci];
                  chl.track_loop_bit_pos=current_track_bit_poss[ci];
                  memcpy(chl.track_loop_decomp_buf, chl.decomp_buf, sizeof(chl.track_loop_decomp_buf));
                }
              }
            } break;

            case pmfsubfx_note_cut:
            {
              chl.effect=pmffx_subfx|(pmfsubfx_note_cut<<pmfcfg_num_effect_bits);
              chl.effect_data=effect_data&0xf;
            } break;

            case pmfsubfx_note_delay:
            {
              if(note_idx==0xff)
                break;
              chl.effect=pmffx_subfx|(pmfsubfx_note_delay<<pmfcfg_num_effect_bits);
              chl.effect_data=effect_data&0xf;
              chl.fxmem_note_delay_idx=note_idx;
              note_idx=0xff;
            } break;
          }
        } break;
      }
    }

    // check for note hit
    if(note_idx!=0xff)
      hit_note(chl, note_idx, sample_start_pos*long(65536));
    else if(update_sample_speed && chl.sample_speed)
      chl.sample_speed=get_sample_speed(chl.note_period, m_flags, chl.sample_speed>=0);
  }

  // check for pattern loop
  if(loop_pattern)
  {
    for(unsigned ci=0; ci<m_num_processed_pattern_channels; ++ci)
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
  if(chl_.track_pos==m_pmf_file)
    return;
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
    data_mask=read_bits(chl_.track_pos, chl_.track_bit_pos, chl_.decomp_type&0x4?8:4)&(chl_.decomp_type&4?0xff:0x0f);
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
      uint8_t num_volume_bits=chl_.decomp_type&0x8?pmfcfg_num_volume_bits+2:pmfcfg_num_volume_bits;
      volume_=read_bits(chl_.track_pos, chl_.track_bit_pos, num_volume_bits)&((1<<num_volume_bits)-1);
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
  for(unsigned ci=0; ci<m_num_processed_pattern_channels; ++ci)
  {
    // init audio track
    audio_channel &chl=m_channels[ci];
    uint16_t track_offs=pgm_read_word(pattern+pmfcfg_offset_pattern_metadata_track_offsets+ci*pmfcfg_pattern_metadata_track_offset_size);
    chl.track_pos=m_pmf_file+track_offs;
    chl.track_bit_pos=0;
    chl.track_loop_pos=chl.track_pos;
    chl.track_loop_bit_pos=chl.track_bit_pos;
    if(track_offs)
      chl.decomp_type=read_bits(chl.track_pos, chl.track_bit_pos, 4)&15;

    // skip to given row
    uint8_t note_idx, inst_idx, volume, effect, effect_data;
    for(unsigned ri=0; ri<row_; ++ri)
      process_track_row(chl, note_idx, inst_idx, volume, effect, effect_data);
  }
}
//---------------------------------------------------------------------------

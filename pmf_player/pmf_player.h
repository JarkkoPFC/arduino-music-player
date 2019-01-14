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

#ifndef PFC_PMF_PLAYER_H
#define PFC_PMF_PLAYER_H
//---------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include <Arduino.h>

// new
class pmf_player;
//---------------------------------------------------------------------------


//===========================================================================
// PMF player config
//===========================================================================
#define PMF_AUDIO_LEVEL 2
#define PMF_LINEAR_INTERPOLATION         // interpolate samples linearly for better sound quality (more CPU intensive)
enum {pmfplayer_sampling_rate=22050};    // playback frequency in Hz
enum {pmfplayer_max_channels=32};        // maximum number of audio playback channels
enum {pmfplayer_led_beat_ticks=1};       // number of ticks to display LED upon not hit
//---------------------------------------------------------------------------


//===========================================================================
// pmf_player
//===========================================================================
class pmf_player
{
public:
  // construction
  pmf_player();
  ~pmf_player();
  //-------------------------------------------------------------------------

  // player control
  void start(const void *pmem_pmf_file_);
  void stop();
  void update();
  //-------------------------------------------------------------------------

private:
  struct audio_channel;
  struct mixer_buffer;
  void start_playback();
  void stop_playback();
  void mix_buffer(mixer_buffer&, unsigned num_samples_);
  mixer_buffer get_mixer_buffer();
  void apply_channel_effect_volume_slide(audio_channel&);
  void apply_channel_effect_note_slide(audio_channel&);
  void apply_channel_effect_vibrato(audio_channel&);
  void apply_channel_effects();
  void evaluate_envelopes();
  bool init_effect_volume_slide(audio_channel&, uint8_t effect_data_);
  bool init_effect_note_slide(audio_channel&, uint8_t slide_speed_, uint16_t target_note_pediod_);
  void init_effect_vibrato(audio_channel&, uint8_t vibrato_depth_, uint8_t vibrato_speed_);
  void hit_note(audio_channel&, uint8_t note_idx_, uint32_t sample_start_pos_);
  void process_pattern_row();
  void process_track_row(audio_channel&, uint8_t &note_idx_, uint8_t &inst_idx_, uint8_t &volume_, uint8_t &effect_, uint8_t &effect_data_);
  void init_pattern(uint8_t playlist_pos_, uint8_t row_=0);
  void visualize_pattern_frame();
  //-------------------------------------------------------------------------

  //=========================================================================
  // mixer_buffer
  //=========================================================================
  struct mixer_buffer
  {
    void *begin;
    unsigned num_samples;
  };
  //-------------------------------------------------------------------------

  //=========================================================================
  // audio_channel
  //=========================================================================
  struct audio_channel
  {
    // sample playback
    const uint8_t *inst_metadata;
    uint32_t sample_pos;           // sample position (24.8 fp)
    int16_t sample_speed;          // sample speed (8.8 fp)
    int16_t sample_finetune;       // sample finetune (9.7 fp)
    uint8_t sample_volume;         // sample volume (0.8 fp)
    // sound effects
    uint16_t note_period;          // current note period (see s_note_periods for base note periods)
    uint8_t base_note_idx;         // base note index
    uint8_t effect;                // current effect
    uint8_t effect_data;           // current effect data
    uint8_t vol_effect;            // current volume effect
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
    uint8_t vol_env_tick;          // volume envelope ticks
    int8_t vol_env_pos;            // volume envelope position
    uint8_t vol_env_value;         // volume envelope value
    // track state
    const uint8_t *track_pos;
    uint8_t track_bit_pos;
    const uint8_t *track_loop_pos;
    uint8_t track_loop_bit_pos;
    uint8_t decomp_type;
    uint8_t decomp_buf[6][2];
    uint8_t track_loop_decomp_buf[6][2];
    // visualization
    uint8_t note_hit;              // note hit
  };
  //-------------------------------------------------------------------------

  // audio assets
  const uint8_t *m_pmf_file;
  const uint8_t *m_pmf_pattern_meta;
  const uint8_t *m_pmf_instrument_meta;
  uint8_t m_note_slide_speed;
  // audio channel states
  uint8_t m_num_playback_channels;
  uint8_t m_num_pattern_channels;
  uint16_t m_flags;  // e_pmf_flags
  audio_channel m_channels[pmfplayer_max_channels];
  // audio buffer state
  uint16_t m_num_batch_samples;
  uint16_t m_batch_pos;
  // pattern playback state
  uint8_t m_current_pattern_playlist_pos;
  uint8_t m_current_pattern_last_row;
  uint8_t m_current_pattern_row_idx;
  uint8_t m_current_row_tick;
  uint16_t m_note_period_min;
  uint16_t m_note_period_max;
  uint8_t m_speed;
  uint8_t m_arpeggio_counter;
  uint8_t m_pattern_delay;
  uint8_t m_pattern_loop_cnt;
  uint8_t m_pattern_loop_row_idx;
};
//---------------------------------------------------------------------------

//============================================================================
#endif

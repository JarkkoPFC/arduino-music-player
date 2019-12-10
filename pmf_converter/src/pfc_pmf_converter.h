//============================================================================
// Spin-X Platform
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

#ifndef PFC_MOD_CONVERTER_H
#define PFC_MOD_CONVERTER_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "sxp_src/core/core.h"
namespace pfc
{
template<typename> class array;

// new
struct pmf_header;
struct pmf_pattern_track_row;
struct pmf_pattern;
struct pmf_instrument;
struct pmf_song;
// PMF flags
enum e_pmf_flags
{
  pmfflag_linear_freq_table  =0x01,  // 0=Amiga, 1=linear
};
// PMF sample flags
enum e_pmf_sample_flags
{
  pmfsmpflag_16bit      = 0x01,
  pmfsmpflag_bidi_loop  = 0x02,
};
// PMF special notes
enum {pmfcfg_note_cut=120};
enum {pmfcfg_note_off=121};
// PMF effects
enum {num_subfx_value_bits=4};
enum {subfx_value_mask=~(unsigned(-1)<<num_subfx_value_bits)};
enum e_pmf_effect
{
  // global playback control
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
  pmffx_tremolo,           // [xxxxyyyy], x=tremolo speed, y=tremolo depth
  pmffx_note_vol_slide,    // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_vibrato_vol_slide, // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_retrig_vol_slide,  // [xxxxyyyy], x=volume slide param, y=sample retrigger frequency
  pmffx_set_sample_offs,   // [xxxxxxxx], offset=x*256
  pmffx_subfx,             // [xxxxyyyy], x=sub-effect, y=sub-effect value
  pmffx_panning,           // [xyzwwwww], x=type (0=set, 1=slide), y=precision (0=normal, 1=fine), z=direction (0=left, 1=right), w=value (if x=0, panning value is [yzwwwww])
};
enum e_pmf_subfx
{
  pmfsubfx_set_glissando,    // 0=off, 1=on (when enabled "note slide" slides half a note at a time)
  pmfsubfx_set_finetune,     // [-8, 7]
  pmfsubfx_set_vibrato_wave, // [0xyy], x=[0=retrigger, 1=no retrigger], yy=vibrato wave=[0=sine, 1=ramp down, 2=square, 3=random]
  pmfsubfx_set_tremolo_wave, // [0xyy], x=[0=retrigger, 1=no retrigger], yy=tremolo wave=[0=sine, 1=ramp down, 2=square, 3=random]
  pmfsubfx_pattern_delay,    // [1, 15]
  pmfsubfx_pattern_loop,     // [0, 15], 0=set loop start, >0 = loop N times from loop start
  pmfsubfx_note_cut,         // [0, 15], cut on X tick
  pmfsubfx_note_delay,       // [0, 15], delay X ticks
};
enum e_pmfx_volslide_type
{
  pmffx_volsldtype_down      =0x00,
  pmffx_volsldtype_up        =0x10,
  pmffx_volsldtype_fine_down =0x20,
  pmffx_volsldtype_fine_up   =0x30,
  //----
  pmffx_volsldtype_mask      =0x30
};
enum e_pmfx_panslide_type
{
  pmffx_pansldtype_left        =0x80,
  pmffx_pansldtype_right       =0xa0,
  pmffx_pansldtype_fine_left   =0xc0,
  pmffx_pansldtype_fine_right  =0xe0,
  //----
  pmffx_pansldtype_val_mask    =0x0f,
  pmffx_pansldtype_dir_mask    =0x20,
  pmffx_pansldtype_fine_mask   =0x40,
  pmffx_pansldtype_enable_mask =0x80
};
enum e_pmf_voleffect
{
  pmfvolfx_vol_slide            =0x40,
  pmfvolfx_vol_slide_down       =0x40,
  pmfvolfx_vol_slide_up         =0x50,
  pmfvolfx_vol_slide_fine_down  =0x60,
  pmfvolfx_vol_slide_fine_up    =0x70,
  pmfvolfx_note_slide_down      =0x80,
  pmfvolfx_note_slide_up        =0x90,
  pmfvolfx_note_slide           =0xa0,
  pmfvolfx_set_vibrato_speed    =0xb0,
  pmfvolfx_vibrato              =0xc0,
  pmfvolfx_set_panning          =0xd0,
  pmfvolfx_pan_slide_fine_left  =0xe0,
  pmfvolfx_pan_slide_fine_right =0xf0,
};
//----------------------------------------------------------------------------


//============================================================================
// e_pmf_error
//============================================================================
enum e_pmf_error
{
  pmferr_ok,
  pmferr_unknown_format,
  pmferr_conversion_failure,
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_header
//============================================================================
struct pmf_header
{
  char signature[4];
  uint16 version;
  uint16 flags; // e_pmf_flags
  uint32 file_size;
  uint32 sample_meta_offs;
  uint32 instrument_meta_offs;
  uint32 pattern_meta_offs;
  uint32 env_data_offs;
  uint32 nmap_data_offs;
  uint32 track_data_offs;
  uint8 initial_speed;
  uint8 initial_tempo;
  uint16 note_period_min;
  uint16 note_period_max;
  uint16 playlist_length;
  uint8 num_channels;
  uint8 num_patterns;
  uint8 num_instruments;
  uint8 num_samples;
  uint8 first_playlist_entry;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_sample_header
//============================================================================
struct pmf_sample_header
{
  uint32 data_offset;
  uint32 length;
  uint32 loop_length_and_panning; // loop_length=bits 0-23, panning=bits 24-31 (-127=left, 0=center, 127=right, -128=no pan)
  int16 finetune;
  uint8 flags;
  uint8 volume;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_instrument_header
//============================================================================
struct pmf_instrument_header
{
  uint16 sample_idx; // sample index or offset to a note map
  uint16 vol_env_offset;
  uint16 pitch_env_offset;
  uint16 fadeout_speed;
  uint8 volume;
  int8 panning; // (-127=left, 0=center, 127=right, -128=no pan)
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_channel
//============================================================================
struct pmf_channel
{
  // construction
  pmf_channel();
  //--------------------------------------------------------------------------

  int8 panning; // (-127=left, 0=center, 127=right, -128=surround)
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_pattern_track_row
//============================================================================
struct pmf_pattern_track_row
{
  // construction
  pmf_pattern_track_row();
  bool operator==(const pmf_pattern_track_row&) const;
  void clear();
  bool is_empty() const;
  bool is_global_effect() const;
  //--------------------------------------------------------------------------

  uint8 note;        // octave*12+note_idx (254=note cut, 255=no note), note_idx={0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F, 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B}
  uint8 instrument;  // 255=no instrument
  uint8 volume;      // vol=[0, 63], 255=no volume change
  uint8 effect;      // 255=no effect
  uint8 effect_data;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_pattern
//============================================================================
struct pmf_pattern
{
  // construction
  pmf_pattern();
  //--------------------------------------------------------------------------

  unsigned num_rows; // [1, 256]
  array<pmf_pattern_track_row> rows;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_envelope
//============================================================================
struct pmf_envelope
{
  // construction
  pmf_envelope();
  bool operator==(const pmf_envelope&) const;
  //--------------------------------------------------------------------------

  uint8 loop_start;
  uint8 loop_end;
  uint8 sustain_loop_start;
  uint8 sustain_loop_end;
  array<pair<uint16, uint16> > data;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_note_map_entry
//============================================================================
struct pmf_note_map_entry
{
  // construction
  PFC_INLINE pmf_note_map_entry() {note_idx_offs=0; sample_idx=0xff;}
  //--------------------------------------------------------------------------

  int8 note_idx_offs;
  uint8 sample_idx;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_instrument
//============================================================================
struct pmf_instrument
{
  // construction
  pmf_instrument();
  //--------------------------------------------------------------------------

  unsigned sample_idx;
  uint16 fadeout_speed;
  uint8 volume;
  int8 panning;
  pmf_envelope vol_envelope;
  pmf_envelope pitch_envelope;
  array<pmf_note_map_entry> note_map;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_sample
//============================================================================
struct pmf_sample
{
  // construction
  pmf_sample();
  //--------------------------------------------------------------------------

  uint32 length;
  uint32 loop_start, loop_len;
  int16 finetune;
  uint8 flags; // e_pmf_sample_flags
  uint8 volume;
  int8 panning;
  owner_data data;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_song
//============================================================================
struct pmf_song
{
  // construction
  pmf_song();
  //--------------------------------------------------------------------------

  heap_str name;
  uint16 flags; // e_pmf_flags
  uint8 initial_speed;
  uint8 initial_tempo;
  uint16 note_period_min;
  uint16 note_period_max;
  unsigned num_valid_instruments;
  unsigned num_valid_samples;
  usize_t total_src_pattern_data_bytes;
  usize_t total_src_sample_data_bytes;
  array<pmf_channel> channels;
  array<uint8> playlist;
  array<pmf_pattern> patterns;
  array<pmf_instrument> instruments;
  array<pmf_sample> samples;
};
//----------------------------------------------------------------------------

//============================================================================
} // namespace pfc
#endif

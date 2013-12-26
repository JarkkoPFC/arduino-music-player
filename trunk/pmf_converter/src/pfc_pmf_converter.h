//============================================================================
// Spin-X Platform (http://www.spinxplatform.com)
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
  pmffx_note_vol_slide,    // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_vibrato_vol_slide, // [000xyyyy], x=vol slide type (0=down, 1=up), y=vol slide value [1, 15], if y=0, use previous slide type & value (x is ignored).
  pmffx_retrig_vol_slide,  // [xxxxyyyy], x=volume slide param, y=sample retrigger frequency
  pmffx_set_sample_offs,   // [xxxxxxxx], offset=x*256
  pmffx_subfx,             // [xxxxyyyy], x=sub-effect, y=sub-effect value
};
enum e_pmf_subfx
{
  pmfsubfx_set_finetune,     // [-8, 7]
  pmfsubfx_set_vibrato_wave, // [0xyy], x=[0=retrigger, 1=no retrigger], yy=vibrato wave=[0=sine, 1=ramp down, 2=square, 3=random]
  pmfsubfx_pattern_delay,    // [1, 15]
  pmfsubfx_loop_pattern,     // [0, 15], 0=set loop start, >0 = loop N times from loop start
};
enum e_pmfx_vslide_type
{
  pmffx_vslidetype_down      =0x00,
  pmffx_vslidetype_up        =0x10,
  pmffx_vslidetype_fine_down =0x20,
  pmffx_vslidetype_fine_up   =0x30,
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_header
//============================================================================
struct pmf_header
{
  char signature[4];
  uint16 version;
  uint16 flags; // e_pmf_flag
  uint32 file_size;
  uint8 initial_speed;
  uint8 initial_tempo;
  uint16 playlist_length;
  uint8 num_channels;
  uint8 num_patterns;
  uint8 num_instruments;
  uint8 first_playlist_entry;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_instrument_header
//============================================================================
struct pmf_instrument_header
{
  uint32 data_offset;
  uint8 flags; // e_pmf_inst_flag
  uint8 default_volume;
  uint16 length;
  uint16 loop_length;
  uint16 c4hz;
  uint16 vol_env_offset;
  uint16 fadeout_speed;
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
  bool is_empty() const;
  unsigned num_bits() const;
  //--------------------------------------------------------------------------

  uint8 note;        // octave*12+note_idx (254=note cut, 255=no note), note_idx={0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F, 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B}
  uint8 instrument;  // 255=no instrument
  uint8 volume;      // vol=[0, 63], 255=no volume change
  uint8 effect;      // 255=no effect
  uint8 effect_data;
  uint8 trailing_empty_rows;
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

  uint8 sustain_loop_start;
  uint8 sustain_loop_end;
  uint8 loop_start;
  uint8 loop_end;
  array<pair<uint8, uint8> > data;
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

  unsigned length;
  unsigned loop_start, loop_len;
  uint32 c4hz;
  uint8 volume;
  uint16 fadeout_speed;
  owner_data data;
  pmf_envelope vol_envelope;
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

  unsigned num_channels;
  uint16 flags; // e_pmf_flag
  uint8 initial_speed;
  uint8 initial_tempo;
  unsigned total_pattern_data_bytes;
  unsigned total_instrument_data_bytes;
  array<uint8> playlist;
  array<pmf_pattern> patterns;
  array<pmf_instrument> instruments;
};
//----------------------------------------------------------------------------

//============================================================================
} // namespace pfc
#endif

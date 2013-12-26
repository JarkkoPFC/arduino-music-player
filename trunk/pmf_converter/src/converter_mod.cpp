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

#include "sxp_src/core/fsys/fsys.h"
#include "sxp_src/core/containers.h"
#include "pfc_pmf_converter.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// convert_mod
//============================================================================
bool convert_mod(bin_input_stream_base &in_file_, pmf_song &song_)
{
  // read signature
  uint32 mod_id;
  in_file_.skip(1080);
  in_file_>>mod_id;
  in_file_.rewind();

  // set mod format
  unsigned num_channels=4, max_samples=15;
  switch(mod_id)
  {
    // "M.K."
    case 0x2e4b2e4d: max_samples=31; break;
    // "4CHN"
    case 0x4e484334: num_channels=4; max_samples=31; break;
    // "6CHN"
    case 0x4e484336: num_channels=6; max_samples=31; break;
    // "8CHN"
    case 0x4e484338: num_channels=8; max_samples=31; break;
    // "FLT4"
    case 0x34544c46: num_channels=4; max_samples=31; break;
    // "FLT8"
    case 0x38544c46: num_channels=4; max_samples=31; break;
    // unknown
    default: return false;
  }

  // setup song
  song_.num_channels=num_channels;

  // read sample infos
  in_file_.skip(20);
  for(unsigned i=0; i<max_samples; ++i)
  {
    in_file_.skip(22);
    uint16 len;
    in_file_>>len;
    len=swap_bytes(len)*2;
    pmf_instrument &inst=song_.instruments.push_back();
    inst.length=len;
    song_.total_instrument_data_bytes+=len;
    if(len>2)
    {
      uint16 loop_start, loop_len;
      uint8 finetune, volume;
      in_file_>>finetune>>volume>>loop_start>>loop_len;
      inst.volume=volume<64?volume<<2:255;
      inst.loop_start=unsigned(swap_bytes(loop_start))*2;
      inst.loop_len=unsigned(swap_bytes(loop_len))*2;
      if(inst.loop_len<3)
        inst.loop_len=0;

      // set finetune
      switch(finetune)
      {
        case  0: inst.c4hz=8363; break;
        case  1: inst.c4hz=8413; break;
        case  2: inst.c4hz=8463; break;
        case  3: inst.c4hz=8529; break;
        case  4: inst.c4hz=8581; break;
        case  5: inst.c4hz=8651; break;
        case  6: inst.c4hz=8723; break;
        case  7: inst.c4hz=8757; break;
        case  8: inst.c4hz=7895; break;
        case  9: inst.c4hz=7941; break;
        case 10: inst.c4hz=7985; break;
        case 11: inst.c4hz=8046; break;
        case 12: inst.c4hz=8107; break;
        case 13: inst.c4hz=8169; break;
        case 14: inst.c4hz=8232; break;
        case 15: inst.c4hz=8280; break;
      }
    }
    else
      in_file_.skip(6);
  }

  // read pattern playlist
  uint8 playlist_len, restart_pos;
  uint8 playlist[128];
  in_file_>>playlist_len>>restart_pos;
  in_file_.read(playlist, 128);
  in_file_.skip(4);
  song_.playlist.resize(playlist_len);
  for(unsigned i=0; i<playlist_len; ++i)
    song_.playlist[i]=playlist[i];

  // get number of patterns
  unsigned num_patterns=0;
  for(unsigned i=0; i<128; ++i)
    num_patterns=max(num_patterns, unsigned(playlist[i]));
  ++num_patterns;

  // read patterns
  unsigned pattern_size=64*num_channels;
  song_.patterns.resize(num_patterns);
  array<uint32> src_pattern_data(pattern_size);
  for(unsigned i=0; i<num_patterns; ++i)
  {
    // process pattern
    song_.total_pattern_data_bytes+=pattern_size*4;
    pmf_pattern &pattern=song_.patterns[i];
    pattern.rows.resize(pattern_size);
    in_file_.read(src_pattern_data.data(), pattern_size);
    unsigned src_data_idx=0;
    for(unsigned ri=0; ri<64; ++ri)
    {
      // process channels in the pattern row
      for(unsigned ci=0; ci<num_channels; ++ci)
      {
        // parse channel data for the row
        pmf_pattern_track_row &track_row=pattern.rows[ri*num_channels+ci];
        uint32 chl_data=src_pattern_data[src_data_idx++];
        uint8 sample_idx=(chl_data&0xf0)|((chl_data>>20)&0xf);
        uint16 note=((chl_data>>8)&0xff)|((chl_data<<8)&0xf00);
        uint8 effect=(chl_data>>16)&0xf;
        uint8 effect_data=chl_data>>24;

        // set note and sample index if defined
        if(note)
        {
          // find closest note
          static const int s_mod_note_periods[]=
          {
            // C     C#     D      D#     E      F      F#     G      G#     A      A#     B
            27392, 25856, 24384, 23040, 21696, 20480, 19328, 18240, 17216, 16256, 15360, 14496,  // octave 0
            13696, 12928, 12192, 11520, 10848, 10240,  9664,  9120,  8608,  8128,  7680,  7248,  // octave 1
             6848,  6464,  6096,  5760,  5424,  5120,  4832,  4560,  4304,  4064,  3840,  3624,  // octave 2
             3424,  3232,  3048,  2880,  2712,  2560,  2416,  2280,  2152,  2032,  1920,  1812,  // octave 3
             1712,  1616,  1524,  1440,  1356,  1280,  1208,  1140,  1076,  1016,   960,   906,  // octave 4
              856,   808,   762,   720,   678,   640,   604,   570,   538,   508,   480,   453,  // octave 5
              428,   404,   381,   360,   339,   320,   302,   285,   269,   254,   240,   227,  // octave 6
              214,   202,   191,   180,   170,   160,   151,   143,   135,   127,   120,   113,  // octave 7
              107,   101,    95,    90,    85,    80,    76,    71,    67,    64,    60,    57,  // octave 8
               54,    51,    48,    45,    42,    40,    38,    36,    34,    32,    30,    28,  // octave 9
          };
          unsigned min_dist=s_mod_note_periods[0]-note;
          unsigned note_idx=0;
          for(unsigned i=1; i<sizeof(s_mod_note_periods)/sizeof(*s_mod_note_periods); ++i)
          {
            unsigned dist=abs(s_mod_note_periods[i]-note);
            if(dist<min_dist)
            {
              min_dist=dist;
              note_idx=i;
            }
          }
          track_row.note=uint8(note_idx-12);
        }

        if(sample_idx)
          track_row.instrument=sample_idx-1;

        // set effect / volume
        if(effect || effect_data)
        {
          switch(effect)
          {
            // arpeggio
            case 0x0:
            {
              track_row.effect=pmffx_arpeggio;
              track_row.effect_data=effect_data;
            } break;

            // note slide up
            case 0x1:
            {
              track_row.effect=pmffx_note_slide_up;
              track_row.effect_data=effect_data<0xe0?effect_data:0xdf;
            } break;

            // slide down
            case 0x2:
            {
              track_row.effect=pmffx_note_slide_down;
              track_row.effect_data=effect_data<0xe0?effect_data:0xdf;
            } break;

            // slide to note
            case 0x3:
            {
              track_row.effect=pmffx_note_slide;
              track_row.effect_data=effect_data<0xe0?effect_data:0xdf;
            } break;

            // vibrato
            case 0x4:
            {
              track_row.effect=pmffx_vibrato;
              track_row.effect_data=effect_data;
            } break;

            // continue slide to note + volume slide
            case 0x5:
            {
              // ignore illegal volume slide
              if(effect_data&0xf0 && effect_data&0x0f)
                break;
              track_row.effect=pmffx_note_vol_slide;
              track_row.effect_data=effect_data<0x10?pmffx_vslidetype_down+effect_data:(pmffx_vslidetype_up+(effect_data>>4));
            } break;

            // continue vibrato + volume slide
            case 0x6:
            {
              // ignore illegal volume slide
              if(effect_data&0xf0 && effect_data&0x0f)
                break;
              track_row.effect=pmffx_vibrato_vol_slide;
              track_row.effect_data=effect_data<0x10?pmffx_vslidetype_down+effect_data:(pmffx_vslidetype_up+(effect_data>>4));
            } break;

            // set sample offset
            case 0x9:
            {
              track_row.effect=pmffx_set_sample_offs;
              track_row.effect_data=effect_data;
            } break;

            // volume slide
            case 0xa:
            {
              if(effect_data&0xf0)
              {
                track_row.effect=pmffx_volume_slide;
                track_row.effect_data=(effect_data>>4)|pmffx_vslidetype_up;
              }
              else if(effect_data&0x0f)
              {
                track_row.effect=pmffx_volume_slide;
                track_row.effect_data=effect_data|pmffx_vslidetype_down;
              }
            } break;

            // position jump
            case 0xb:
            {
              if(effect_data<playlist_len)
              {
                track_row.effect=pmffx_position_jump;
                track_row.effect_data=effect_data;
              }
            } break;

            // set volume
            case 0xc:
            {
              track_row.volume=effect_data>63?63:effect_data;
            } break;

            // pattern break
            case 0xd:
            {
              uint8 row=(effect_data>>4)*10+(effect_data&0x0f);
              if(row<64)
              {
                track_row.effect=pmffx_pattern_break;
                track_row.effect_data=row;
              }
            } break;

            // sub-effect
            case 0xe:
            {
              switch(effect_data>>4)
              {
                // note extra fineslide up
                case 1:
                {
                  track_row.effect=pmffx_note_slide_up;
                  track_row.effect_data=(effect_data&0xf)+0xe0;
                } break;

                // note extra fineslide down
                case 2:
                {
                  track_row.effect=pmffx_note_slide_down;
                  track_row.effect_data=(effect_data&0xf)+0xe0;
                } break;

                // set vibrato waveform
                case 4:
                {
                  if((effect_data&0xf)<8)
                  {
                    track_row.effect=pmffx_subfx;
                    track_row.effect_data=(pmfsubfx_set_vibrato_wave<<num_subfx_value_bits)|(effect_data&0xf);
                  }
                } break;

                // set finetune
                case 5:
                {
                  track_row.effect=pmffx_subfx;
                  uint8 finetune_val=int8(effect_data<<4)>>4;
                  track_row.effect_data=(pmfsubfx_set_finetune<<num_subfx_value_bits)|(finetune_val&subfx_value_mask);
                } break;

                // loop pattern
                case 6:
                {
                  track_row.effect=pmffx_subfx;
                  track_row.effect_data=(pmfsubfx_loop_pattern<<num_subfx_value_bits)|(effect_data&0xf);
                } break;

                // retrigger sample
                case 9:
                {
                  track_row.effect=pmffx_retrig_vol_slide;
                  track_row.effect_data=effect_data&0xf;
                } break;

                // volume fineslide up
                case 10:
                {
                  track_row.effect=pmffx_volume_slide;
                  track_row.effect_data=(effect_data&0xf)|pmffx_vslidetype_fine_up;
                } break;

                // volume fineslide down
                case 11:
                {
                  track_row.effect=pmffx_volume_slide;
                  track_row.effect_data=(effect_data&0xf)|pmffx_vslidetype_fine_down;
                } break;

                // delay pattern
                case 14:
                {
                  if(effect_data&0xf)
                  {
                    track_row.effect=pmffx_subfx;
                    track_row.effect_data=(pmfsubfx_pattern_delay<<num_subfx_value_bits)|(effect_data&0xf);
                  }
                } break;
              }
            } break;

            // set speed
            case 0xf:
            {
              track_row.effect=pmffx_set_speed_tempo;
              track_row.effect_data=effect_data?effect_data:1;
            } break;
          }
        }
      }
    }
  }

  // read sample data
  unsigned num_instruments=song_.instruments.size();
  for(unsigned i=0; i<num_instruments; ++i)
  {
    pmf_instrument &inst=song_.instruments[i];
    if(inst.length>2)
    {
      inst.data=PFC_MEM_ALLOC(inst.length);
      in_file_.read_bytes(inst.data.data, inst.length);
    }
    else
      in_file_.skip(inst.length);
  }

  return true;
}
//----------------------------------------------------------------------------

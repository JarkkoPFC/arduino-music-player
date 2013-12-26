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
// xm_sample
//============================================================================
struct xm_sample
{
  uint32 length;
  uint32 loop_start;
  uint32 loop_length;
  uint8 volume;
  int8 finetune;
  uint8 type;
  uint8 panning;
  int8 rel_note;
  uint8 compression;
  owner_data data;
};
//----------------------------------------------------------------------------


//============================================================================
// convert_xm
//============================================================================
bool convert_xm(bin_input_stream_base &in_file_, pmf_song &song_)
{
  // check for XM file type
  char id[17];
  in_file_.read_bytes(id, 17);
  in_file_.rewind();
  if(!mem_eq(id, "Extended Module: ", 17))
    return false;

  // read header
  uint16 version, song_len, restart_pos, num_channels, num_patterns, num_instruments, flags, default_tempo, default_speed;
  uint32 header_size;
  in_file_.skip(58);
  in_file_>>version>>header_size>>song_len>>restart_pos>>num_channels>>num_patterns>>num_instruments>>flags>>default_speed>>default_tempo;
  uint8 pattern_orders[256];
  in_file_.read_bytes(pattern_orders, 256);

  // setup song
  song_.num_channels=num_channels;
  song_.flags=pmfflag_fast_note_slides|(flags&1?pmfflag_linear_freq_table:0);
  song_.initial_speed=uint8(default_speed);
  song_.initial_tempo=uint8(default_tempo);

  // setup orders
  song_.playlist.resize(song_len);
  for(unsigned i=0; i<song_len; ++i)
    song_.playlist[i]=pattern_orders[i];

  // read patterns
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    // read pattern header
    uint32 header_len;
    uint8 packing;
    uint16 num_rows, packed_size;
    in_file_>>header_len>>packing>>num_rows>>packed_size;
    song_.total_pattern_data_bytes+=packed_size;

    // add pattern
    pmf_pattern &pattern=song_.patterns.push_back();
    pattern.num_rows=num_rows;
    pattern.rows.resize(num_rows*num_channels);
    pmf_pattern_track_row *track_row=pattern.rows.data();

    if(packed_size)
    {
      // process pattern rows
      for(unsigned ri=0; ri<num_rows; ++ri)
      {
        // process channels
        for(unsigned ci=0; ci<num_channels; ++ci)
        {
          // read channel row
          uint8 data_mask, note=0, instrument=0, volume=0, effect=0, effect_data=0;
          in_file_>>data_mask;
          if(data_mask&0x80)
          {
            // read packed channel row
            if(data_mask&0x01)
              in_file_>>note;
            if(data_mask&0x02)
              in_file_>>instrument;
            if(data_mask&0x04)
              in_file_>>volume;
            if(data_mask&0x08)
              in_file_>>effect;
            if(data_mask&0x10)
              in_file_>>effect_data;
          }
          else
          {
            // read unpacked channel row
            note=data_mask;
            in_file_>>instrument>>volume>>effect>>effect_data;
          }

          // check for note
          if(note)
            track_row->note=--note==96?pmfcfg_note_off:note+12;

          // check for instrument
          if(instrument)
            track_row->instrument=instrument-1;

          // volume
          if(volume>=0x10 && volume<=0x50)
            track_row->volume=volume<0x50?volume-0x10:63;

          // effect
          if(effect || effect_data)
          {
            switch(effect)
            {
              // arpeggio
              case 0:
              {
                track_row->effect=pmffx_arpeggio;
                track_row->effect_data=effect_data;
              } break;

              // porta up
              case 1:
              {
                track_row->effect=pmffx_note_slide_up;
                track_row->effect_data=effect_data<0xe0?effect_data:0xdf;
              } break;

              // porta down
              case 2:
              {
                track_row->effect=pmffx_note_slide_down;
                track_row->effect_data=effect_data<0xe0?effect_data:0xdf;
              } break;

              // tone porta
              case 3:
              {
                track_row->effect=pmffx_note_slide;
                track_row->effect_data=effect_data<0xe0?effect_data:0xdf;
              } break;

              // vibrato
              case 4:
              {
                track_row->effect=pmffx_vibrato;
                track_row->effect_data=effect_data;
              } break;

              // tone porta + volume slide
              case 5:
              {
                // ignore illegal volume slide
                if(effect_data&0xf0 && effect_data&0x0f)
                  break;
                track_row->effect=pmffx_note_vol_slide;
                track_row->effect_data=effect_data<0x10?pmffx_vslidetype_down+effect_data:(pmffx_vslidetype_up+(effect_data>>4));
              } break;

              // vibrato + volume slide
              case 6:
              {
                // ignore illegal volume slide
                if(effect_data&0xf0 && effect_data&0x0f)
                  break;
                track_row->effect=pmffx_vibrato_vol_slide;
                track_row->effect_data=effect_data<0x10?pmffx_vslidetype_down+effect_data:(pmffx_vslidetype_up+(effect_data>>4));
              } break;

              // set sample offset
              case 9:
              {
                track_row->effect=pmffx_set_sample_offs;
                track_row->effect_data=effect_data;
              } break;

              // volume slide
              case 10:
              {
                if(effect_data&0xf0)
                {
                  track_row->effect=pmffx_volume_slide;
                  track_row->effect_data=(effect_data>>4)|pmffx_vslidetype_up;
                }
                else if(effect_data&0x0f)
                {
                  track_row->effect=pmffx_volume_slide;
                  track_row->effect_data=effect_data|pmffx_vslidetype_down;
                }
              } break;

              // position jump
              case 11:
              {
                if(effect_data<song_len)
                {
                  track_row->effect=pmffx_position_jump;
                  track_row->effect_data=effect_data;
                }
              } break;

              // set volume
              case 12:
              {
                track_row->volume=effect_data>63?63:effect_data;
              } break;

              // pattern break
              case 13:
              {
                uint8 row=(effect_data>>4)*10+(effect_data&0x0f);
                track_row->effect=pmffx_pattern_break;
                track_row->effect_data=row;
              } break;

              // sub-effect
              case 14:
              {
                switch(effect_data>>4)
                {
                  // fine porta up
                  case 1:
                  {
                    track_row->effect=pmffx_note_slide_up;
                    track_row->effect_data=(effect_data&0xf)+0xf0; /*todo: not quite sure if this should be fine (0xf0) or extra-fine (0xe0) slide like in MOD*/
                  } break;

                  // fine porta down
                  case 2:
                  {
                    track_row->effect=pmffx_note_slide_down;
                    track_row->effect_data=(effect_data&0xf)+0xf0; /*todo: not quite sure if this should be fine (0xf0) or extra-fine (0xe0) slide like in MOD*/
                  } break;

                  // set vibrato control
                  case 4:
                  {
                    if((effect_data&0xf)<8)
                    {
                      track_row->effect=pmffx_subfx;
                      track_row->effect_data=(pmfsubfx_set_vibrato_wave<<num_subfx_value_bits)|(effect_data&0xf);
                    }
                  } break;

                  // set finetune
                  case 5:
                  {
                    track_row->effect=pmffx_subfx;
                    uint8 finetune_val=int8(effect_data<<4)>>4;
                    track_row->effect_data=(pmfsubfx_set_finetune<<num_subfx_value_bits)|(finetune_val&subfx_value_mask);
                  } break;

                  // loop pattern
                  case 6:
                  {
                    track_row->effect=pmffx_subfx;
                    track_row->effect_data=(pmfsubfx_loop_pattern<<num_subfx_value_bits)|(effect_data&0xf);
                  } break;

                  // retrig note
                  case 9:
                  {
                    track_row->effect=pmffx_retrig_vol_slide;
                    track_row->effect_data=effect_data&0xf;
                  } break;

                  // fine volume slide up
                  case 10:
                  {
                    track_row->effect=pmffx_volume_slide;
                    track_row->effect_data=(effect_data&0xf)|pmffx_vslidetype_fine_up;
                  } break;

                  // fine volume slide down
                  case 11:
                  {
                    track_row->effect=pmffx_volume_slide;
                    track_row->effect_data=(effect_data&0xf)|pmffx_vslidetype_fine_down;
                  } break;

                  // note cut
                  case 12:
                  {
                    /*todo*/
//                    track_row->note=pmfcfg_note_cut;
                  } break;

                  // pattern delay
                  case 14:
                  {
                    if(effect_data&0xf)
                    {
                      track_row->effect=pmffx_subfx;
                      track_row->effect_data=(pmfsubfx_pattern_delay<<num_subfx_value_bits)|(effect_data&0xf);
                    }
                  } break;
                }
              } break;

              // set tempo/bpm
              case 15:
              {
                track_row->effect=pmffx_set_speed_tempo;
                track_row->effect_data=effect_data?effect_data:1;
              } break;

              // key off
              case 20:
              {
                track_row->note=0xfe;
              } break;

              // extra fine porta
              case 33:
              {
                switch(effect_data>>4)
                {
                  // extra fine porta up
                  case 1:
                  {
                    track_row->effect=pmffx_note_slide_up;
                    track_row->effect_data=(effect_data&0xf)+0xe0;
                  } break;

                  // extra fine porta down
                  case 2:
                  {
                    track_row->effect=pmffx_note_slide_down;
                    track_row->effect_data=(effect_data&0xf)+0xe0;
                  } break;
                }
              } break;
            }
          }

          // proceed to the next channel row
          ++track_row;
        }
      }
    }
  }

  // read instruments
  song_.instruments.resize(num_instruments);
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    // read instrument header
    uint32 inst_header_size;
    in_file_>>inst_header_size;
    char name[22];
    in_file_.read_bytes(name, 22);
    in_file_.skip(1);
    uint16 num_samples;
    in_file_>>num_samples;
    if(num_samples)
    {
      // read common sample info
      uint32 sample_header_size;
      in_file_>>sample_header_size;
      in_file_.skip(96);
      uint16 vol_envelope_pnts[24], pan_envelope_pnts[24];
      in_file_.read(vol_envelope_pnts, 24);
      in_file_.read(pan_envelope_pnts, 24);
      uint8 num_vol_env_pnts, num_pan_env_pnts;
      uint8 vol_env_sustain_pnt, vol_env_loop_start_pnt, vol_env_loop_end_pnt;
      uint8 pan_env_sustain_pnt, pan_env_loop_start_pnt, pan_env_loop_end_pnt;
      uint8 vol_env_type, pan_env_type;
      in_file_>>num_vol_env_pnts>>num_pan_env_pnts;
      in_file_>>vol_env_sustain_pnt>>vol_env_loop_start_pnt>>vol_env_loop_end_pnt;
      in_file_>>pan_env_sustain_pnt>>pan_env_loop_start_pnt>>pan_env_loop_end_pnt;
      in_file_>>vol_env_type>>pan_env_type;
      uint8 vibrato_type, vibrato_sweep, vibrato_depth, vibrato_rate;
      in_file_>>vibrato_type>>vibrato_sweep>>vibrato_depth>>vibrato_rate;
      uint16 vol_fadeout;
      in_file_>>vol_fadeout;
      in_file_.skip(inst_header_size-29-212);

      // read instrument samples headers
      array<xm_sample> inst_samples(num_samples);
      for(unsigned si=0; si<num_samples; ++si)
      {
        xm_sample &smp=inst_samples[si];
        in_file_>>smp.length>>smp.loop_start>>smp.loop_length;
        in_file_>>smp.volume>>smp.finetune>>smp.type>>smp.panning>>smp.rel_note;
        in_file_>>smp.compression;
        in_file_.skip(sample_header_size-18);
        smp.volume=smp.volume<64?(smp.volume<<2)|(smp.volume>>4):0xff;
        song_.total_instrument_data_bytes+=smp.length;
      }

      // read sample data
      for(unsigned si=0; si<num_samples; ++si)
      {
        xm_sample &smp=inst_samples[si];
        if(smp.type&0x10)
        {
          // read 16-bit sample and convert to 8-bit
          smp.length/=2;
          smp.loop_start/=2;
          smp.loop_length/=2;
          smp.data=PFC_MEM_ALLOC(smp.length);
          int8 *smp_data=(int8*)smp.data.data;
          int16 smp_val=0;
          for(unsigned i=0; i<smp.length; ++i)
          {
            int16 v;
            in_file_>>v;
            smp_val+=v;
            smp_data[i]=smp_val>>8;
          }
        }
        else
        {
          // read 8-bit sample
          smp.data=PFC_MEM_ALLOC(smp.length);
          int8 *smp_data=(int8*)smp.data.data;
          in_file_.read_bytes(smp_data, smp.length);
          int8 smp_val=0;
          for(unsigned i=0; i<smp.length; ++i)
          {
            smp_val+=smp_data[i];
            smp_data[i]=smp_val;
          }
        }
      }

      // add first sample as instrument to the song (don't support multiple samples/instrument)
      xm_sample &smp=inst_samples[0];
      pmf_instrument &inst=song_.instruments[ii];
      inst.length=smp.length;
      inst.loop_start=smp.loop_start;
      inst.loop_len=smp.type&3?smp.loop_length:0;
      inst.volume=smp.volume<64?smp.volume<<2:0xff;
      inst.data=smp.data;
      inst.c4hz=uint32(8363.0f*pow(1.059463094359f, smp.rel_note+smp.finetune/128.0f)+0.5f);
      inst.fadeout_speed=vol_env_type&1?vol_fadeout:65535;

      // set instrument volume envelope
      if(vol_env_type&1)
      {
        if(vol_env_type&2)
          inst.vol_envelope.sustain_loop_start=inst.vol_envelope.sustain_loop_end=vol_env_sustain_pnt;
        if(vol_env_type&4)
        {
          inst.vol_envelope.loop_start=vol_env_loop_start_pnt;
          inst.vol_envelope.loop_end=vol_env_loop_end_pnt;
        }
        inst.vol_envelope.data.resize(num_vol_env_pnts);
        for(unsigned i=0; i<num_vol_env_pnts; ++i)
        {
          inst.vol_envelope.data[i].first=uint8(vol_envelope_pnts[i*2+0]);
          uint16 vol=vol_envelope_pnts[i*2+1];
          inst.vol_envelope.data[i].second=uint8(vol<64?vol<<2:255);
        }
      }
    }
    else
      in_file_.skip(inst_header_size-29);
  }

  return true;
}
//----------------------------------------------------------------------------

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
  uint8 sample_idx;
};
//----------------------------------------------------------------------------


//============================================================================
// convert_xm
//============================================================================
e_pmf_error convert_xm(bin_input_stream_base &in_file_, pmf_song &song_)
{
  // check for XM file type
  char id[17];
  in_file_.read_bytes(id, 17);
  in_file_.rewind();
  if(!mem_eq(id, "Extended Module: ", 17))
    return pmferr_unknown_format;

  // read header
  uint16 version, song_len, restart_pos, num_channels, num_patterns, num_instruments, flags, default_tempo, default_speed;
  uint32 header_size;
  in_file_.skip(17);
  char song_name[21]={0};
  in_file_.read_bytes(song_name, 20);
  in_file_.skip(21);
  in_file_>>version>>header_size>>song_len>>restart_pos>>num_channels>>num_patterns>>num_instruments>>flags>>default_speed>>default_tempo;
  uint8 pattern_orders[256];
  in_file_.read_bytes(pattern_orders, 256);

  // setup song
  song_.name=song_name;
  song_.channels.resize(num_channels);
  song_.flags=flags&1?pmfflag_linear_freq_table:0;
  song_.initial_speed=uint8(default_speed);
  song_.initial_tempo=uint8(default_tempo);
  song_.note_period_min=28;
  song_.note_period_max=27392;

  // setup orders
  song_.playlist.resize(song_len);
  for(unsigned i=0; i<song_len; ++i)
    song_.playlist[i]=pattern_orders[i];

  // read patterns
  in_file_.seek(header_size+60);
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    // read pattern header
    uint32 header_len;
    uint8 packing;
    uint16 num_rows, packed_size;
    in_file_>>header_len>>packing>>num_rows>>packed_size;
    song_.total_src_pattern_data_bytes+=packed_size;

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

          // map volume/volume effect
          if(volume>=0x10 && volume<=0x50)
            track_row->volume=volume<0x50?volume-0x10:63; // set volume
          else if(volume>=0x60 && volume<=0x9f)
            track_row->volume=volume-0x60+0x40;           // volume slide
          else if(volume>=0xa0 && volume<=0xaf)
            track_row->volume=volume-0xa0+0xb0;           // set vibrato speed
          else if(volume>=0xb0 && volume<=0xbf)
            track_row->volume=volume-0xb0+0xc0;           // vibrato
          else if(volume>=0xf0 && volume<=0xff)
            track_row->volume=volume-0xf0+0x0a0;          // tone porta (note slide)

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

              // tremolo
              case 7:
              {
                track_row->effect=pmffx_tremolo;
                track_row->effect_data=effect_data;
              } break;

              // set panning
              case 8:
              {
                /*todo*/
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
                track_row->effect=pmffx_volume_slide;
                if(effect_data&0xf0)
                  track_row->effect_data=(effect_data>>4)|pmffx_vslidetype_up;
                else if(effect_data&0x0f)
                  track_row->effect_data=effect_data|pmffx_vslidetype_down;
                else
                  track_row->effect_data=0;
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

                  // set gliss control
                  case 3:
                  {
                    track_row->effect=pmffx_subfx;
                    track_row->effect_data=(pmfsubfx_set_glissando<<num_subfx_value_bits)|(effect_data&0xf?1:0);
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
                    track_row->effect_data=(pmfsubfx_pattern_loop<<num_subfx_value_bits)|(effect_data&0xf);
                  } break;

                  // set tremolo control
                  case 7:
                  {
                    if((effect_data&0xf)<8)
                    {
                      track_row->effect=pmffx_subfx;
                      track_row->effect_data=(pmfsubfx_set_tremolo_wave<<num_subfx_value_bits)|(effect_data&0xf);
                    }
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
                    track_row->effect=pmffx_subfx;
                    track_row->effect_data=(pmfsubfx_note_cut<<num_subfx_value_bits)|(effect_data&0xf);
                  } break;

                  // note delay
                  case 13:
                  {
                    track_row->effect=pmffx_subfx;
                    track_row->effect_data=(pmfsubfx_note_delay<<num_subfx_value_bits)|(effect_data&0xf);
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
                if(effect_data)
                {
                  track_row->effect=pmffx_set_speed_tempo;
                  track_row->effect_data=effect_data;
                }
              } break;

              // set global volume
              case 16:
              {
                /*todo*/
              } break;

              // global volume slide
              case 17:
              {
                /*todo*/
              } break;

              // key off
              case 20:
              {
                track_row->note=0xfe;
              } break;

              // set envelope pos
              case 21:
              {
                /*todo*/
              } break;

              // panning slide
              case 25:
              {
                /*todo*/
              } break;

              // multi retrig
              case 27:
              {
                /*todo*/
              } break;

              // tremor
              case 29:
              {
                /*todo*/
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
    uint16 num_inst_samples;
    in_file_>>num_inst_samples;
    if(num_inst_samples)
    {
      // update song stats
      ++song_.num_valid_instruments;
      song_.num_valid_samples+=num_inst_samples;

      // read common sample info
      uint32 sample_header_size;
      in_file_>>sample_header_size;
      uint8 note_sample_map[96];
      in_file_.read_bytes(note_sample_map, 96);
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
      array<xm_sample> inst_samples(num_inst_samples);
      for(unsigned si=0; si<num_inst_samples; ++si)
      {
        xm_sample &smp=inst_samples[si];
        in_file_>>smp.length>>smp.loop_start>>smp.loop_length;
        in_file_>>smp.volume>>smp.finetune>>smp.type>>smp.panning>>smp.rel_note;
        in_file_>>smp.compression;
        in_file_.skip(sample_header_size-18);
        smp.volume=smp.volume<64?(smp.volume<<2)|(smp.volume>>4):0xff;
        smp.sample_idx=0xff;
        song_.total_src_sample_data_bytes+=smp.length;
      }

      // read sample data
      for(unsigned si=0; si<num_inst_samples; ++si)
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

      // add instrument samples
      pmf_instrument &pmf_inst=song_.instruments[ii];
      pmf_inst.note_map.resize(120);
      pmf_note_map_entry *nmap=pmf_inst.note_map.data();
      unsigned num_used_samples=0;
      for(unsigned si=0; si<96; ++si)
      {
        xm_sample &smp=inst_samples[note_sample_map[si]];
        if(smp.data.data)
        {
          smp.sample_idx=uint8(song_.samples.size());
          pmf_sample &pmf_smp=song_.samples.push_back();
          pmf_smp.flags=(smp.type&3)==2?pmfsmpflag_bidi_loop:0;
          pmf_smp.length=smp.length;
          pmf_smp.loop_start=smp.loop_start;
          pmf_smp.loop_len=smp.type&3?smp.loop_length:0;
          pmf_smp.volume=smp.volume<64?smp.volume<<2:0xff;
          pmf_smp.data=smp.data;
          pmf_smp.finetune=smp.rel_note*128+smp.finetune;
          pmf_inst.fadeout_speed=vol_env_type&1?uint16(min(vol_fadeout*2, 65535)):65535;
          ++num_used_samples;
        }
        nmap[si+12].sample_idx=smp.sample_idx;
      }
      if(num_used_samples<2)
        pmf_inst.note_map.clear();
      pmf_inst.sample_idx=inst_samples[note_sample_map[12*5]].sample_idx;

      // set instrument volume envelope
      if(vol_env_type&1)
      {
        if(vol_env_type&2)
          pmf_inst.vol_envelope.sustain_loop_start=pmf_inst.vol_envelope.sustain_loop_end=vol_env_sustain_pnt;
        if(vol_env_type&4)
        {
          pmf_inst.vol_envelope.loop_start=vol_env_loop_start_pnt;
          pmf_inst.vol_envelope.loop_end=vol_env_loop_end_pnt;
        }
        pmf_inst.vol_envelope.data.resize(num_vol_env_pnts);
        for(unsigned i=0; i<num_vol_env_pnts; ++i)
        {
          pmf_inst.vol_envelope.data[i].first=uint16(vol_envelope_pnts[i*2+0]);
          uint16 vol=vol_envelope_pnts[i*2+1];
          pmf_inst.vol_envelope.data[i].second=uint16(vol<64?vol<<10:65535);
        }
      }
    }
    else
      in_file_.skip(inst_header_size-29);
  }

  return pmferr_ok;
}
//----------------------------------------------------------------------------

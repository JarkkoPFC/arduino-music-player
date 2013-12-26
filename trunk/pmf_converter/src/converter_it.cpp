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
// config
//============================================================================
enum {max_it_channels=64};
//----------------------------------------------------------------------------


//============================================================================
// note_sample_map
//============================================================================
struct note_sample
{
  uint8 note_idx;
  uint8 sample;
};
//----------------------------------------------------------------------------


//============================================================================
// envelope_node_point
//============================================================================
struct envelope_node_point
{
  uint16 x;
  uint8 y;
};
//----------------------------------------------------------------------------


//============================================================================
// envelope
//============================================================================
struct envelope
{
  envelope()
  {
    flags=0;
    num_nodes=0;
    loop_begin=loop_end=0;
    sustain_loop_begin=sustain_loop_end=0;
    mem_zero(node_points, sizeof(node_points));
  }
  //----

  uint8 flags;
  uint8 num_nodes;
  uint8 loop_begin, loop_end;
  uint8 sustain_loop_begin, sustain_loop_end;
  envelope_node_point node_points[25];
};
//----------------------------------------------------------------------------


//============================================================================
// it_instrument
//============================================================================
struct it_instrument
{
  it_instrument()
  {
    fadeout=0;
    new_note_action=0;
    dup_check_type=dup_check_action=0;
    global_volume=0;
    pitch_pan_separation=0;
    pitch_pan_center=0;
    default_pan=0;
    random_vol_var=random_pan_var=0;
    mem_zero(note_sample_map, sizeof(note_sample_map));
  }
  //----

  uint16 fadeout;
  uint8 new_note_action;
  uint8 dup_check_type, dup_check_action;
  uint8 global_volume;
  int8 pitch_pan_separation;
  uint8 pitch_pan_center;
  uint8 default_pan;
  uint8 random_vol_var, random_pan_var;
  note_sample note_sample_map[120];
  envelope envelopes[3];
};
//----------------------------------------------------------------------------


//============================================================================
// it_sample
//============================================================================
struct it_sample
{
  it_sample()
  {
    global_volume=0;
    flags=0;
    default_vol=0;
    default_pan=0;
    loop_begin=loop_end=0;
    c5speed=0;
    sustain_loop_begin=sustain_loop_end=0;
    vib_speed=vib_depth=vib_type=vib_rate=0;
  }
  //----

  uint8 global_volume;
  uint8 flags;
  uint8 default_vol;
  uint8 default_pan;
  uint32 loop_begin, loop_end;
  uint32 c5speed;
  uint32 sustain_loop_begin, sustain_loop_end;
  uint8 vib_speed, vib_depth, vib_type, vib_rate;
  array<int8> data;
};
//----------------------------------------------------------------------------


//============================================================================
// convert_it
//============================================================================
bool convert_it(bin_input_stream_base &in_file_, pmf_song &song_)
{
  // check for IT file type
  uint32 id;
  in_file_>>id;
  in_file_.rewind();
  if(id!=0x4d504d49)
    return false;

  // read header
  in_file_.skip(0x20);
  uint16 num_ord, num_insts, num_samples, num_patterns, cwt, cmwt, flags, special;
  in_file_>>num_ord>>num_insts>>num_samples>>num_patterns;
  in_file_>>cwt>>cmwt;
  in_file_>>flags>>special;
  uint8 global_vol, mixing_vol, init_speed, init_tempo;
  in_file_>>global_vol>>mixing_vol>>init_speed>>init_tempo;
  uint8 chl_vols[max_it_channels];
  in_file_.seek(0x80);
  in_file_.read_bytes(chl_vols, max_it_channels);
  PFC_ASSERT_MSG(cmwt>=0x200, ("Unable to read IT files older than version 2.0"));

  // setup song
  unsigned num_channels=max_it_channels;
  song_.num_channels=num_channels;
  song_.flags=flags&8?pmfflag_linear_freq_table:0;
  song_.initial_speed=init_speed;
  song_.initial_tempo=init_tempo;
  array<uint8> order_indices(num_ord);
  for(unsigned i=0; i<num_ord; ++i)
  {
    order_indices[i]=uint8(song_.playlist.size());
    uint8 order;
    in_file_>>order;
    if(order<254)
      song_.playlist.push_back(order);
  }

  // read data offsets
  array<uint32> inst_offsets(num_insts);
  array<uint32> sample_offsets(num_samples);
  array<uint32> pattern_offsets(num_patterns);
  in_file_.read_bytes(inst_offsets.data(), 4*num_insts);
  in_file_.read_bytes(sample_offsets.data(), 4*num_samples);
  in_file_.read_bytes(pattern_offsets.data(), 4*num_patterns);

  // read instruments
  array<it_instrument> instruments(num_insts);
  for(unsigned ii=0; ii<num_insts; ++ii)
  {
    // check instrument
    in_file_.seek(inst_offsets[ii]);
    uint32 inst_id;
    in_file_>>inst_id;
    if(inst_id!=0x49504d49)
      continue;
    in_file_.skip(13);

    // read instrument data
    it_instrument &inst=instruments[ii];
    in_file_>>inst.new_note_action;
    in_file_>>inst.dup_check_type>>inst.dup_check_action;
    in_file_>>inst.fadeout;
    in_file_>>inst.pitch_pan_separation>>inst.pitch_pan_center;
    in_file_>>inst.global_volume;
    in_file_>>inst.default_pan;
    in_file_>>inst.random_vol_var>>inst.random_pan_var;
    in_file_.skip(0x24);
    in_file_.read_bytes(inst.note_sample_map, 240);

    // read instrument envelopes
    for(unsigned ei=0; ei<3; ++ei)
    {
      envelope &env=inst.envelopes[ei];
      in_file_>>env.flags;
      in_file_>>env.num_nodes;
      in_file_>>env.loop_begin>>env.loop_end;
      in_file_>>env.sustain_loop_begin>>env.sustain_loop_end;
      for(unsigned i=0; i<25; ++i)
        in_file_>>env.node_points[i].y>>env.node_points[i].x;
    }
  }

  // read samples
  array<it_sample> samples(num_samples);
  for(unsigned si=0; si<num_samples; ++si)
  {
    // check sample
    in_file_.seek(sample_offsets[si]);
    uint32 smp_id;
    in_file_>>smp_id;
    if(smp_id!=0x53504d49)
      continue;
    in_file_.skip(13);

    // read sample header
    it_sample &smp=samples[si];
    in_file_>>smp.global_volume;
    in_file_>>smp.flags;
    in_file_>>smp.default_vol;
    in_file_.skip(26);
    uint8 convert;
    in_file_>>convert;
    in_file_>>smp.default_pan;
    uint32 smp_length, data_offset;
    in_file_>>smp_length;
    in_file_>>smp.loop_begin>>smp.loop_end;
    in_file_>>smp.c5speed;
    in_file_>>smp.sustain_loop_begin>>smp.sustain_loop_end;
    in_file_>>data_offset;
    in_file_>>smp.vib_speed>>smp.vib_depth>>smp.vib_type>>smp.vib_rate;

    // init sample reading
    PFC_ASSERT_MSG(!(smp.flags&4), ("IT module loader doesn't support stereo samples"));
    PFC_ASSERT_MSG(!(convert&2), ("IT module loader doesn't support big-endian samples"));
    PFC_ASSERT_MSG(!(convert&4), ("IT module loader doesn't support delta encoded samples"));
    PFC_ASSERT_MSG(!(convert&16), ("IT module loader doesn't support TX-Wave 12-bit samples"));
    bool is_16bit=(smp.flags&2)!=0;
    smp.data.resize(smp_length);
    int8 *smp_data=smp.data.data();
    in_file_.seek(data_offset);

    // check for compressed sample
    if(smp.flags&8)
    {
      // initialize sample decompression
      static const unsigned max_block_samples=is_16bit?0x4000:0x8000;
      static const unsigned max_data_width=is_16bit?17:9;
      static const unsigned data_size_bits=is_16bit?4:3;
      static const unsigned smp_shift=is_16bit?8:0;

      // decompress sample blocks
      int16 smp=0;
      for(unsigned block_offs=0; block_offs<smp_length; block_offs+=max_block_samples)
      {
        // read block compressed size and process the block
        uint16 block_len;
        in_file_>>block_len;
        song_.total_instrument_data_bytes+=block_len;
        unsigned data_width=max_data_width;
        bit_input_stream bstream(in_file_, block_len*8);
        unsigned smp_idx=0, num_block_samples=min(max_block_samples, smp_length-block_offs);
        while(!bstream.is_eos() && smp_idx<num_block_samples)
        {
          // get sample data
          int32 delta_smp;
          bstream.read_bits(delta_smp, data_width);
          if(data_width<7)
          {
            // type A: check for data width change
            if(delta_smp==int32(-1)<<(data_width-1))
            {
              uint32 new_data_width;
              bstream.read_bits(new_data_width, data_size_bits);
              if(++new_data_width>=data_width)
                ++new_data_width;
              data_width=new_data_width;
              continue;
            }
          }
          else if(data_width==max_data_width)
          {
            // type C: check for data width change
            if(delta_smp<0)
            {
              data_width=(delta_smp&255)+1;
              if(!data_width || data_width>max_data_width)
                break;
              continue;
            }
          }
          else
          {
            // type B: check for data width change
            if(   delta_smp>=(1<<(data_width-1))-(1<<(data_size_bits-1))
               || delta_smp<=~((1<<(data_width-1))-(1<<(data_size_bits-1))))
            {
              unsigned new_data_width=((delta_smp+(1<<(data_size_bits-1)))&15)+1;
              if(new_data_width>=data_width)
                ++new_data_width;
              data_width=new_data_width;
              continue;
            }
          }

          // add new sample
          smp+=int16(delta_smp);
          smp_data[block_offs+smp_idx]=int8(smp>>smp_shift);
          ++smp_idx;
        }

        // set remaining samples in the block
        mem_set(smp_data+block_offs+smp_idx, smp>>8, num_block_samples-smp_idx);
      }
    }
    else
    {
      // read uncompressed sample data
      if(is_16bit)
      {
        // read 16-bit sample
        for(unsigned i=0; i<smp_length; ++i)
        {
          uint16 v;
          in_file_>>v;
          smp_data[i]=v>>8;
        }
        song_.total_instrument_data_bytes+=smp_length*2;
      }
      else
      {
        // read 8-bit sample
        in_file_.read_bytes(smp_data, smp_length);
        song_.total_instrument_data_bytes+=smp_length;
      }
    }

    // convert to signed if unsigned
    if(!(convert&1))
      for(unsigned i=0; i<smp_length; ++i)
        smp_data[i]-=0x80;
  }

  // read patterns
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    unsigned file_offset=pattern_offsets[pi];
    pmf_pattern &pattern=song_.patterns.push_back();
    if(file_offset)
    {
      // read pattern header
      in_file_.seek(pattern_offsets[pi]);
      uint16 packed_length, num_rows;
      in_file_>>packed_length>>num_rows;
      in_file_.skip(4);
      PFC_ASSERT(num_rows>=32 && num_rows<=200);
      song_.total_pattern_data_bytes+=packed_length;

      // init pattern memories
      uint8 channel_data_masks[max_it_channels];
      uint8 channel_notes[max_it_channels];
      uint8 channel_instruments[max_it_channels]={0xff};
      uint8 channel_volumes[max_it_channels]={0xff};
      uint8 channel_effects[max_it_channels];
      uint8 channel_effect_datas[max_it_channels]={0};
      mem_zero(channel_data_masks, max_it_channels);
      mem_set(channel_notes, 255, max_it_channels);
      mem_set(channel_instruments, 255, max_it_channels);
      mem_set(channel_volumes, 255, max_it_channels);
      mem_set(channel_effects, 255, max_it_channels);
      mem_zero(channel_effect_datas, max_it_channels);

      // unpack pattern
      pattern.num_rows=num_rows;
      pattern.rows.resize(num_rows*num_channels);
      pmf_pattern_track_row *pattern_data=pattern.rows.data();
      for(unsigned ri=0; ri<num_rows; ++ri)
      {
        // process row
        while(true)
        {
          // check for end-of-row
          uint8 data;
          in_file_>>data;
          if(!data)
            break;

          // get channel and data mask
          uint8 channel=(data-1)&63;
          if(data&128)
            in_file_>>channel_data_masks[channel];
          uint8 data_mask=channel_data_masks[channel];
          pmf_pattern_track_row &track_row=pattern_data[channel];

          // get note
          if(data_mask&1)
          {
            uint8 note;
            in_file_>>note;
            if(note<120)
              channel_notes[channel]=track_row.note=note;
            else if(note==254 || note==255)
              channel_notes[channel]=track_row.note=note==254?pmfcfg_note_cut:pmfcfg_note_off;
          }
          if(data_mask&16)
            track_row.note=channel_notes[channel];

          // get instrument
          if(data_mask&2)
          {
            uint8 instrument;
            in_file_>>instrument;
            if(instrument && instrument<100)
              channel_instruments[channel]=track_row.instrument=instrument-1;
          }
          if(data_mask&32)
            track_row.instrument=channel_instruments[channel];

          // get volume
          if(data_mask&4)
          {
            // read volume and ignore volume column effects (i.e. volume/pitch slides & vibrato)
            uint8 volume;
            in_file_>>volume;
            if(volume<=64)
              channel_volumes[channel]=track_row.volume=volume<63?volume:63;
          }
          if(data_mask&64)
            track_row.volume=channel_volumes[channel];

          // get command
          if(data_mask&8)
          {
            // map command to PMF effect
            uint8 command, command_info;
            in_file_>>command>>command_info;
            switch(command)
            {
              // A: Set tempo
              case 1:
              {
                if(command_info>32)
                {
                  track_row.effect=pmffx_set_speed_tempo;
                  track_row.effect_data=command_info;
                }
              } break;

              // B: Jump to order
              case 2:
              {
                if(command_info<order_indices.size())
                {
                  track_row.effect=pmffx_position_jump;
                  track_row.effect_data=order_indices[command_info];
                }
              } break;

              // C: Break to row
              case 3:
              {
                track_row.effect=pmffx_pattern_break;
                track_row.effect_data=command_info;
              } break;

              // D: Volume slide
              case 4:
              {
                track_row.effect=pmffx_volume_slide;
                if(!command_info)
                  track_row.effect_data=0;
                else if((command_info&0x0f)==0)
                {
                  if(command_info!=0xf0)
                    track_row.effect_data=pmffx_vslidetype_up|(command_info>>4);
                  else
                    track_row.effect_data=pmffx_vslidetype_fine_up|0xf;
                }
                else if((command_info&0xf0)==0)
                {
                  if(command_info!=0x0f)
                    track_row.effect_data=pmffx_vslidetype_down|command_info;
                  else
                    track_row.effect_data=pmffx_vslidetype_fine_down|0xf;
                }
                else if((command_info&0x0f)==0x0f)
                  track_row.effect_data=pmffx_vslidetype_fine_up|(command_info>>4);
                else if((command_info&0xf0)==0xf0)
                  track_row.effect_data=pmffx_vslidetype_fine_down|(command_info&0xf);
                else
                  track_row.effect=0xff;
              } break;

              // H: Vibrato
              case 8:
              {
                track_row.effect=pmffx_vibrato;
                track_row.effect_data=command_info;
              } break;

              // J: Arpeggio
              case 10:
              {
                track_row.effect=pmffx_arpeggio;
                track_row.effect_data=command_info;
              } break;
            }
          }
          if(data_mask&128)
          {
            track_row.effect=channel_effects[channel];
            track_row.effect_data=channel_effect_datas[channel];
          }
        }

        // move to next row
        pattern_data+=num_channels;
      }
    }
    else
    {
      // add empty pattern
      pattern.num_rows=64;
      pattern.rows.resize(64*num_channels);
    }
  }

  // add PMF instruments
  song_.instruments.resize(num_insts);
  for(unsigned ii=0; ii<num_insts; ++ii)
  {
    // get first sample in the instrument mapping table
    const it_instrument &inst=instruments[ii];
    it_sample *smp=0;
    for(unsigned i=0; i<120; ++i)
    {
      if(inst.note_sample_map[i].sample)
      {
        smp=&samples[inst.note_sample_map[i].sample-1];
        break;
      }
    }

    if(smp)
    {
      // setup PMF instrument
      pmf_instrument &pmf_inst=song_.instruments[ii];
      pmf_inst.length=smp->data.size();
      pmf_inst.loop_len=smp->flags&16?smp->loop_end-smp->loop_begin:0;
      pmf_inst.loop_start=smp->loop_begin;
      pmf_inst.c4hz=smp->c5speed;
      pmf_inst.volume=smp->default_vol<64?smp->default_vol<<2:255;
      pmf_inst.data=smp->data.steal_data().steal_data();
    }
  }

  return true;
}
//----------------------------------------------------------------------------

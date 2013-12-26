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
// convert_s3m
//============================================================================
bool convert_s3m(bin_input_stream_base &in_file_, pmf_song &song_)
{
  // check for S3M file
  in_file_.skip(0x2c);
  uint32 s3m_id;
  in_file_>>s3m_id;
  in_file_.rewind();
  if(s3m_id!=0x4d524353)
    return false;

  // read s3m header
  uint16 num_orders, num_inst, num_patterns, flags, tracker_ver, ffi;
  uint8 global_vol, master_vol, init_speed, init_tempo, usnd_cr, dpp;
  uint8 chl_settings[32];
  in_file_.skip(0x20);
  in_file_>>num_orders>>num_inst>>num_patterns>>flags>>tracker_ver>>ffi;
  in_file_.skip(4);
  in_file_>>global_vol>>init_speed>>init_tempo>>master_vol>>usnd_cr>>dpp;
  in_file_.skip(10);
  in_file_.read_bytes(chl_settings, 32);
  PFC_CHECK_MSG((flags&64)==0, ("S3M ST3.0 volume slides not supported"));

  // setup song
  song_.num_channels=32;
  song_.flags=(flags&4)?0:pmfflag_fast_note_slides;
  song_.initial_speed=init_speed?init_speed:6;
  song_.initial_tempo=init_tempo<32?125:init_tempo;

  // read orders
  array<uint8> orders;
  array<uint8> order_indices(num_orders);
  for(unsigned i=0; i<num_orders; ++i)
  {
    order_indices[i]=uint8(orders.size());
    uint8 order;
    in_file_>>order;
    if(order<254)
      orders.push_back(order);
  }
  num_orders=uint16(orders.size());

  // read instrument and pattern para-pointers and default pan positions
  array<uint16> inst_pptrs(num_inst);
  array<uint16> pat_pptrs(num_patterns);
  uint8 def_ppos[32]={0};
  in_file_.read_bytes(inst_pptrs.data(), num_inst*2);
  in_file_.read_bytes(pat_pptrs.data(), num_patterns*2);
  if(dpp==252)
    in_file_.read_bytes(def_ppos, 32);
  song_.playlist.resize(num_orders);
  for(unsigned i=0; i<num_orders; ++i)
    song_.playlist[i]=orders[i];

  // read instruments
  song_.instruments.resize(num_inst);
  for(unsigned i=0; i<num_inst; ++i)
  {
    // read instrument parameters
    unsigned file_offset=inst_pptrs[i]*16;
    in_file_.seek(file_offset);
    uint32 length, loop_begin, loop_end, data_offset=0;
    uint16 c2spd;
    uint8 type, volume, packed, flags;
    in_file_>>type;
    in_file_.skip(0xc);
    in_file_.read_bytes(&data_offset, 3);
    data_offset=(((data_offset&0xff)<<16)+(data_offset>>8))*16;
    in_file_>>length>>loop_begin>>loop_end>>volume;
    length&=0xffff; loop_begin&=0xffff; loop_end&=0xffff;
    in_file_.skip(1);
    in_file_>>packed>>flags>>c2spd;
    bool has_loop=(flags&1)!=0;

    // setup instrument and read instrument data
    if(type)
    {
      // check instrument type
      PFC_CHECK_MSG(type==1, ("S3M loader doesn't support Adlib instruments"))
      PFC_CHECK_MSG((flags&2)==0, ("S3M loader doesn't support stereo samples"));
      PFC_CHECK_MSG((flags&4)==0, ("S3M loader doesn't support 16-bit samples"));
      PFC_CHECK_MSG(packed==0, ("S3M loader doesn't support ADPCM packed samples"));

      // set instrument data
      song_.total_instrument_data_bytes+=length;
      pmf_instrument &inst=song_.instruments[i];
      inst.length=uint16(length);
      inst.loop_start=has_loop?uint16(loop_begin):0;
      inst.loop_len=has_loop?uint16(loop_end-loop_begin):0;
      inst.c4hz=c2spd;
      inst.volume=volume<64?(volume<<2)|(volume>>4):0xff;
      inst.data=PFC_MEM_ALLOC(inst.length);
      in_file_.seek(data_offset);
      in_file_.read_bytes(inst.data.data, inst.length);
      if(ffi==2)
      {
        // convert sample from 8-bit unsigned to 8-bit signed
        uint8 *d=(uint8*)inst.data.data;
        for(unsigned i=0; i<inst.length; ++i)
          d[i]-=0x80;
      }
    }
  }

  // read patterns
  song_.patterns.resize(num_patterns);
  for(unsigned i=0; i<num_patterns; ++i)
  {
    // parse packed pattern data
    unsigned file_offset=pat_pptrs[i]*16;
    in_file_.seek(file_offset+2);
    pmf_pattern &pat=song_.patterns[i];
    pat.rows.resize(64*32);
    unsigned start_pattern_data_pos=in_file_.pos();
    unsigned row=0;
    while(row<64)
    {
      // unpack row
      while(true)
      {
        // read channel row desc and check for end of row
        uint8 desc;
        in_file_>>desc;
        if(!desc)
          break;

        // unpack data
        pmf_pattern_track_row &track_row=pat.rows[row*32+(desc&31)];
        if(desc&32)
        {
          uint8 note, instrument;
          in_file_>>note>>instrument;
          uint8 note_idx=(1+(note>>4))*12+(note&0xf);
          if(note!=255)
          {
            PFC_ASSERT(note==254 || note_idx<120);
            track_row.note=note!=254?note_idx:pmfcfg_note_cut;
          }
          track_row.instrument=instrument?instrument-1:0xff;
        }
        if(desc&64)
        {
          uint8 volume;
          in_file_>>volume;
          track_row.volume=volume<64?volume:63;
        }
        if(desc&128)
        {
          uint8 command, command_info;
          in_file_>>command>>command_info;
          enum {fx_offset=1-'A'};
          switch(command)
          {
            // A: set speed
            case 'A'+fx_offset:
            {
              if(command_info && command_info<=32)
              {
                track_row.effect=pmffx_set_speed_tempo;
                track_row.effect_data=command_info;
              }
            } break;

            // B: pattern jump
            case 'B'+fx_offset:
            {
              if(command_info<order_indices.size())
              {
                track_row.effect=pmffx_position_jump;
                track_row.effect_data=order_indices[command_info];
              }
            } break;

            // C: pattern break
            case 'C'+fx_offset:
            {
              uint8 row=(command_info>>4)*10+(command_info&0xf);
              if(row<64)
              {
                track_row.effect=pmffx_pattern_break;
                track_row.effect_data=row;
              }
            } break;

            // D: volume slide
            case 'D'+fx_offset:
            {
              if((command_info&0xf0)==0xf0)
              {
                if(command_info&0x0f)
                {
                  // fine-slide down
                  track_row.effect=pmffx_volume_slide;
                  track_row.effect_data=(command_info&0x0f)|pmffx_vslidetype_fine_down;
                }
              }
              else if((command_info&0x0f)==0x0f)
              {
                if(command_info&0xf0)
                {
                  // fine-slide up
                  track_row.effect=pmffx_volume_slide;
                  track_row.effect_data=(command_info&0x0f)|pmffx_vslidetype_fine_up;
                }
              }
              else
              {
                if(!(command_info&0xf0) || !(command_info&0x0f))
                {
                  // normal volume slide
                  track_row.effect=pmffx_volume_slide;
                  track_row.effect_data=command_info&0xf0?(command_info>>4)|pmffx_vslidetype_up:((command_info&0x0f)|pmffx_vslidetype_down);
                }
              }
            } break;

            // E: portamento down
            case 'E'+fx_offset:
            {
              if(command_info!=0xe0 && command_info!=0xf0)
              {
                track_row.effect=pmffx_note_slide_down;
                track_row.effect_data=command_info;
              }
            } break;

            // F: portamento up
            case 'F'+fx_offset:
            {
              if(command_info!=0xe0 && command_info!=0xf0)
              {
                track_row.effect=pmffx_note_slide_up;
                track_row.effect_data=command_info;
              }
            } break;

            // G: tone portamento
            case 'G'+fx_offset:
            {
              track_row.effect=pmffx_note_slide;
              track_row.effect_data=command_info<0xe0?command_info:0xdf;
            } break;

            // H: vibrato
            case 'H'+fx_offset:
            {
              track_row.effect=pmffx_vibrato;
              track_row.effect_data=command_info;
            } break;

            // J: arpeggio
            case 'J'+fx_offset:
            {
              if(command_info)
              {
                track_row.effect=pmffx_arpeggio;
                track_row.effect_data=command_info;
              }
            } break;

            // K: vibrato + volume slide
            case 'K'+fx_offset:
            {
              if(command_info&0xf0 && command_info&0x0f)
                break;
              track_row.effect=pmffx_vibrato_vol_slide;
              track_row.effect_data=command_info<0x10?pmffx_vslidetype_down+command_info:(pmffx_vslidetype_up+(command_info>>4));
            } break;

            // L: portamento + volume slide
            case 'L'+fx_offset:
            {
              // ignore illegal volume slide
              if(command_info&0xf0 && command_info&0x0f)
                break;
              track_row.effect=pmffx_note_vol_slide;
              track_row.effect_data=command_info<0x10?pmffx_vslidetype_down+command_info:(pmffx_vslidetype_up+(command_info>>4));
            } break;

            case 'O'+fx_offset:
            {
              track_row.effect=pmffx_set_sample_offs;
              track_row.effect_data=command_info;
            } break;

            // retrigger sample + volume slide
            case 'Q'+fx_offset:
            {
              if(command_info&0x0f)
              {
                track_row.effect=pmffx_retrig_vol_slide;
                track_row.effect_data=command_info;
              }
            } break;

            // S: sub-effect
            case 'S'+fx_offset:
            {
              switch(command_info>>4)
              {
                // set finetune
                case 0x2:
                {
                  track_row.effect=pmffx_subfx;
                  track_row.effect_data=(pmfsubfx_set_finetune<<num_subfx_value_bits)|(((command_info&0xf)-8)&subfx_value_mask);
                } break;

                // set vibrato waveform
                case 0x3:
                {
                  if((command_info&0xf)<8)
                  {
                    track_row.effect=pmffx_subfx;
                    track_row.effect_data=(pmfsubfx_set_vibrato_wave<<num_subfx_value_bits)|(command_info&0xf);
                  }
                } break;

                // loop pattern
                case 0xb:
                {
                  track_row.effect=pmffx_subfx;
                  track_row.effect_data=(pmfsubfx_loop_pattern<<num_subfx_value_bits)|(command_info&0xf);
                } break;

                // pattern delay
                case 0xe:
                {
                  if(command_info&0xf)
                  {
                    track_row.effect=pmffx_subfx;
                    track_row.effect_data=(pmfsubfx_pattern_delay<<num_subfx_value_bits)|(command_info&0xf);
                  }
                } break;
              }
            } break;

            // T: set tempo
            case 'T'+fx_offset:
            {
              if(command_info>32)
              {
                track_row.effect=pmffx_set_speed_tempo;
                track_row.effect_data=command_info;
              }
            } break;
          }
        }
      }
      ++row;
    }
    song_.total_pattern_data_bytes+=in_file_.pos()-start_pattern_data_pos;
  }

  return true;
}
//----------------------------------------------------------------------------

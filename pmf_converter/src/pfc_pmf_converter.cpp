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

#include "sxp_src/core/main.h"
#include "sxp_src/core/fsys/fsys.h"
#include "sxp_src/core/containers.h"
#include "sxp_src/core/sort.h"
#include "pfc_pmf_converter.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// PMF format config
//============================================================================
// PMF config
enum {pmf_converter_version=0x0600}; // v0.6
enum {pmf_file_version=0x1400}; // v1.4
// PMF file structure
enum {pmfcfg_offset_signature=PFC_OFFSETOF(pmf_header, signature)};
enum {pmfcfg_offset_version=PFC_OFFSETOF(pmf_header, version)};
enum {pmfcfg_offset_flags=PFC_OFFSETOF(pmf_header, flags)};
enum {pmfcfg_offset_file_size=PFC_OFFSETOF(pmf_header, file_size)};
enum {pmfcfg_offset_smp_meta_offs=PFC_OFFSETOF(pmf_header, sample_meta_offs)};
enum {pmfcfg_offset_inst_meta_offs=PFC_OFFSETOF(pmf_header, instrument_meta_offs)};
enum {pmfcfg_offset_pat_meta_offs=PFC_OFFSETOF(pmf_header, pattern_meta_offs)};
enum {pmfcfg_offset_nmap_data_offs=PFC_OFFSETOF(pmf_header, nmap_data_offs)};
enum {pmfcfg_offset_env_data_offs=PFC_OFFSETOF(pmf_header, env_data_offs)};
enum {pmfcfg_offset_track_data_offs=PFC_OFFSETOF(pmf_header, track_data_offs)};
enum {pmfcfg_offset_init_speed=PFC_OFFSETOF(pmf_header, initial_speed)};
enum {pmfcfg_offset_init_tempo=PFC_OFFSETOF(pmf_header, initial_tempo)};
enum {pmfcfg_offset_note_period_min=PFC_OFFSETOF(pmf_header, note_period_min)};
enum {pmfcfg_offset_note_period_max=PFC_OFFSETOF(pmf_header, note_period_max)};
enum {pmfcfg_offset_playlist_length=PFC_OFFSETOF(pmf_header, playlist_length)};
enum {pmfcfg_offset_num_channels=PFC_OFFSETOF(pmf_header, num_channels)};
enum {pmfcfg_offset_num_patterns=PFC_OFFSETOF(pmf_header, num_patterns)};
enum {pmfcfg_offset_num_instruments=PFC_OFFSETOF(pmf_header, num_instruments)};
enum {pmfcfg_offset_num_samples=PFC_OFFSETOF(pmf_header, num_samples)};
enum {pmfcfg_offset_playlist=PFC_OFFSETOF(pmf_header, first_playlist_entry)};
enum {pmfcfg_pattern_metadata_header_size=2};
enum {pmfcfg_pattern_metadata_track_offset_size=2};
enum {pmfcfg_offset_pattern_metadata_last_row=0};
enum {pmfcfg_offset_pattern_metadata_track_offsets=2};
// PMF sample config
enum {pmfcfg_sample_metadata_size=sizeof(pmf_sample_header)};
enum {pmfcfg_offset_smp_data=PFC_OFFSETOF(pmf_sample_header, data_offset)};
enum {pmfcfg_offset_smp_length=PFC_OFFSETOF(pmf_sample_header, length)};
enum {pmfcfg_offset_smp_loop_length_and_panning=PFC_OFFSETOF(pmf_sample_header, loop_length_and_panning)};
enum {pmfcfg_offset_smp_finetune=PFC_OFFSETOF(pmf_sample_header, finetune)};
enum {pmfcfg_offset_smp_flags=PFC_OFFSETOF(pmf_sample_header, flags)};
enum {pmfcfg_offset_smp_volume=PFC_OFFSETOF(pmf_sample_header, volume)};
// PMF instrument config
enum {pmfcfg_instrument_metadata_size=sizeof(pmf_instrument_header)};
enum {pmfcfg_offset_inst_vol_env=PFC_OFFSETOF(pmf_instrument_header, vol_env_offset)};
enum {pmfcfg_offset_inst_pitch_env=PFC_OFFSETOF(pmf_instrument_header, pitch_env_offset)};
enum {pmfcfg_offset_inst_fadeout_speed=PFC_OFFSETOF(pmf_instrument_header, fadeout_speed)};
enum {pmfcfg_offset_inst_volume=PFC_OFFSETOF(pmf_instrument_header, volume)};
enum {pmfcfg_offset_inst_panning=PFC_OFFSETOF(pmf_instrument_header, panning)};
// envelope configs
enum {pmfcfg_offset_env_num_points=0};
enum {pmfcfg_offset_env_loop_start=1};
enum {pmfcfg_offset_env_loop_end=2};
enum {pmfcfg_offset_env_sustain_loop_start=3};
enum {pmfcfg_offset_env_sustain_loop_end=4};
enum {pmfcfg_offset_env_points=6};
enum {pmfcfg_envelope_point_size=4};
enum {pmfcfg_offset_env_point_tick=0};
enum {pmfcfg_offset_env_point_val=2};
// note map config
enum {pmfcfg_max_note_map_regions=8};
enum {pmfcfg_offset_nmap_num_entries=0};
enum {pmfcfg_offset_nmap_entries=1};
enum {pmfcfg_nmap_entry_size_direct=2};
enum {pmfcfg_nmap_entry_size_range=3};
enum {pmgcfg_offset_nmap_entry_note_idx_offs=0};
enum {pmgcfg_offset_nmap_entry_sample_idx=1};
// bit-compression settings
enum {pmfcfg_num_data_mask_bits=4};
enum {pmfcfg_num_note_bits=7};       // max 10 octaves (0-9) (12*10=120)
enum {pmfcfg_num_instrument_bits=6}; // max 64 instruments
enum {pmfcfg_num_volume_bits=6};     // volume range [0, 63]
enum {pmfcfg_num_effect_bits=4};     // effects 0-15
enum {pmfcfg_num_effect_data_bits=8};
//----
enum {pmfcfg_max_instruments=1<<pmfcfg_num_instrument_bits};
//----------------------------------------------------------------------------


//============================================================================
// locals
//============================================================================
static const char *s_converter_name="PMF Converter";
static const char *s_copyright_message="Copyright (c) 2019, Profoundic Technologies, Inc. All rights reserved.";
static const char *s_usage_message="Usage: pmf_converter [options] -i <input.mod/s3m/xm/it> -o <output.pmf>   (-h for help)";
//----------------------------------------------------------------------------


//============================================================================
// bcd16_version_str
//============================================================================
stack_str8 bcd16_version_str(uint16 version_)
{
  stack_str8 s;
  s.format("%i.", version_>>12);
  version_<<=4;
  do
  {
    s.push_back_format("%x", (version_>>12));
    version_<<=4;
  } while(version_);
  return s;
}
//----------------------------------------------------------------------------


//============================================================================
// parse_command_arguments
//============================================================================
struct command_arguments
{
  command_arguments()
  {
    max_channels=64;
    output_binary=true;
    output_dwords=false;
    enable_data_ref_optim=true;
    suppress_copyright=false;
  }
  //----

  heap_str input_file;
  heap_str friendly_input_file;
  heap_str output_file;
  unsigned max_channels;
  bool output_binary;
  bool output_dwords;
  bool enable_data_ref_optim;
  bool suppress_copyright;
};
//----

bool parse_command_arguments(command_arguments &ca_, const char **args_, unsigned num_args_)
{
  // parse arguments
  for(unsigned i=0; i<num_args_; ++i)
  {
    // check compiler for option
    if(args_[i][0]=='-')
    {
      // switch to proper argument handling
      usize_t arg_size=str_size(args_[i]);
      switch(to_lower(args_[i][1]))
      {
        // help
        case 'h':
        {
          if(arg_size==2)
          {
            // output help
            logf("%s v%s\r\n" // converter name & version
                 "%s\r\n" // copyright
                 "\r\n"
                 "%s\r\n" // usage
                 "\r\n"
                 "Options:\r\n"
                 "  -o <file>       Output filename\r\n"
                 "\r\n"
                 "  -hex            Output data as comma separated ASCII hex codes (instead of binary)\r\n"
                 "  -hexd           Use dwords instead of bytes for ASCII output\r\n"
                 "  -ch <num_chl>   Maximum number of channels (Default: 64)\r\n"
                 "  -dro            Disable data reference optimizations\r\n"
                 "\r\n"
                 "  -h              Print this screen\n"
                 "  -c              Suppress copyright message\r\n", 
                 s_converter_name, bcd16_version_str(pmf_converter_version).c_str(),
                 s_copyright_message,
                 s_usage_message);
            return false;
          }
          else if(arg_size==4)
          {
            // output ASCII hex codes (bytes)
            if(str_ieq(args_[i], "-hex"))
            {
              ca_.output_binary=false;
              ca_.output_dwords=false;
            }
          }
          else if(arg_size==5)
          {
            // output ASCII hex codes (dwords)
            if(str_ieq(args_[i], "-hexd"))
            {
              ca_.output_binary=false;
              ca_.output_dwords=true;
            }
          }
        } break;

        // input file
        case 'i':
        {
          if(arg_size==2 && i<num_args_-1)
          {
            ca_.input_file=args_[i+1];
            ca_.friendly_input_file=get_filename(ca_.input_file.c_str());
            str_lower(ca_.friendly_input_file.c_str());
            ++i;
          }
        } break;

        // output file
        case 'o':
        {
          if(arg_size==2 && i<num_args_-1)
          {
            ca_.output_file=args_[i+1];
            ++i;
          }
        } break;

        case 'c':
        {
          // suppress copyright text
          if(arg_size==2)
            ca_.suppress_copyright=true;
          else if(arg_size==3 && i<num_args_-1 && str_ieq(args_[i], "-ch"))
          {
            // get max channels
            int64 max_channels=0;
            if(str_to_int64(max_channels, args_[i+1]) && max_channels>0)
              ca_.max_channels=(unsigned)max_channels;
          }
        } break;

        case 'd':
        {
          // disable data reference optimizations
          if(arg_size==4 && str_ieq(args_[i], "-dro"))
            ca_.enable_data_ref_optim=false;
        } break;
      }
    }
  }

  // check for help string and copyright message output
  if(!ca_.suppress_copyright)
  {
    logf("%s v%s\r\n", s_converter_name, bcd16_version_str(pmf_converter_version).c_str());
    logf("%s\r\n\r\n", s_copyright_message);
  }
  if(!ca_.input_file.size() || !ca_.output_file.size())
  {
    log(s_usage_message);
    return false;
  }
  return true;
}
//----------------------------------------------------------------------------


//============================================================================
// write_bits
//============================================================================
void write_bits(array<uint8> &comp_data_, unsigned &bit_pos_, unsigned num_bits_, uint8 v_)
{
  // write given number of bits to the arrays
  if(!bit_pos_)
    comp_data_.push_back(0);
  comp_data_.back()|=v_<<bit_pos_;
  bit_pos_+=num_bits_;
  if(bit_pos_>7)
  {
    bit_pos_-=8;
    if(bit_pos_)
    {
      comp_data_.push_back(0);
      comp_data_.back()|=v_>>(num_bits_-bit_pos_);
    }
  }
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_channel
//============================================================================
pmf_channel::pmf_channel()
{
  panning=0;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_pattern_track_row
//============================================================================
pmf_pattern_track_row::pmf_pattern_track_row()
{
  clear();
}
//----

void pmf_pattern_track_row::clear()
{
  note=0xff;
  instrument=0xff;
  volume=0xff;
  effect=0xff;
  effect_data=0;
}
//----

bool pmf_pattern_track_row::operator==(const pmf_pattern_track_row &row_) const
{
  return    note==row_.note
         && instrument==row_.instrument
         && volume==row_.volume
         && effect==row_.effect
         && effect_data==row_.effect_data;
}
//----

bool pmf_pattern_track_row::is_empty() const
{
  return    note==0xff
         && instrument==0xff
         && volume==0xff
         && effect==0xff
         && effect_data==0;
}
//----

bool pmf_pattern_track_row::is_global_effect() const
{
  // check for global effect
  uint8 sub_effect=effect_data>>4;
  return    effect==pmffx_set_speed_tempo
         || effect==pmffx_position_jump
         || effect==pmffx_pattern_break
         || (effect==pmffx_subfx && (   sub_effect==pmfsubfx_pattern_delay
                                     || sub_effect==pmfsubfx_pattern_loop));
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_pattern_track
//============================================================================
struct pmf_pattern_track
{
  pmf_pattern_track()
  {
    offset=0;
  }
  //----

  bool operator==(const pmf_pattern_track &track_) const
  {
    return mem_eq(rows.data(), track_.rows.data(), min(rows.size(), track_.rows.size())*sizeof(track_.rows[0]));
  }
  //--------------------------------------------------------------------------

  usize_t offset;
  array<uint8> compressed_data;
  array<pmf_pattern_track_row> rows;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_pattern
//============================================================================
pmf_pattern::pmf_pattern()
{
  num_rows=64;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_envelope
//============================================================================
pmf_envelope::pmf_envelope()
{
  loop_start=255;
  loop_end=255;
  sustain_loop_start=255;
  sustain_loop_end=255;
}
//----

bool pmf_envelope::operator==(const pmf_envelope &env_) const
{
  // check for matching envelopes
  usize_t num_pnts=data.size();
  if(   loop_start!=env_.loop_start
     || loop_end!=env_.loop_end
     || sustain_loop_start!=env_.sustain_loop_start
     || sustain_loop_end!=env_.sustain_loop_end
     || num_pnts!=env_.data.size())
    return false;
  return mem_eq(data.data(), env_.data.data(), num_pnts*sizeof(*data.data()));
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_instrument
//============================================================================
pmf_instrument::pmf_instrument()
{
  sample_idx=unsigned(-1);
  fadeout_speed=65535;
  volume=0xff;
  panning=-128;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_sample
//============================================================================
pmf_sample::pmf_sample()
{
  length=0;
  loop_start=0;
  loop_len=0;
  finetune=0;
  flags=0;
  volume=0;      // [0, 255]
  panning=-128;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_song
//============================================================================
pmf_song::pmf_song()
{
  flags=0;
  initial_speed=6;
  initial_tempo=125;
  note_period_min=28;
  note_period_max=27392;
  num_valid_instruments=0;
  num_valid_samples=0;
  total_src_pattern_data_bytes=0;
  total_src_sample_data_bytes=0;
}
//----------------------------------------------------------------------------


//============================================================================
// pattern_info
//============================================================================
struct pattern_info
{
  pattern_info()
  {
    is_referred=false;
    index=0;
    num_rows=0;
  }
  //--------------------------------------------------------------------------

  bool is_referred;
  unsigned index;
  unsigned num_rows; // [1, 256]
  array<usize_t> tracks;
};
//----------------------------------------------------------------------------


//============================================================================
// instrument_info
//============================================================================
struct instrument_info
{
  instrument_info()
  {
    is_referred=false;
    index=0;
    vol_env_offset=unsigned(-1);
    pitch_env_offset=unsigned(-1);
    mem_zero(ref_notes, sizeof(ref_notes));
    num_note_map_regions=0;
  }
  //--------------------------------------------------------------------------

  bool is_referred;
  unsigned index;
  usize_t vol_env_offset;
  usize_t pitch_env_offset;
  uint8 ref_notes[120];
  unsigned num_note_map_regions;
  array<uint8> note_map_data;
};
//----------------------------------------------------------------------------


//============================================================================
// sample_info
//============================================================================
struct sample_info
{
  sample_info()
  {
    is_referred=false;
    index=0;
    data_offset=0;
    cropped_len=0;
  }
  //--------------------------------------------------------------------------

  bool is_referred;
  unsigned index;
  usize_t data_offset;
  usize_t cropped_len;
};
//----------------------------------------------------------------------------


//============================================================================
// comp_type
//============================================================================
struct comp_type
{
  uint8 type;
  usize_t size;
};
//----

PFC_INLINE bool operator<(const comp_type &ct0_, const comp_type &ct1_)
{
  return ct0_.size<ct1_.size;
}
//----------------------------------------------------------------------------


//============================================================================
// write_pmf_file
//============================================================================
void write_pmf_file(pmf_song &song_, const command_arguments &ca_)
{
  // get song info
  const unsigned num_channels=(unsigned)song_.channels.size();
  const usize_t num_patterns=song_.patterns.size();
  const usize_t num_instruments=song_.instruments.size();
  const usize_t num_samples=song_.samples.size();
  const usize_t playlist_length=song_.playlist.size();
  const usize_t num_virtual_instruments=num_instruments?num_instruments:num_samples;

  // check for patterns referred by the playlist
  unsigned num_active_patterns=0;
  array<pattern_info> pat_infos(num_patterns);
  for(unsigned i=0; i<num_patterns; ++i)
  {
    pattern_info &pinfo=pat_infos[i];
    pinfo.tracks.resize(num_channels);
  }
  for(unsigned i=0; i<playlist_length; ++i)
  {
    uint8 pat_idx=song_.playlist[i];
    pattern_info &pinfo=pat_infos[pat_idx];
    pinfo.is_referred=true;
  }

  // re-index patterns
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    pattern_info &pinfo=pat_infos[pi];
    if(!ca_.enable_data_ref_optim || pinfo.is_referred)
      pinfo.index=num_active_patterns++;
  }

  // check for instruments referred by active patterns & mark active channels
  array<uint8> active_channels(num_channels, uint8(0));
  array<instrument_info> inst_infos(num_virtual_instruments);
  array<sample_info> smp_infos(num_samples);
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    pattern_info &pinfo=pat_infos[pi];
    if(ca_.enable_data_ref_optim && !pinfo.is_referred)
      continue;
    pinfo.num_rows=song_.patterns[pi].num_rows;
    PFC_ASSERT(pinfo.num_rows>0 && pinfo.num_rows<=256);
    pmf_pattern_track_row *row_data=song_.patterns[pi].rows.data();
    const unsigned num_pattern_rows=song_.patterns[pi].num_rows;
    for(unsigned ri=0; ri<num_pattern_rows; ++ri)
      for(unsigned ci=0; ci<num_channels; ++ci)
      {
        uint8 note_idx=row_data->note;
        if(note_idx!=0xff)
          active_channels[ci]|=1;
        uint8 inst=row_data->instrument;
        if(inst!=0xff && inst<num_virtual_instruments)
        {
          if(note_idx==pmfcfg_note_cut || note_idx==pmfcfg_note_off)
            row_data->instrument=0xff;
          else
          {
            active_channels[ci]|=2;
            inst_infos[inst].is_referred=true;
            if(note_idx<120)
              inst_infos[inst].ref_notes[note_idx]=1;
          }
        }
        if(row_data->is_global_effect())
          active_channels[ci]|=4;
        ++row_data;
      }
  }

  // set active channel indices (for empty channel removal)
  array<uint8> active_channel_map;
  for(uint8 i=0; i<num_channels; ++i)
  {
    if(!ca_.enable_data_ref_optim || (active_channels[i] && active_channel_map.size()<ca_.max_channels))
      active_channel_map.push_back(i);
  }
  const unsigned num_active_channels=(unsigned)active_channel_map.size();

  // re-index the instruments and setup envelopes and sample references
  array<pmf_envelope> envelopes;
  array<usize_t> env_offsets;
  usize_t total_note_map_data_size=0;
  usize_t total_envelope_data_size=0;
  unsigned num_active_instruments=0;
  for(unsigned ii=0; ii<num_virtual_instruments; ++ii)
  {
    pmf_instrument dummy_inst;
    dummy_inst.sample_idx=ii;
    const pmf_instrument &inst=num_instruments?song_.instruments[ii]:dummy_inst;
    instrument_info &iinfo=inst_infos[ii];
    if(   (!ca_.enable_data_ref_optim || iinfo.is_referred)
       && inst.sample_idx!=unsigned(-1)
       && song_.samples[inst.sample_idx].length)
    {
      iinfo.index=num_active_instruments++;
      if(inst.vol_envelope.data.size())
      {
        // check if envelope for volume exists
        const pmf_envelope *env=linear_search(envelopes.begin(), envelopes.size(), inst.vol_envelope);
        if(!env)
        {
          // add new envelope
          env_offsets.push_back(total_envelope_data_size);
          envelopes.push_back(inst.vol_envelope);
          env=envelopes.last();
          total_envelope_data_size+=pmfcfg_offset_env_points+env->data.size()*pmfcfg_envelope_point_size;
        }

        // set instrument volume envelope data offset
        unsigned env_index=unsigned(env-envelopes.begin());
        iinfo.vol_env_offset=env_offsets[env_index];
      }

      if(inst.pitch_envelope.data.size())
      {
        // check if the envelope for pitch exists
        const pmf_envelope *env=linear_search(envelopes.begin(), envelopes.size(), inst.pitch_envelope);
        if(!env)
        {
          // add new envelope
          env_offsets.push_back(total_envelope_data_size);
          envelopes.push_back(inst.pitch_envelope);
          env=envelopes.last();
          total_envelope_data_size+=pmfcfg_offset_env_points+env->data.size()*pmfcfg_envelope_point_size;
        }

        // set instrument pitch envelope data offset
        unsigned env_index=unsigned(env-envelopes.begin());
        iinfo.pitch_env_offset=env_offsets[env_index];
      }

      // set sample references
      if(inst.note_map.size())
      {
        const pmf_note_map_entry *nmap=inst.note_map.data();
        for(unsigned ni=0; ni<120; ++ni)
          if(iinfo.ref_notes[ni])
          {
            uint8 sample_idx=nmap[ni].sample_idx;
            if(sample_idx!=0xff)
              smp_infos[sample_idx].is_referred=true;
          }
      }
      else if(inst.sample_idx!=unsigned(-1))
        smp_infos[inst.sample_idx].is_referred=true;
    }
  }
  if(!num_instruments)
    num_active_instruments=0;

  // re-index samples and calculate sample data offsets
  unsigned num_active_samples=0;
  usize_t total_sample_data_bytes=0;
  for(unsigned si=0; si<num_samples; ++si)
  {
    sample_info &sinfo=smp_infos[si];
    const pmf_sample &smp=song_.samples[si];
    if(sinfo.is_referred || (!ca_.enable_data_ref_optim && !num_instruments && smp.length))
    {
      sinfo.index=num_active_samples++;
      sinfo.data_offset=total_sample_data_bytes;
      sinfo.cropped_len=smp.loop_len?smp.loop_start+smp.loop_len:smp.length;
      total_sample_data_bytes+=sinfo.cropped_len+1;
    }
  }

  // build note map data
  if(num_instruments)
    for(unsigned ii=0; ii<num_instruments; ++ii)
    {
      instrument_info &iinfo=inst_infos[ii];
      const pmf_instrument &inst=song_.instruments[ii];
      if((!ca_.enable_data_ref_optim || iinfo.is_referred) && inst.note_map.size())
      {
        // build note map (region and direct map)
        array<uint8> region_note_map, direct_note_map;
        region_note_map.push_back(0);
        direct_note_map.push_back(120);
        const pmf_note_map_entry *nmap=inst.note_map.data();
        int8 prev_note_idx_offs=0;
        uint8 prev_sample_idx=0xff;
        unsigned num_note_map_regions=0;
        for(unsigned ni=0; ni<120; ++ni)
        {
          int8 note_idx_offs=nmap[ni].note_idx_offs;
          uint8 sample_idx=nmap[ni].sample_idx;
          if(iinfo.ref_notes[ni])
          {
            // check for sample/note offset change to store note map region
            if(sample_idx!=0xff && prev_sample_idx!=0xff && (prev_note_idx_offs!=note_idx_offs || prev_sample_idx!=sample_idx))
            {
              uint8 reg_data[]={uint8(ni-1), uint8(prev_note_idx_offs), uint8(smp_infos[prev_sample_idx].index)};
              region_note_map.insert_back(sizeof(reg_data), reg_data);
              ++num_note_map_regions;
            }
            prev_note_idx_offs=note_idx_offs;
            prev_sample_idx=sample_idx;
          }

          // store direct map entry
          uint8 nme[]={uint8(note_idx_offs), uint8(sample_idx!=0xff?smp_infos[sample_idx].index:0xff)};
          direct_note_map.insert_back(sizeof(nme), nme);
        }

        if(num_note_map_regions || prev_note_idx_offs)
        {
          if(num_note_map_regions<pmfcfg_max_note_map_regions)
          {
            // add the last region entry and store region map for the instrument
            uint8 reg_data[]={119, uint8(prev_note_idx_offs), uint8(smp_infos[prev_sample_idx].index)};
            region_note_map.insert_back(sizeof(reg_data), reg_data);
            iinfo.num_note_map_regions=++num_note_map_regions;
            region_note_map[0]=uint8(num_note_map_regions);
            iinfo.note_map_data.swap(region_note_map);
          }
          else
          {
            // store direct note map for the instrument
            iinfo.num_note_map_regions=120;
            iinfo.note_map_data.swap(direct_note_map);
          }
          total_note_map_data_size+=iinfo.note_map_data.size();
        }
      }
    }

  // split patterns to unique pattern tracks
  deque<pmf_pattern_track> tracks;
  pmf_pattern_track temp_track;
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    // check if pattern is referred by the playlist
    pattern_info &pinfo=pat_infos[pi];
    if(ca_.enable_data_ref_optim && !pinfo.is_referred)
      continue;

    const pmf_pattern &pattern=song_.patterns[pi];
    for(unsigned ci=0; ci<num_active_channels; ++ci)
    {
      // build temp track
      temp_track.rows.resize(pattern.num_rows);
      unsigned chl_idx=active_channel_map[ci];
      bool valid_instrument_state=true;
      for(unsigned ri=0; ri<pattern.num_rows; ++ri)
      {
        // copy track row
        pmf_pattern_track_row &row=temp_track.rows[ri];
        row=pattern.rows[chl_idx+ri*num_channels];

        // validate track row data
        if(row.instrument!=0xff && row.note<120)
        {
          // verify proper instrument
          valid_instrument_state=true;
          if(row.instrument>=num_virtual_instruments)
            valid_instrument_state=false;
          else
          {
            unsigned sample_idx=row.instrument;
            if(num_instruments)
            {
              const pmf_instrument &inst=song_.instruments[row.instrument];
              if(inst.note_map.size())
                sample_idx=inst.note_map[row.note].sample_idx;
              else
                sample_idx=inst.sample_idx;
            }
            if(sample_idx>=num_samples || !smp_infos[sample_idx].cropped_len)
              valid_instrument_state=false;
          }
        }

        if(!valid_instrument_state)
        {
          // kill row for invalid instrument
          if(row.note!=0xff)
            row.note=pmfcfg_note_cut;
          row.instrument=0xff;
          row.volume=0xff;
          if(!row.is_global_effect())
            row.effect=0xff;
        }
      }

      // search for track from unique tracks
      deque<pmf_pattern_track>::iterator it=linear_search(tracks.begin(), tracks.size(), temp_track);
      if(is_valid(it))
      {
        if(temp_track.rows.size()>it->rows.size())
          it->rows.swap(temp_track.rows);
      }
      else
      {
        tracks.push_back(temp_track);
        it=tracks.last();
      }
      pinfo.tracks[chl_idx]=it.index();
    }
  }
  usize_t num_tracks=tracks.size();
  unsigned num_chl_tracks=num_active_patterns*num_active_channels;
  float track_uniqueness=num_chl_tracks?100.0f*float(num_tracks)/float(num_chl_tracks):0;

  // compress tracks
  usize_t total_compressed_track_bytes=0;
  for(unsigned ti=0; ti<num_tracks; ++ti)
  {
    // track stats
    unsigned num_empty_rows=0;
    uint8 compression_type=0xff;
    unsigned total_num_dmask4_packed_bits=0;
    unsigned total_num_dmask8_packed_bits=0;
    unsigned total_num_dmasks=0;

    // check for track volume effects
    pmf_pattern_track &track=tracks[ti];
    usize_t num_rows=track.rows.size();
    bool has_volume_effect=false;
    uint8 num_volume_bits=pmfcfg_num_volume_bits;
    const pmf_pattern_track_row *track_row=track.rows.data();
    for(unsigned ri=0; ri<num_rows; ++ri)
    {
      uint8 volume=track_row[ri].volume;
      if(volume!=0xff && volume>=(1<<pmfcfg_num_volume_bits))
      {
        has_volume_effect=true;
        num_volume_bits=8;
        break;
      }
    }

    // process track in two passes: pass 0=collect stats, pass 1=compress
    unsigned bit_pos=0;
    for(unsigned pass=0; pass<2; ++pass)
    {
      // setup track data buffers
      uint8 dmask_buf[2]={0, 0};
      uint8 note_buf[2]={0xff, 0xff};
      uint8 inst_buf[2]={0xff, 0xff};
      uint8 volume_buf[2]={0xff, 0xff};
      uint8 effect_buf[2]={0xff, 0xff};
      uint8 effect_data_buf[2]={0, 0};

      // check compression type
      if(pass==1)
      {
        // get compression type that results in the smallest size
        if(num_empty_rows==num_rows)
          break;
        const usize_t total_num_dmask4_packed_bits_sparse=total_num_dmask4_packed_bits-num_empty_rows*4+num_rows;
        const usize_t total_num_dmask8_packed_bits_sparse=total_num_dmask8_packed_bits-num_empty_rows*8+num_rows;
        const usize_t total_num_dmask4_packed_bits_dmask_ref=total_num_dmask4_packed_bits-num_rows*(4-2)+total_num_dmasks*4;
        const usize_t total_num_dmask8_packed_bits_dmask_ref=total_num_dmask8_packed_bits-num_rows*(8-2)+total_num_dmasks*8;
        const comp_type ctypes[]={{0x0, total_num_dmask4_packed_bits},
                                  {0x1, total_num_dmask4_packed_bits_sparse},
                                  {0x2, total_num_dmask4_packed_bits_dmask_ref},
                                  {0x4, total_num_dmask8_packed_bits},
                                  {0x5, total_num_dmask8_packed_bits_sparse},
                                  {0x6, total_num_dmask8_packed_bits_dmask_ref},
        };
        compression_type=find_min(ctypes, sizeof(ctypes)/sizeof(*ctypes))->type;

        // check for volume effect
        if(has_volume_effect)
          compression_type|=0x8;

        // write track compression type
        write_bits(track.compressed_data, bit_pos, 4, compression_type);
      }

      // compress track
      const pmf_pattern_track_row *track_row=track.rows.data();
      for(unsigned ri=0; ri<num_rows; ++ri)
      {
        // track row stats
        unsigned num_dmask4_packed_bits=4;
        unsigned num_dmask8_packed_bits=8;

        // check for note
        uint8 data_mask=0;
        uint8 note=track_row->note;
        if(note!=0xff)
        {
          num_dmask4_packed_bits+=pmfcfg_num_note_bits;
          if(note==note_buf[0])
            data_mask|=0x10;
          else if(note==note_buf[1])
            data_mask|=0x11;
          else
          {
            num_dmask8_packed_bits+=pmfcfg_num_note_bits;
            data_mask|=0x01;
            note_buf[1]=note_buf[0];
            note_buf[0]=note;
          }
        }

        // check for instrument
        uint8 inst=track_row->instrument;
        if(inst!=0xff)
        {
          num_dmask4_packed_bits+=pmfcfg_num_instrument_bits;
          if(inst==inst_buf[0])
            data_mask|=0x20;
          else if(inst==inst_buf[1])
            data_mask|=0x22;
          else
          {
            num_dmask8_packed_bits+=pmfcfg_num_instrument_bits;
            data_mask|=0x02;
            inst_buf[1]=inst_buf[0];
            inst_buf[0]=inst;
          }
        }

        // check for volume
        uint8 volume=track_row->volume;
        if(volume!=0xff)
        {
          num_dmask4_packed_bits+=num_volume_bits;
          if(volume==volume_buf[0])
            data_mask|=0x40;
          else if(volume==volume_buf[1])
            data_mask|=0x44;
          else
          {
            num_dmask8_packed_bits+=num_volume_bits;
            data_mask|=0x04;
            volume_buf[1]=volume_buf[0];
            volume_buf[0]=volume;
          }
        }

        // check effect
        uint8 effect=track_row->effect, effect_data=track_row->effect_data;
        if(effect!=0xff)
        {
          num_dmask4_packed_bits+=pmfcfg_num_effect_bits+pmfcfg_num_effect_data_bits;
          if(effect==effect_buf[0] && effect_data==effect_data_buf[0])
            data_mask|=0x80;
          else if(effect==effect_buf[1] && effect_data==effect_data_buf[1])
            data_mask|=0x88;
          else
          {
            num_dmask8_packed_bits+=pmfcfg_num_effect_bits+pmfcfg_num_effect_data_bits;
            data_mask|=0x08;
            effect_buf[1]=effect_buf[0];
            effect_data_buf[1]=effect_data_buf[0];
            effect_buf[0]=effect;
            effect_data_buf[0]=effect_data;
          }
        }

        if(pass==0)
        {
          // collect stats for compression
          if(!data_mask)
            ++num_empty_rows;
          total_num_dmask4_packed_bits+=num_dmask4_packed_bits;
          total_num_dmask8_packed_bits+=num_dmask8_packed_bits;
          if(data_mask && data_mask!=dmask_buf[0] && data_mask!=dmask_buf[1])
          {
            dmask_buf[1]=dmask_buf[0];
            dmask_buf[0]=data_mask;
            ++total_num_dmasks;
          }
        }
        else
        {
          // write data mask
          uint8 dmask_size=8;
          if(!(compression_type&0x4))
          {
            data_mask=(data_mask|(data_mask>>4))&0xf;
            dmask_size=4;
          }
          switch(compression_type&0x3)
          {
            // store mask
            case 0x0:
            {
              write_bits(track.compressed_data, bit_pos, dmask_size, data_mask);
            } break;

            // sparse track
            case 0x1:
            {
              write_bits(track.compressed_data, bit_pos, 1, data_mask?1:0);
              if(data_mask)
                write_bits(track.compressed_data, bit_pos, dmask_size, data_mask);
            } break;

            // dmask reference
            case 0x2:
            {
              uint8 ref_value=0;
              if(data_mask)
              {
                ref_value=1;
                if(data_mask==dmask_buf[0])
                  ref_value=2;
                else if(data_mask==dmask_buf[1])
                  ref_value=3;
              }
              write_bits(track.compressed_data, bit_pos, 2, ref_value);
              if(ref_value==1)
              {
                write_bits(track.compressed_data, bit_pos, dmask_size, data_mask);
                dmask_buf[1]=dmask_buf[0];
                dmask_buf[0]=data_mask;
              }
            } break;
          }

          // write note data
          if((data_mask&0x11)==0x01)
          {
            PFC_ASSERT(note<(1<<pmfcfg_num_note_bits));
            write_bits(track.compressed_data, bit_pos, pmfcfg_num_note_bits, note);
          }
          
          // write instrument data
          if((data_mask&0x22)==0x02)
          {
            unsigned inst_idx=inst_infos[inst].index;
            PFC_ASSERT(inst_idx<(1<<pmfcfg_num_instrument_bits));
            write_bits(track.compressed_data, bit_pos, pmfcfg_num_instrument_bits, uint8(inst_idx));
          }

          // write volume data
          if((data_mask&0x44)==0x04)
          {
            write_bits(track.compressed_data, bit_pos, num_volume_bits, volume);
          }

          // write effect data
          if((data_mask&0x88)==0x08)
          {
            PFC_ASSERT(effect<(1<<pmfcfg_num_effect_bits));
            write_bits(track.compressed_data, bit_pos, pmfcfg_num_effect_bits, effect);
            write_bits(track.compressed_data, bit_pos, pmfcfg_num_effect_data_bits, effect_data);
          }
        }

        // proceed to the next row
        ++track_row;
      }
    }
    track.offset=total_compressed_track_bytes;
    total_compressed_track_bytes+=track.compressed_data.size();
  }

  // calculate data offsets
  const usize_t num_padding_bytes=(0-playlist_length-num_active_channels)&3;
  const usize_t header_size=pmfcfg_offset_playlist+playlist_length+num_active_channels+num_padding_bytes;
  const usize_t base_offs_sample_metadata=header_size;
  const usize_t base_offs_instrument_metadata=base_offs_sample_metadata+num_active_samples*sizeof(pmf_sample_header);
  const usize_t base_offs_pattern_metadata=base_offs_instrument_metadata+num_active_instruments*sizeof(pmf_instrument_header);
  const usize_t pattern_bytes=num_active_patterns*(pmfcfg_pattern_metadata_header_size+num_active_channels*pmfcfg_pattern_metadata_track_offset_size);
  const usize_t base_offs_envelope_data=base_offs_pattern_metadata+pattern_bytes;
  const usize_t base_offs_note_map_data=base_offs_envelope_data+total_envelope_data_size;
  const usize_t base_offs_track_data=base_offs_note_map_data+total_note_map_data_size;
  const usize_t base_offs_sample_data=base_offs_track_data+total_compressed_track_bytes;
  const usize_t total_file_size=base_offs_sample_data+total_sample_data_bytes;

  // write PMF header
  array<uint8> pmf_data;
  container_output_stream<array<uint8> > out_stream(pmf_data);
  out_stream<<uint32(0x78666d70);  // "pmfx"
  out_stream<<uint16(pmf_file_version);
  out_stream<<uint16(song_.flags);
  out_stream<<uint32(total_file_size);
  out_stream<<uint32(base_offs_sample_metadata);
  out_stream<<uint32(base_offs_instrument_metadata);
  out_stream<<uint32(base_offs_pattern_metadata);
  out_stream<<uint32(base_offs_envelope_data);
  out_stream<<uint32(base_offs_note_map_data);
  out_stream<<uint32(base_offs_track_data);
  out_stream<<uint8(song_.initial_speed);
  out_stream<<uint8(song_.initial_tempo);
  out_stream<<uint16(song_.note_period_min);
  out_stream<<uint16(song_.note_period_max);
  out_stream<<uint16(playlist_length);
  out_stream<<uint8(num_active_channels);
  out_stream<<uint8(num_active_patterns);
  out_stream<<uint8(num_active_instruments);
  out_stream<<uint8(num_active_samples);
  for(unsigned i=0; i<playlist_length; ++i)
    out_stream<<uint8(pat_infos[song_.playlist[i]].index);
  for(unsigned i=0; i<num_active_channels; ++i)
    out_stream<<int8(song_.channels[active_channel_map[i]].panning);
  for(unsigned i=0; i<num_padding_bytes; ++i)
    out_stream<<uint8(0);

  // write sample metadata
  PFC_ASSERT(out_stream.pos()==base_offs_sample_metadata);
  for(unsigned si=0; si<num_samples; ++si)
  {
    const sample_info &sinfo=smp_infos[si];
    const pmf_sample &smp=song_.samples[si];
    if(sinfo.is_referred || (!ca_.enable_data_ref_optim && !num_instruments && smp.length))
    {
      usize_t sample_file_offset=base_offs_sample_data+sinfo.data_offset;
      out_stream<<uint32(sample_file_offset);
      out_stream<<uint32(sinfo.cropped_len);
      out_stream<<((uint32(smp.loop_len<sinfo.cropped_len?smp.loop_len:sinfo.cropped_len)&0xffffff)|(uint32(smp.panning)<<24));
      out_stream<<int16(smp.finetune);
      out_stream<<uint8(smp.flags);
      out_stream<<uint8(smp.volume);
    }
  }

  // write instrument metadata
  PFC_ASSERT(out_stream.pos()==base_offs_instrument_metadata);
  unsigned num_active_inst=0;
  usize_t note_map_offs=0;
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    const instrument_info &iinfo=inst_infos[ii];
    const pmf_instrument &inst=song_.instruments[ii];
    if(   (!ca_.enable_data_ref_optim || iinfo.is_referred)
       && inst.sample_idx!=unsigned(-1)
       && song_.samples[inst.sample_idx].length)
    {
      if(usize_t nmap_size=iinfo.note_map_data.size())
      {
        out_stream<<uint16(note_map_offs+num_active_samples);
        note_map_offs+=nmap_size;
      }
      else
        out_stream<<uint16(smp_infos[inst.sample_idx].index);
      out_stream<<uint16(iinfo.vol_env_offset);
      out_stream<<uint16(iinfo.pitch_env_offset);
      out_stream<<uint16(inst.fadeout_speed);
      out_stream<<uint8(inst.volume);
      out_stream<<uint8(inst.panning);
      ++num_active_inst;
    }
  }

  // write pattern metadata
  PFC_ASSERT(out_stream.pos()==base_offs_pattern_metadata);
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    const pattern_info &pinfo=pat_infos[pi];
    if(!ca_.enable_data_ref_optim || pinfo.is_referred)
    {
      out_stream<<uint8(pinfo.num_rows-1);
      out_stream<<uint8(0);
      for(unsigned ci=0; ci<num_active_channels; ++ci)
      {
        unsigned chl_idx=active_channel_map[ci];
        const pmf_pattern_track &track=tracks[pinfo.tracks[chl_idx]];
        out_stream<<uint16(track.compressed_data.size()?base_offs_track_data+track.offset:0);
      }
    }
  }

  // write envelope data
  PFC_ASSERT(out_stream.pos()==base_offs_envelope_data);
  usize_t num_envelopes=envelopes.size();
  for(unsigned ei=0; ei<num_envelopes; ++ei)
  {
    const pmf_envelope &env=envelopes[ei];
    out_stream<<uint8(env.data.size());
    out_stream<<uint8(env.loop_start);
    out_stream<<uint8(env.loop_end);
    out_stream<<uint8(env.sustain_loop_start!=0xff?env.sustain_loop_start:env.loop_start);
    out_stream<<uint8(env.sustain_loop_end!=0xff?env.sustain_loop_end:env.loop_end);
    out_stream<<uint8(0);
    out_stream.write(env.data.data(), env.data.size());
  }

  // write note mapping data
  PFC_ASSERT(out_stream.pos()==base_offs_note_map_data);
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    const instrument_info &iinfo=inst_infos[ii];
    if(!ca_.enable_data_ref_optim || iinfo.is_referred)
      out_stream.write_bytes(iinfo.note_map_data.data(), iinfo.note_map_data.size());
  }

  // write track data
  PFC_ASSERT(out_stream.pos()==base_offs_track_data);
  for(unsigned ti=0; ti<num_tracks; ++ti)
  {
    const pmf_pattern_track &track=tracks[ti];
    out_stream.write_bytes(track.compressed_data.data(), track.compressed_data.size());
  }

  // write sample data
  PFC_ASSERT(out_stream.pos()==base_offs_sample_data);
  for(unsigned si=0; si<num_samples; ++si)
  {
    const sample_info &sinfo=smp_infos[si];
    usize_t smp_len=smp_infos[si].cropped_len;
    if(smp_len && (!ca_.enable_data_ref_optim || sinfo.is_referred))
    {
      out_stream.write_bytes(song_.samples[si].data.data, smp_len);
      out_stream<<((uint8*)song_.samples[si].data.data)[smp_len-1];
    }
  }
  out_stream.flush();

  // log PMF info
  float pattern_data_compression=song_.total_src_pattern_data_bytes?100.0f*float(total_compressed_track_bytes+pattern_bytes)/float(song_.total_src_pattern_data_bytes):0.0f;
  float sample_data_compression=song_.total_src_sample_data_bytes?100.0f*float(total_sample_data_bytes)/float(song_.total_src_sample_data_bytes):0.0f;
  logf("Song name: %s%s(%s)\r\n", song_.name.c_str(), song_.name.size()?" ":"", ca_.friendly_input_file.c_str());
  logf("Playlist length: %i\r\n", playlist_length);
  logf("Active channels: %i\r\n", num_active_channels);
  logf("Active patterns: %i\r\n", num_active_patterns);
  logf("Active inst|samp: %i|%i (orig %i|%i)\r\n", num_active_instruments, num_active_samples, song_.num_valid_instruments, song_.num_valid_samples);
  logf("Unique pattern tracks %i/%i (%3.1f%%)\r\n", num_tracks, num_active_patterns*num_active_channels, track_uniqueness);
  logf("PMF pattern data size: %i bytes (%3.1f%% of orig %i bytes)\r\n", total_compressed_track_bytes+pattern_bytes, pattern_data_compression, song_.total_src_pattern_data_bytes);
  logf("PMF sample data size: %i bytes (%3.1f%% of orig %i bytes)\r\n", total_sample_data_bytes, sample_data_compression, song_.total_src_sample_data_bytes);
  logf("Total PMF binary size: %i bytes\r\n", total_file_size);

  // write data as binary or hex codes
  owner_ref<bin_output_stream_base> out_file=afs_open_write(ca_.output_file.c_str());
  if(ca_.output_binary)
    out_file->write_bytes(pmf_data.data(), pmf_data.size());
  else
  {
    // write song info as comments
    text_output_stream(*out_file)<<"// Song name: "<<song_.name.c_str()<<(song_.name.size()?" (":"(")<<ca_.friendly_input_file.c_str()<<")\r\n"
                                 <<"//    Length: "<<playlist_length<<"\r\n"
                                 <<"//  Channels: "<<num_active_channels<<"\r\n"
                                 <<"//      Size: "<<total_file_size<<" bytes\r\n"
                                 <<"//  Exporter: "<<s_converter_name<<" v"<<bcd16_version_str(pmf_converter_version).c_str()<<" (PMF v"<<bcd16_version_str(pmf_file_version).c_str()<<")\r\n";

    if(ca_.output_dwords)
    {
      // write data as dword hex codes
      stack_str32 strbuf;
      pmf_data.insert_back((0-pmf_data.size())&3, uint8(0));
      usize_t dwords_left=pmf_data.size()/4;
      const uint32 *dwords=(const uint32*)pmf_data.data();
      while(dwords_left)
      {
        usize_t num_dwords=min<usize_t>(dwords_left, 128);
        for(unsigned i=0; i<num_dwords; ++i)
        {
          strbuf.format("0x%08x, ", dwords[i]);
          *out_file<<strbuf.c_str();
        }
        *out_file<<"\r\n";
        dwords+=num_dwords;
        dwords_left-=num_dwords;
      }
    }
    else
    {
      // write data as byte hex codes
      stack_str32 strbuf;
      usize_t data_left=pmf_data.size();
      const uint8 *bytes=pmf_data.data();
      while(data_left)
      {
        usize_t num_bytes=min<usize_t>(data_left, 256);
        for(unsigned i=0; i<num_bytes; ++i)
        {
          strbuf.format("0x%02x, ", bytes[i]);
          *out_file<<strbuf.c_str();
        }
        *out_file<<"\r\n";
        bytes+=num_bytes;
        data_left-=num_bytes;
      }
    }
  }
}
//----------------------------------------------------------------------------


//============================================================================
// converters
//============================================================================
e_pmf_error convert_mod(bin_input_stream_base&, pmf_song&);
e_pmf_error convert_s3m(bin_input_stream_base&, pmf_song&);
e_pmf_error convert_xm(bin_input_stream_base&, pmf_song&);
e_pmf_error convert_it(bin_input_stream_base&, pmf_song&);
//----------------------------------------------------------------------------


//============================================================================
// main
//============================================================================
PFC_MAIN(const char *args_[], unsigned num_args_)
{
  // parse arguments
  command_arguments ca;
  if(!parse_command_arguments(ca, args_, num_args_))
    return -1;
  owner_ref<file_system_base> fsys=create_default_file_system(true);

  // convert PMF file
  owner_ptr<bin_input_stream_base> in_file=afs_open_read(ca.input_file.c_str(), 0, fopencheck_none);
  if(!in_file.data)
  {
    logf("Unable to open file \"%s\" for reading\r\n", ca.input_file.c_str());
    return -1;
  }
  pmf_song song;
  e_pmf_error err=pmferr_unknown_format;
  if(err==pmferr_unknown_format)
    err=convert_mod(*in_file, song);
  if(err==pmferr_unknown_format)
    err=convert_s3m(*in_file, song);
  if(err==pmferr_unknown_format)
    err=convert_xm(*in_file, song);
  if(err==pmferr_unknown_format)
    err=convert_it(*in_file, song);

  // check for conversion failure
  switch(err)
  {
    case pmferr_ok: break;
    case pmferr_unknown_format: errorf("Unknown input file format - the file not converted\r\n"); return -1;
    case pmferr_conversion_failure: errorf("File conversion failed\r\n"); return -1;
  }

  // write PMF file
  song.name.resize(str_strip_outer_whitespace(song.name.c_str(), true));
  write_pmf_file(song, ca);
  logf("\r\nConversion Succeeded!\r\n");
  return 0;
}
//----------------------------------------------------------------------------

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
enum {pmf_converter_version=0x0420}; // v0.42
enum {pmf_file_version=0x1210};      // v1.21
// PMF file structure
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
// PMF instrument config
enum {pmfcfg_instrument_metadata_size=sizeof(pmf_instrument_header)};
enum {pmfcfg_offset_inst_offset=PFC_OFFSETOF(pmf_instrument_header, data_offset)};
enum {pmfcfg_offset_inst_length=PFC_OFFSETOF(pmf_instrument_header, length)};
enum {pmfcfg_offset_inst_loop_length=PFC_OFFSETOF(pmf_instrument_header, loop_length)};
enum {pmfcfg_offset_inst_vol_env=PFC_OFFSETOF(pmf_instrument_header, vol_env_offset)};
enum {pmfcfg_offset_inst_fadeout_speed=PFC_OFFSETOF(pmf_instrument_header, fadeout_speed)};
enum {pmfcfg_offset_inst_finetune=PFC_OFFSETOF(pmf_instrument_header, finetune)};
enum {pmfcfg_offset_inst_flags=PFC_OFFSETOF(pmf_instrument_header, flags)};
enum {pmfcfg_offset_inst_volume=PFC_OFFSETOF(pmf_instrument_header, default_volume)};
// envelope configs
enum {pmfcfg_offset_env_num_points=0};
enum {pmfcfg_offset_env_sustain_loop_start=1};
enum {pmfcfg_offset_env_sustain_loop_end=2};
enum {pmfcfg_offset_env_loop_start=3};
enum {pmfcfg_offset_env_loop_end=4};
enum {pmfcfg_offset_env_points=5};
enum {pmfcfg_envelope_point_size=2};
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
    enable_optim=true;
    suppress_copyright=false;
  }
  //----

  heap_str input_file;
  heap_str friendly_input_file;
  heap_str output_file;
  unsigned max_channels;
  bool output_binary;
  bool enable_optim;
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
            // output ASCII hex codes
            if(str_ieq(args_[i], "-hex"))
              ca_.output_binary=false;
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
          // suppress copyright test
          if(arg_size==2)
            ca_.suppress_copyright=true;
          else if(arg_size==3 && i<num_args_-1 && str_ieq(args_[i], "-ch"))
          {
            // get max channels
            int max_channels=0;
            if(str_to_int(max_channels, args_[i+1]) && max_channels>0)
              ca_.max_channels=max_channels;
          }
        } break;

        case 'd':
        {
          // disable data reference optimizations
          if(arg_size==4 && str_ieq(args_[i], "-dro"))
            ca_.enable_optim=false;
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
  sustain_loop_start=255;
  sustain_loop_end=255;
  loop_start=255;
  loop_end=255;
}
//----

bool pmf_envelope::operator==(const pmf_envelope &env_) const
{
  // check for matching envelopes
  usize_t num_pnts=data.size();
  if(   sustain_loop_start!=env_.sustain_loop_start
     || sustain_loop_end!=env_.sustain_loop_end
     || loop_start!=env_.loop_start
     || loop_end!=env_.loop_end
     || num_pnts!=env_.data.size())
    return false;
  for(usize_t i=0; i<num_pnts; ++i)
    if(data[i]!=env_.data[i])
      return false;
  return true;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_instrument
//============================================================================
pmf_instrument::pmf_instrument()
{
  sample_idx=unsigned(-1);
  fadeout_speed=65535;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_sample
//============================================================================
pmf_sample::pmf_sample()
{
  volume=0;      // [0, 255]
  flags=0;
  length=0;
  loop_start=0;
  loop_len=0;
  finetune=0;
}
//----------------------------------------------------------------------------


//============================================================================
// pmf_song
//============================================================================
pmf_song::pmf_song()
{
  num_channels=0;
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
// pmf_pattern_info
//============================================================================
struct pmf_pattern_info
{
  pmf_pattern_info()
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
  }
  //--------------------------------------------------------------------------

  bool is_referred;
  unsigned index;
  usize_t vol_env_offset;
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
void write_pmf_file(const pmf_song &song_, const command_arguments &ca_)
{
  // get song info
  const unsigned num_channels=song_.num_channels;
  const usize_t num_patterns=song_.patterns.size();
  const usize_t num_instruments=song_.instruments.size();
  const usize_t num_samples=song_.samples.size();
  const usize_t playlist_length=song_.playlist.size();

  // check for patterns referred by the playlist
  unsigned num_active_patterns=0;
  array<pmf_pattern_info> pat_infos(num_patterns);
  for(unsigned i=0; i<num_patterns; ++i)
  {
    pmf_pattern_info &pinfo=pat_infos[i];
    pinfo.tracks.resize(num_channels);
  }
  for(unsigned i=0; i<playlist_length; ++i)
  {
    uint8 pat_idx=song_.playlist[i];
    pmf_pattern_info &pinfo=pat_infos[pat_idx];
    pinfo.is_referred=true;
  }

  // reindex patterns
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    pmf_pattern_info &pinfo=pat_infos[pi];
    if(!ca_.enable_optim || pinfo.is_referred)
      pinfo.index=num_active_patterns++;
  }

  // check for instruments referred by active patterns & mark active channels
  array<uint8> active_channels(num_channels, uint8(0));
  array<instrument_info> inst_infos(num_instruments);
  array<sample_info> smp_infos(num_samples);
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    pmf_pattern_info &pinfo=pat_infos[pi];
    if(ca_.enable_optim && !pinfo.is_referred)
      continue;
    pinfo.num_rows=song_.patterns[pi].num_rows;
    PFC_ASSERT(pinfo.num_rows>0 && pinfo.num_rows<=256);
    const pmf_pattern_track_row *row_data=song_.patterns[pi].rows.data();
    const unsigned num_pattern_rows=song_.patterns[pi].num_rows;
    for(unsigned ri=0; ri<num_pattern_rows; ++ri)
      for(unsigned ci=0; ci<num_channels; ++ci)
      {
        if(row_data->note!=0xff)
          active_channels[ci]|=1;
        uint8 inst=row_data->instrument;
        if(inst!=0xff && inst<num_instruments)
        {
          active_channels[ci]|=2;
          inst_infos[inst].is_referred=true;
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
    if(!ca_.enable_optim || (active_channels[i] && active_channel_map.size()<ca_.max_channels))
      active_channel_map.push_back(i);
  }
  const usize_t num_active_channels=active_channel_map.size();

  // set instrument cropped length, re-index the instruments and setup envelopes
  array<pmf_envelope> envelopes;
  array<usize_t> env_offsets;
  usize_t total_envelope_data_size=0;
  unsigned num_active_instruments=0;
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    const pmf_instrument &inst=song_.instruments[ii];
    instrument_info &iinfo=inst_infos[ii];
    if(!ca_.enable_optim || iinfo.is_referred)
    {
      const pmf_sample *smp=inst.sample_idx<song_.samples.size()?&song_.samples[inst.sample_idx]:0;
      if(smp && smp->length)
      {
        // set instrument index and check for envelope
        sample_info &sinfo=smp_infos[inst.sample_idx];
        sinfo.is_referred=true;
        sinfo.cropped_len=smp->loop_len?min<unsigned>(smp->length, smp->loop_start+smp->loop_len):smp->length;
        iinfo.index=num_active_instruments++;
        if(inst.vol_envelope.data.size())
        {
          // check if envelope exists
          const pmf_envelope *env=linear_search(envelopes.begin(), envelopes.size(), inst.vol_envelope);
          if(!env)
          {
            // add new envelope
            env_offsets.push_back(total_envelope_data_size);
            envelopes.push_back(inst.vol_envelope);
            env=envelopes.last();
            total_envelope_data_size+=pmfcfg_offset_env_points+env->data.size()*pmfcfg_envelope_point_size;
          }

          // set instrument envelope data offset
          unsigned env_index=unsigned(env-envelopes.begin());
          iinfo.vol_env_offset=env_offsets[env_index];
        }
      }
      else
        iinfo.is_referred=false;
    }
  }

  // re-index samples and calculate sample data offsets
  unsigned num_active_samples=0;
  unsigned total_sample_data_bytes=0;
  for(unsigned si=0; si<num_samples; ++si)
  {
    sample_info &sinfo=smp_infos[si];
    if(!ca_.enable_optim || sinfo.is_referred)
    {
      sinfo.index=num_active_samples++;
      sinfo.data_offset=total_sample_data_bytes;
      total_sample_data_bytes+=sinfo.cropped_len;
    }
  }

  // split patterns to unique pattern tracks
  deque<pmf_pattern_track> tracks;
  pmf_pattern_track temp_track;
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    // check if pattern is referred by the playlist
    pmf_pattern_info &pinfo=pat_infos[pi];
    if(ca_.enable_optim && !pinfo.is_referred)
      continue;

    const pmf_pattern &pattern=song_.patterns[pi];
    for(unsigned ci=0; ci<num_active_channels; ++ci)
    {
      // build temp track
      temp_track.rows.resize(pattern.num_rows);
      unsigned chl_idx=active_channel_map[ci];
      bool invalid_instrument_state=false;
      for(unsigned ri=0; ri<pattern.num_rows; ++ri)
      {
        // copy track row
        pmf_pattern_track_row &row=temp_track.rows[ri];
        row=pattern.rows[chl_idx+ri*num_channels];

        // validate track row data
        if(row.instrument!=0xff)
          invalid_instrument_state=   row.instrument>=num_instruments
                                   || song_.instruments[row.instrument].sample_idx>=num_samples
                                   || !smp_infos[song_.instruments[row.instrument].sample_idx].cropped_len;
        if(invalid_instrument_state)
        {
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
            PFC_ASSERT(inst_infos[inst].index<(1<<pmfcfg_num_instrument_bits));
            write_bits(track.compressed_data, bit_pos, pmfcfg_num_instrument_bits, uint8(inst_infos[inst].index));
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
  const usize_t pattern_header_bytes=num_active_patterns*(pmfcfg_pattern_metadata_header_size+num_active_channels*pmfcfg_pattern_metadata_track_offset_size);
  float pattern_data_compression=song_.total_src_pattern_data_bytes?100.0f*float(total_compressed_track_bytes+pattern_header_bytes)/float(song_.total_src_pattern_data_bytes):0.0f;

  // write PMF header
  array<uint8> pmf_data;
  container_output_stream<array<uint8> > out_stream(pmf_data);
  out_stream<<uint32(0x78666d70);  // "pmfx"
  out_stream<<uint16(pmf_file_version);
  out_stream<<uint16(song_.flags);
  out_stream<<uint32(0);
  out_stream<<uint8(song_.initial_speed);
  out_stream<<uint8(song_.initial_tempo);
  out_stream<<uint16(song_.note_period_min);
  out_stream<<uint16(song_.note_period_max);
  out_stream<<uint16(playlist_length);
  out_stream<<uint8(num_active_channels);
  out_stream<<uint8(num_active_patterns);
  out_stream<<uint8(num_active_instruments);
  for(unsigned i=0; i<playlist_length; ++i)
    out_stream<<uint8(pat_infos[song_.playlist[i]].index);
  unsigned num_padding_bytes=(1-playlist_length)&3;
  for(unsigned i=0; i<num_padding_bytes; ++i)
    out_stream<<uint8(0);

  // calculate data offsets
  const usize_t envelope_data_base_offset=out_stream.pos()+num_active_instruments*sizeof(pmf_instrument_header)+pattern_header_bytes;
  const usize_t track_data_base_offset=envelope_data_base_offset+total_envelope_data_size;
  const usize_t sample_data_base_offset=track_data_base_offset+total_compressed_track_bytes;

  // write instrument metadata
  unsigned num_active_inst=0;
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    const instrument_info &iinfo=inst_infos[ii];
    if(!ca_.enable_optim || iinfo.is_referred)
    {
      const pmf_instrument &inst=song_.instruments[ii];
      unsigned sample_idx=inst.sample_idx;
      if(sample_idx!=unsigned(-1))
      {
        const pmf_sample &smp=song_.samples[sample_idx];
        const sample_info &sinfo=smp_infos[sample_idx];
        usize_t sample_file_offset=sample_data_base_offset+sinfo.data_offset;
        out_stream<<uint32(sample_file_offset);
        out_stream<<uint32(sinfo.cropped_len);
        out_stream<<uint32(smp.loop_len<sinfo.cropped_len?smp.loop_len:sinfo.cropped_len);
        out_stream<<uint16(iinfo.vol_env_offset!=unsigned(-1)?envelope_data_base_offset+iinfo.vol_env_offset:0);
        out_stream<<uint16(inst.fadeout_speed);
        out_stream<<int16(smp.finetune);
        out_stream<<uint8(smp.flags);
        out_stream<<uint8(smp.volume);
        ++num_active_inst;
      }
    }
  }
  float sample_data_compression=song_.total_src_sample_data_bytes?100.0f*float(total_sample_data_bytes)/float(song_.total_src_sample_data_bytes):0.0f;

  // write pattern metadata
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    const pmf_pattern_info &pinfo=pat_infos[pi];
    if(!ca_.enable_optim || pinfo.is_referred)
    {
      out_stream<<uint8(pinfo.num_rows-1);
      out_stream<<uint8(0);
      for(unsigned ci=0; ci<num_active_channels; ++ci)
      {
        unsigned chl_idx=active_channel_map[ci];
        const pmf_pattern_track &track=tracks[pinfo.tracks[chl_idx]];
        out_stream<<uint16(track.compressed_data.size()?track_data_base_offset+track.offset:0);
      }
    }
  }

  // write envelope data
  usize_t num_envelopes=envelopes.size();
  for(unsigned ei=0; ei<num_envelopes; ++ei)
  {
    const pmf_envelope &env=envelopes[ei];
    out_stream<<uint8(env.data.size());
    out_stream<<uint8(env.sustain_loop_start);
    out_stream<<uint8(env.sustain_loop_end);
    out_stream<<uint8(env.loop_start);
    out_stream<<uint8(env.loop_end);
    out_stream.write(env.data.data(), env.data.size());
  }

  // write track data
  for(unsigned ti=0; ti<num_tracks; ++ti)
  {
    const pmf_pattern_track &track=tracks[ti];
    out_stream.write_bytes(track.compressed_data.data(), track.compressed_data.size());
  }

  // write instrument sample data
  for(unsigned si=0; si<num_samples; ++si)
  {
    const sample_info &sinfo=smp_infos[si];
    if(!ca_.enable_optim || sinfo.is_referred)
      out_stream.write_bytes(song_.samples[si].data.data, smp_infos[si].cropped_len);
  }

  // log PMF info
  out_stream.flush();
  usize_t total_file_size=out_stream.pos();
  logf("Song name: %s%s(%s)\r\n", song_.name.c_str(), song_.name.size()?" ":"", ca_.friendly_input_file.c_str());
  logf("Playlist length: %i\r\n", playlist_length);
  logf("Active channels: %i\r\n", num_active_channels);
  logf("Active patterns: %i\r\n", num_active_patterns);
  logf("Active inst/samp: %i/%i (orig %i/%i)\r\n", num_active_instruments, num_active_instruments, song_.num_valid_instruments, song_.num_valid_samples);
  logf("Unique pattern tracks %i/%i (%3.1f%%)\r\n", num_tracks, num_active_patterns*num_active_channels, track_uniqueness);
  logf("PMF pattern data size: %i bytes (%3.1f%% of orig %i bytes)\r\n", total_compressed_track_bytes+pattern_header_bytes, pattern_data_compression, song_.total_src_pattern_data_bytes);
  logf("PMF sample data size: %i bytes (%3.1f%% of orig %i bytes)\r\n", total_sample_data_bytes, sample_data_compression, song_.total_src_sample_data_bytes);
  logf("Total PMF binary size: %i bytes\r\n", total_file_size);

  // write data as binary or hex codes
  memcpy(pmf_data.data()+8/*PFC_OFFSETOF(pmf_header, file_size)*/, &total_file_size, 4);
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

    // write data as ascii hex codes
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
  return 0;
}
//----------------------------------------------------------------------------

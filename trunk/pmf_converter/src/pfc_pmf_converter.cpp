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

#include "sxp_src/core/main.h"
#include "sxp_src/core/fsys/fsys.h"
#include "sxp_src/core/containers.h"
#include "sxp_src/core/sort.h"
#include "pfc_pmf_converter.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// PMF format confing
//============================================================================
// PMF config
// PMF file structure
enum {pmfcfg_offset_flags=PFC_OFFSETOF(pmf_header, flags)};
enum {pmfcfg_offset_init_speed=PFC_OFFSETOF(pmf_header, initial_speed)};
enum {pmfcfg_offset_init_tempo=PFC_OFFSETOF(pmf_header, initial_tempo)};
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
enum {pmfcfg_offset_inst_volume=PFC_OFFSETOF(pmf_instrument_header, default_volume)};
enum {pmfcfg_offset_inst_length=PFC_OFFSETOF(pmf_instrument_header, length)};
enum {pmfcfg_offset_inst_loop_length=PFC_OFFSETOF(pmf_instrument_header, loop_length)};
enum {pmfcfg_offset_inst_c4hz=PFC_OFFSETOF(pmf_instrument_header, c4hz)};
enum {pmfcfg_offset_inst_vol_env=PFC_OFFSETOF(pmf_instrument_header, vol_env_offset)};
enum {pmfcfg_offset_inst_fadeout_speed=PFC_OFFSETOF(pmf_instrument_header, fadeout_speed)};
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
enum {pmfcfg_num_instrument_bits=5}; // max 32 instruments
enum {pmfcfg_num_volume_bits=6};     // volume range [0, 63]
enum {pmfcfg_num_effect_bits=4};     // effects 0-15
enum {pmfcfg_num_effect_data_bits=8};
//----------------------------------------------------------------------------


//============================================================================
// locals
//============================================================================
static const char *s_copyright_message=
  "PMF Converter v0.3\r\n"
  "Copyright (C) 2013, Profoundic Technologies, Inc. All rights reserved.\r\n"
  "\n";
static const char *s_usage_message="Usage: pfc_mod_converter [options] -i <input.mod/s3m/xm/it> -o <output.pmf>   (-h for help)\r\n";
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
    suppress_copyright=false;
  }
  //----

  heap_str input_file;
  heap_str output_file;
  unsigned max_channels;
  bool output_binary;
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
      unsigned arg_size=str_size(args_[i]);
      switch(to_lower(args_[i][1]))
      {
        // help
        case 'h':
        {
          if(arg_size==2)
          {
            // output help
            logf("%s"
                 "%s"
                 "\r\n"
                 "Options:\r\n"
                 "  -o <file>       Output filename\r\n"
                 "\r\n"
                 "  -hex            Output data as comma separated ASCII hex codes (instead of binary)\r\n"
                 "  -ch <num_chl>   Maximum number of channels (Default: 64)\r\n"
                 "\r\n"
                 "  -h              Print this screen\n"
                 "  -c              Suppress copyright message\r\n", s_copyright_message, s_usage_message);
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

        // suppress copyright test
        case 'c':
        {
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
      }
    }
  }

  // check for help string and copyright message output
  if(!ca_.suppress_copyright)
    log(s_copyright_message);
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
  note=0xff;
  instrument=0xff;
  volume=0xff;
  effect=0xff;
  effect_data=0;
  trailing_empty_rows=0;
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

unsigned pmf_pattern_track_row::num_bits() const
{
  unsigned num_bits=5;
  if(note!=0xff)
    num_bits+=pmfcfg_num_note_bits;
  if(instrument!=0xff)
    num_bits+=pmfcfg_num_instrument_bits;
  if(volume!=0xff)
    num_bits+=pmfcfg_num_volume_bits;
  if(effect!=0xff)
    num_bits+=pmfcfg_num_effect_bits+pmfcfg_num_effect_data_bits;
  return num_bits;
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

  unsigned offset;
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
  unsigned num_pnts=data.size();
  if(   sustain_loop_start!=env_.sustain_loop_start
     || sustain_loop_end!=env_.sustain_loop_end
     || loop_start!=env_.loop_start
     || loop_end!=env_.loop_end
     || num_pnts!=env_.data.size())
    return false;
  for(unsigned i=0; i<num_pnts; ++i)
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
  length=0;
  loop_start=0;
  loop_len=0;
  c4hz=8363;
  volume=0;      // [0, 255]
  fadeout_speed=65535;
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
  total_pattern_data_bytes=0;
  total_instrument_data_bytes=0;
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
  array<unsigned> tracks;
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
    cropped_len=0;
    index=0;
    vol_env_offset=unsigned(-1);
  }
  //--------------------------------------------------------------------------

  bool is_referred;
  unsigned cropped_len;
  unsigned index;
  unsigned vol_env_offset;
};
//----------------------------------------------------------------------------


//============================================================================
// comp_type
//============================================================================
struct comp_type
{
  uint8 type;
  unsigned size;
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
  const unsigned num_patterns=song_.patterns.size();
  const unsigned num_instruments=song_.instruments.size();
  const unsigned playlist_length=song_.playlist.size();
  logf("Playlist length: %i\r\n", playlist_length);

  // check for patterns referred by the playlist
  unsigned num_active_patterns=0;
  array<pmf_pattern_info> pat_infos(num_patterns);
  for(unsigned i=0; i<num_patterns; ++i)
  {
    pmf_pattern_info &pinfo=pat_infos[i];
    pinfo.tracks.resize(song_.num_channels);
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
    if(pinfo.is_referred)
      pinfo.index=num_active_patterns++;
  }
  logf("Number of active patterns: %i\r\n", num_active_patterns);

  // check for instruments referred by active patterns & mark active channels
  array<uint8> active_channels(num_channels, uint8(0));
  array<instrument_info> inst_infos(num_instruments);
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    pmf_pattern_info &pinfo=pat_infos[pi];
    if(!pinfo.is_referred)
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
        ++row_data;
      }
  }

  // set active channel indices (for empty channel removal)
  array<uint8> active_channel_map;
  for(uint8 i=0; i<num_channels; ++i)
  {
    if(active_channels[i]==3 && active_channel_map.size()<ca_.max_channels)
      active_channel_map.push_back(i);
  }
  const unsigned num_active_channels=active_channel_map.size();
  logf("Number of active channels: %i\r\n", num_active_channels);

  // set instrument cropped length, re-index the instruments and setup envelopes
  array<pmf_envelope> envelopes;
  array<unsigned> env_offsets;
  unsigned total_envelope_data_size=0;
  unsigned num_active_instruments=0;
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    const pmf_instrument &inst=song_.instruments[ii];
    instrument_info &iinfo=inst_infos[ii];
    iinfo.cropped_len=inst.loop_len?min<unsigned>(inst.length, inst.loop_start+inst.loop_len):inst.length;
    if(iinfo.is_referred)
    {
      if(iinfo.cropped_len)
      {
        // set instrument index and check for envelope
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
            total_envelope_data_size+=pmfcfg_offset_env_points+envelopes.back().data.size()*pmfcfg_envelope_point_size;
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

  // split patterns to unique pattern tracks
  deque<pmf_pattern_track> tracks;
  pmf_pattern_track temp_track;
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    // check if pattern is referred by the playlist
    pmf_pattern_info &pinfo=pat_infos[pi];
    if(!pinfo.is_referred)
      continue;

    const pmf_pattern &pattern=song_.patterns[pi];
    for(unsigned ci=0; ci<num_active_channels; ++ci)
    {
      // build temp track
      temp_track.rows.resize(pattern.num_rows);
      unsigned chl_idx=active_channel_map[ci];
      for(unsigned ri=0; ri<pattern.num_rows; ++ri)
      {
        // copy track row
        pmf_pattern_track_row &row=temp_track.rows[ri];
        row=pattern.rows[chl_idx+ri*num_channels];

        // validate track row data
        if(row.instrument!=0xff && (row.instrument>=num_instruments || !song_.instruments[row.instrument].length))
        {
          row.note=pmfcfg_note_cut;
          row.instrument=0xff;
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
  unsigned num_tracks=tracks.size();
  float track_uniqueness=100.0f*float(num_tracks)/float(num_active_patterns*num_active_channels);
  logf("Unique pattern tracks %i/%i (%3.1f%%)\r\n", num_tracks, num_active_patterns*num_active_channels, track_uniqueness);

  // compress tracks
  unsigned total_compressed_track_bytes=0;
  for(unsigned ti=0; ti<num_tracks; ++ti)
  {
    // track stats
    unsigned num_empty_rows=0;
    uint8 compression_type=0xff;
    unsigned total_num_dmask4_packed_bits=0;
    unsigned total_num_dmask8_packed_bits=0;
    unsigned total_num_dmasks=0;

    // process track in two passes: pass 0=collect stats, pass 1=compress
    pmf_pattern_track &track=tracks[ti];
    unsigned num_rows=track.rows.size();
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
        const unsigned total_num_dmask4_packed_bits_sparse=total_num_dmask4_packed_bits-num_empty_rows*4+num_rows;
        const unsigned total_num_dmask8_packed_bits_sparse=total_num_dmask8_packed_bits-num_empty_rows*8+num_rows;
        const unsigned total_num_dmask4_packed_bits_dmask_ref=total_num_dmask4_packed_bits-num_rows*(4-2)+total_num_dmasks*4;
        const unsigned total_num_dmask8_packed_bits_dmask_ref=total_num_dmask8_packed_bits-num_rows*(8-2)+total_num_dmasks*8;
        const comp_type ctypes[]={{0x0, total_num_dmask4_packed_bits},
                                  {0x1, total_num_dmask4_packed_bits_sparse},
                                  {0x2, total_num_dmask4_packed_bits_dmask_ref},
                                  {0x4, total_num_dmask8_packed_bits},
                                  {0x5, total_num_dmask8_packed_bits_sparse},
                                  {0x6, total_num_dmask8_packed_bits_dmask_ref},
        };
        compression_type=find_min(ctypes, sizeof(ctypes)/sizeof(*ctypes))->type;

        // write track compression type
        write_bits(track.compressed_data, bit_pos, 3, compression_type);
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
          num_dmask4_packed_bits+=pmfcfg_num_volume_bits;
          if(volume==volume_buf[0])
            data_mask|=0x40;
          else if(volume==volume_buf[1])
            data_mask|=0x44;
          else
          {
            num_dmask8_packed_bits+=pmfcfg_num_volume_bits;
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
            PFC_ASSERT(volume<(1<<pmfcfg_num_volume_bits));
            write_bits(track.compressed_data, bit_pos, pmfcfg_num_volume_bits, volume);
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
  const unsigned pattern_header_bytes=num_active_patterns*(pmfcfg_pattern_metadata_header_size+num_active_channels*pmfcfg_pattern_metadata_track_offset_size);
  float pattern_data_compression=song_.total_pattern_data_bytes?100.0f*float(total_compressed_track_bytes+pattern_header_bytes)/float(song_.total_pattern_data_bytes):0.0f;
  logf("Compressed pattern data size: %i bytes (%3.1f%% of the original %i bytes)\r\n", total_compressed_track_bytes+pattern_header_bytes, pattern_data_compression, song_.total_pattern_data_bytes);

  // write PMF header
  array<uint8> pmf_data;
  container_output_stream<array<uint8> > out_stream(pmf_data);
  out_stream<<uint32(0x78666d70);  // "pmfx"
  out_stream<<uint16(0x1100);      // v1.1
  out_stream<<uint16(song_.flags);
  out_stream<<uint32(0);
  out_stream<<uint8(song_.initial_speed);
  out_stream<<uint8(song_.initial_tempo);
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
  const unsigned envelope_data_base_offset=out_stream.pos()+num_active_instruments*sizeof(pmf_instrument_header)+num_active_patterns*(pmfcfg_pattern_metadata_header_size+pmfcfg_pattern_metadata_track_offset_size*num_active_channels);
  const unsigned track_data_base_offset=envelope_data_base_offset+total_envelope_data_size;
  const unsigned sample_data_base_offset=track_data_base_offset+total_compressed_track_bytes;

  // write instrument metadata
  unsigned total_instrument_data_bytes=0;
  for(unsigned ii=0; ii<num_instruments; ++ii)
  {
    const instrument_info &iinfo=inst_infos[ii];
    if(iinfo.is_referred)
    {
      const pmf_instrument &inst=song_.instruments[ii];
      uint32 sample_file_offset=sample_data_base_offset+total_instrument_data_bytes;
      out_stream<<sample_file_offset;
      out_stream<<uint8(0);
      out_stream<<uint8(inst.volume);
      out_stream<<uint16(iinfo.cropped_len);
      out_stream<<uint16(inst.loop_len<iinfo.cropped_len?inst.loop_len:iinfo.cropped_len);
      out_stream<<uint16(inst.c4hz/4);
      out_stream<<uint16(iinfo.vol_env_offset!=unsigned(-1)?envelope_data_base_offset+iinfo.vol_env_offset:0);
      out_stream<<uint16(inst.fadeout_speed);
      total_instrument_data_bytes+=iinfo.cropped_len;
    }
  }
  float instrument_data_compression=song_.total_instrument_data_bytes?100.0f*float(total_instrument_data_bytes)/float(song_.total_instrument_data_bytes):0.0f;
  logf("Instrument sample data size: %i bytes (%3.1f%% of the original %i bytes)\r\n", total_instrument_data_bytes, instrument_data_compression, song_.total_instrument_data_bytes);

  // write pattern metadata
  for(unsigned pi=0; pi<num_patterns; ++pi)
  {
    const pmf_pattern_info &pinfo=pat_infos[pi];
    if(pinfo.is_referred)
    {
      out_stream<<uint8(pinfo.num_rows-1);
      out_stream<<uint8(0);
      for(unsigned ci=0; ci<num_active_channels; ++ci)
        out_stream<<uint16(track_data_base_offset+tracks[pinfo.tracks[ci]].offset);
    }
  }

  // write envelope data
  unsigned num_envelopes=envelopes.size();
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
  for(unsigned i=0; i<num_instruments; ++i)
  {
    const instrument_info &iinfo=inst_infos[i];
    if(iinfo.is_referred)
      out_stream.write_bytes(song_.instruments[i].data.data, inst_infos[i].cropped_len);
  }

  // write data as binary or hex codes
  out_stream.flush();
  unsigned total_file_size=out_stream.pos();
  logf("Total PMF binary size: %i bytes\r\n", total_file_size);
  memcpy(pmf_data.data()+8/*PFC_OFFSETOF(pmf_header, file_size)*/, &total_file_size, 4);
  owner_ref<bin_output_stream_base> out_file=open_file_write(ca_.output_file.c_str());
  if(ca_.output_binary)
    out_file->write_bytes(pmf_data.data(), pmf_data.size());
  else
  {
    // write data as ascii hex codes
    stack_str32 strbuf;
    unsigned data_left=pmf_data.size();
    const uint8 *bytes=pmf_data.data();
    while(data_left)
    {
      unsigned num_bytes=min<unsigned>(data_left, 256);
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
bool convert_mod(bin_input_stream_base&, pmf_song&);
bool convert_s3m(bin_input_stream_base&, pmf_song&);
bool convert_xm(bin_input_stream_base&, pmf_song&);
bool convert_it(bin_input_stream_base&, pmf_song&);
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
  owner_ptr<bin_input_stream_base> in_file=open_file_read(ca.input_file.c_str(), 0, fopencheck_none);
  if(!in_file.data)
  {
    logf("Unable to open file \"%s\" for reading\n\r", ca.input_file.c_str());
    return -1;
  }
  pmf_song song;
  if(   !convert_mod(*in_file, song)
     && !convert_s3m(*in_file, song)
     && !convert_xm(*in_file, song)
     && !convert_it(*in_file, song))
  {
    logf("Unknown input file format - the file not converted \r\n");
    return -1;
  }

  // write PMF file
  write_pmf_file(song, ca);
  return 0;
}
//----------------------------------------------------------------------------

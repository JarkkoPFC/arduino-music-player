//============================================================================
// PMF Player
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

#ifndef PFC_PMF_DATA_H
#define PFC_PMF_DATA_H
//---------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include <stddef.h>

// new
struct pmf_instrument_header;
#define PFC_OFFSETOF(type__, mvar__) offsetof(type__, mvar__)
//---------------------------------------------------------------------------


//============================================================================
// pmf_sample_header
//============================================================================
struct pmf_sample_header
{
  uint32_t data_offset;
  uint32_t length;
  uint32_t loop_length_and_panning;
  int16_t finetune;
  uint8_t flags;
  uint8_t volume;
};
//----------------------------------------------------------------------------


//============================================================================
// pmf_instrument_header
//============================================================================
struct pmf_instrument_header
{
  uint16_t sample_idx; // sample index or offset to a note map
  uint16_t vol_env_offset;
  uint16_t pitch_env_offset;
  uint16_t fadeout_speed;
  uint8_t volume;
  int8_t panning;
};
//----------------------------------------------------------------------------


//============================================================================
// e_pmf_sample_flags
//============================================================================
enum e_pmf_sample_flags
{
  pmfsmpflag_16bit      = 0x01,
  pmfsmpflag_bidi_loop  = 0x02,
};
//----------------------------------------------------------------------------


//============================================================================
// sample data offsets
//============================================================================
enum {pmfcfg_sample_metadata_size=sizeof(pmf_sample_header)};
enum {pmfcfg_offset_smp_data=PFC_OFFSETOF(pmf_sample_header, data_offset)};
enum {pmfcfg_offset_smp_length=PFC_OFFSETOF(pmf_sample_header, length)};
enum {pmfcfg_offset_smp_loop_length_and_panning=PFC_OFFSETOF(pmf_sample_header, loop_length_and_panning)};
enum {pmfcfg_offset_smp_finetune=PFC_OFFSETOF(pmf_sample_header, finetune)};
enum {pmfcfg_offset_smp_flags=PFC_OFFSETOF(pmf_sample_header, flags)};
enum {pmfcfg_offset_smp_volume=PFC_OFFSETOF(pmf_sample_header, volume)};
//----------------------------------------------------------------------------


//============================================================================
// instrument data offsets
//============================================================================
enum {pmfcfg_instrument_metadata_size=sizeof(pmf_instrument_header)};
enum {pmfcfg_offset_inst_sample_idx=PFC_OFFSETOF(pmf_instrument_header, sample_idx)};
enum {pmfcfg_offset_inst_vol_env=PFC_OFFSETOF(pmf_instrument_header, vol_env_offset)};
enum {pmfcfg_offset_inst_pitch_env=PFC_OFFSETOF(pmf_instrument_header, pitch_env_offset)};
enum {pmfcfg_offset_inst_fadeout_speed=PFC_OFFSETOF(pmf_instrument_header, fadeout_speed)};
enum {pmfcfg_offset_inst_volume=PFC_OFFSETOF(pmf_instrument_header, volume)};
enum {pmfcfg_offset_inst_panning=PFC_OFFSETOF(pmf_instrument_header, panning)};
//----------------------------------------------------------------------------

//============================================================================
#endif

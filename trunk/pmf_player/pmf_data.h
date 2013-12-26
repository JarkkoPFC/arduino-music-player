//============================================================================
// PMF Player v0.3
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
// pmf_instrument_header
//============================================================================
struct pmf_instrument_header
{
  uint32_t data_offset;
  uint8_t flags; // e_pmf_inst_flag
  uint8_t default_volume;
  uint16_t length;
  uint16_t loop_length;
  uint16_t c4hz;
  uint16_t vol_env_offset;
  uint16_t fadeout_speed;
};
//----------------------------------------------------------------------------

enum {pmfcfg_instrument_metadata_size=sizeof(pmf_instrument_header)};
enum {pmfcfg_offset_inst_offset=PFC_OFFSETOF(pmf_instrument_header, data_offset)};
enum {pmfcfg_offset_inst_volume=PFC_OFFSETOF(pmf_instrument_header, default_volume)};
enum {pmfcfg_offset_inst_length=PFC_OFFSETOF(pmf_instrument_header, length)};
enum {pmfcfg_offset_inst_loop_length=PFC_OFFSETOF(pmf_instrument_header, loop_length)};
enum {pmfcfg_offset_inst_c4hz=PFC_OFFSETOF(pmf_instrument_header, c4hz)};
enum {pmfcfg_offset_inst_vol_env=PFC_OFFSETOF(pmf_instrument_header, vol_env_offset)};
enum {pmfcfg_offset_inst_fadeout_speed=PFC_OFFSETOF(pmf_instrument_header, fadeout_speed)};
//----------------------------------------------------------------------------

//============================================================================
#endif

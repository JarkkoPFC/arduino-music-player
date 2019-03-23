//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "streams.h"
#include "utils.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// bin_input_stream_base
//============================================================================
usize_t bin_input_stream_base::update_buffer_nv(void *p_, usize_t num_bytes_, bool exact_)
{
  // copy remaining data in the buffer to pointer and update the buffer
  PFC_ASSERT_MSG(!exact_ || !m_is_last, ("Binary input stream underflow\r\n"));
  m_data-=num_bytes_;
  uint8 *p=reinterpret_cast<uint8*>(p_);
  usize_t left=usize_t(m_end-m_data);
  mem_copy(p_, m_data, left);
  return update_buffer_impl(p+left, num_bytes_-left, exact_)+left;
}
//----

void bin_input_stream_base::rewind_nv()
{
  rewind_impl();
}
//----

void bin_input_stream_base::rewind_nv(usize_t num_bytes_)
{
  PFC_ASSERT_MSG(!m_is_first, ("Trying to seek beyond the beginning of the stream\r\n"));
  rewind_impl(num_bytes_);
}
//----

void bin_input_stream_base::skip_nv()
{
  PFC_ASSERT_MSG(!m_is_last, ("Trying to seek beyond the end of the stream\r\n"));
  skip_impl();
}
//----

void bin_input_stream_base::seek_nv(usize_t abs_pos_)
{
  PFC_ASSERT_MSG(!m_is_last || abs_pos_<m_begin_pos, ("Trying to seek beyond the end of the stream\r\n"));
  seek_impl(abs_pos_);
}
//----------------------------------------------------------------------------


//============================================================================
// bin_output_stream_base
//============================================================================
void bin_output_stream_base::flush_buffer_nv(const void *a_, usize_t num_bytes_)
{
  m_data-=num_bytes_;
  flush_buffer_impl(a_, num_bytes_);
}
//----------------------------------------------------------------------------


//============================================================================
// bit_input_stream
//============================================================================
bit_input_stream::bit_input_stream(bin_input_stream_base &s_, usize_t bit_stream_length_)
  :m_stream(s_)
  ,m_bit_stream_length(bit_stream_length_)
{
  mem_zero(m_cache, sizeof(m_cache));
  m_cache_start_bit_pos=unsigned(-cache_size*8);
  m_cache_bit_pos=cache_size*8;
}
//----

bit_input_stream::bit_input_stream(bin_input_stream_base &s_)
  :m_stream(s_)
  ,m_bit_stream_length(usize_t(-1))
{
  mem_zero(m_cache, sizeof(m_cache));
  m_cache_start_bit_pos=unsigned(-cache_size*8);
  m_cache_bit_pos=cache_size*8;
}
//----------------------------------------------------------------------------

void bit_input_stream::update_cache()
{
  // update cache with new data from data stream
  if(int(m_cache_start_bit_pos)<int(m_bit_stream_length))
  {
    int num_tail_bytes=cache_size*8-m_cache_bit_pos;
    int num_cache_fill_bytes;
    if(num_tail_bytes<0)
    {
      num_tail_bytes=(num_tail_bytes-7)/8;
      m_stream.skip(-num_tail_bytes);
      m_cache_start_bit_pos-=num_tail_bytes*8;
      num_tail_bytes=0;
      num_cache_fill_bytes=cache_size;
    }
    else
    {
      num_tail_bytes=(num_tail_bytes+7)/8;
      memcpy(m_cache, m_cache+(m_cache_bit_pos>>3), num_tail_bytes);
      num_cache_fill_bytes=max(0, (int)min((m_bit_stream_length-bit_pos()+7)/8, usize_t(cache_size))-num_tail_bytes);
    }
    m_stream.read_bytes(m_cache+num_tail_bytes, num_cache_fill_bytes);
    mem_zero(m_cache+num_tail_bytes+num_cache_fill_bytes, cache_size-num_tail_bytes-num_cache_fill_bytes);
    m_cache_bit_pos&=7;
    m_cache_start_bit_pos+=cache_size*8;
  }
}
//----------------------------------------------------------------------------

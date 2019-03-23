//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_STREAMS_H
#define PFC_CORE_STREAMS_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "str.h"
namespace pfc
{

// new
class bin_input_stream_base;
class bin_output_stream_base;
template<class Container> class container_output_stream;
class text_output_stream;
class bit_input_stream;
template<class S> class prop_enum_input_stream;
template<class S> class prop_enum_output_stream;
static const struct ostream_cmd_flush {} oscmd_flush;
//----------------------------------------------------------------------------


//============================================================================
// bin_input_stream_base
//============================================================================
class bin_input_stream_base
{
public:
  // construction
  enum {is_big_endian=PFC_BIG_ENDIAN};
  PFC_INLINE bin_input_stream_base();
  virtual PFC_INLINE ~bin_input_stream_base()=0;
  //--------------------------------------------------------------------------

  // streaming
  template<typename T> PFC_INLINE bin_input_stream_base &operator>>(T&);
  template<typename T> PFC_INLINE void read(T*, usize_t count_);
  PFC_INLINE usize_t read_bytes(void*, usize_t num_bytes_, bool exact_=true);
  PFC_INLINE usize_t read_cstr(char*, usize_t max_bytes_);
  //--------------------------------------------------------------------------

  // accessors and seeking
  PFC_INLINE bool is_eos() const;
  PFC_INLINE usize_t pos() const;
  PFC_INLINE void rewind();
  PFC_INLINE void rewind(usize_t num_bytes_);
  PFC_INLINE void skip(usize_t num_bytes_);
  PFC_INLINE void seek(usize_t abs_pos_);
  //--------------------------------------------------------------------------

protected:
  PFC_INLINE void init();
  const uint8 *m_begin, *m_end, *m_data;
  bool m_is_first, m_is_last;
  usize_t m_begin_pos;
  //--------------------------------------------------------------------------

private:
  void operator=(const bin_input_stream_base&); // not implemented
  template<class T> PFC_INLINE void stream(T&, meta_bool<true> is_pod_stream_);
  template<typename T> PFC_INLINE void stream(T&, meta_bool<false> is_pod_stream_);
  template<class T> PFC_INLINE void stream(T*, usize_t count_, meta_bool<true> is_pod_stream_);
  template<typename T> PFC_INLINE void stream(T*, usize_t count_, meta_bool<false> is_pod_stream_);
  usize_t update_buffer_nv(void*, usize_t num_bytes_, bool exact_);
  void rewind_nv();
  void rewind_nv(usize_t num_bytes_);
  void skip_nv();
  void seek_nv(usize_t abs_pos_);
  virtual usize_t update_buffer_impl(void*, usize_t num_bytes_, bool exact_)=0;
  virtual void rewind_impl()=0;
  virtual void rewind_impl(usize_t num_bytes_)=0;
  virtual void skip_impl()=0;
  virtual void seek_impl(usize_t abs_pos_)=0;
};
//----------------------------------------------------------------------------


//============================================================================
// bin_output_stream_base
//============================================================================
class bin_output_stream_base
{
public:
  // construction
  enum {is_big_endian=PFC_BIG_ENDIAN};
  PFC_INLINE bin_output_stream_base();
  virtual PFC_INLINE ~bin_output_stream_base()=0;
  //--------------------------------------------------------------------------

  // streaming
  template<typename T> PFC_INLINE bin_output_stream_base &operator<<(const T&);
  PFC_INLINE bin_output_stream_base &operator<<(const char*);
  PFC_INLINE bin_output_stream_base &operator<<(char*);
  PFC_INLINE bin_output_stream_base &operator<<(const ostream_cmd_flush&);
  template<typename T> PFC_INLINE void write(const T*, usize_t count_);
  PFC_INLINE void write_bytes(const void*, usize_t num_bytes_);
  PFC_INLINE void flush();
  //--------------------------------------------------------------------------

  // accessors
  PFC_INLINE usize_t pos() const;
  //--------------------------------------------------------------------------

protected:
  PFC_INLINE void init();
  uint8 *m_begin, *m_end, *m_data;
  usize_t m_begin_pos;
  //--------------------------------------------------------------------------

private:
  void operator=(const bin_output_stream_base&); // not implemented
  template<class T> PFC_INLINE void stream(const T&, meta_bool<true> is_pod_stream_);
  template<typename T> PFC_INLINE void stream(const T&, meta_bool<false> is_pod_stream_);
  template<class T> PFC_INLINE void stream(const T*, usize_t count_, meta_bool<true> is_pod_stream_);
  template<typename T> PFC_INLINE void stream(const T*, usize_t count_, meta_bool<false> is_pod_stream_);
  void flush_buffer_nv(const void*, usize_t num_bytes_);
  virtual void flush_buffer_impl(const void*, usize_t num_bytes_)=0;
};
//----------------------------------------------------------------------------


//============================================================================
// container_output_stream
//============================================================================
template<class Container>
class container_output_stream: public bin_output_stream_base
{
public:
  // construction
  PFC_INLINE container_output_stream();
  PFC_INLINE container_output_stream(Container&);
  PFC_INLINE void init(Container&);
  PFC_INLINE ~container_output_stream();
  //--------------------------------------------------------------------------

private:
  virtual void flush_buffer_impl(const void*, usize_t num_bytes_);
  //--------------------------------------------------------------------------

  enum {buffer_size=256};
  Container *m_container;
  uint8 m_buffer[buffer_size];
};
//----------------------------------------------------------------------------


//============================================================================
// text_output_stream
//============================================================================
class text_output_stream
{
public:
  // construction
  enum {is_big_endian=PFC_BIG_ENDIAN};
  PFC_INLINE text_output_stream(bin_output_stream_base&, char separator_=0);
  //--------------------------------------------------------------------------

  // streaming
  template<typename T> PFC_INLINE text_output_stream &operator<<(const T&);
  PFC_INLINE text_output_stream &operator<<(const char*);
  PFC_INLINE text_output_stream &operator<<(char*);
  template<typename T> PFC_INLINE void write(const T*, usize_t count_);
  PFC_INLINE void write_bytes(const void*, usize_t num_bytes_);
  //--------------------------------------------------------------------------

  // output formatting
  PFC_INLINE void set_float_format(const char *format_);
  PFC_INLINE void set_int_format(const char *format_);
  PFC_INLINE void set_unsigned_format(const char *format_);
  //--------------------------------------------------------------------------

  // accessors
  PFC_INLINE usize_t pos() const;
  //--------------------------------------------------------------------------

private:
  void operator=(const text_output_stream&); // not implemented
  template<typename T> PFC_INLINE void stream(T, meta_case<0> is_type_float_);
  template<typename T> PFC_INLINE void stream(T, meta_case<1> is_type_signed_);
  PFC_INLINE void stream(char, meta_case<1> is_type_signed_);
  template<typename T> PFC_INLINE void stream(T, meta_case<2> is_type_unsigned_);
  PFC_INLINE void stream(bool, meta_case<2> is_type_unsigned_);
  template<class T> PFC_INLINE void stream(const T&, meta_case<3> is_type_class_);
  template<typename T> PFC_INLINE void stream(const T&, meta_case<-1> default_);
  //--------------------------------------------------------------------------

  bin_output_stream_base &m_stream;
  const char m_separator;
  stack_str8 m_float_format;
  stack_str8 m_int_format;
  stack_str8 m_unsigned_format;
};
//----------------------------------------------------------------------------


//============================================================================
// bit_input_stream
//============================================================================
class bit_input_stream
{
public:
  // construction
  bit_input_stream(bin_input_stream_base&, usize_t bit_stream_length_);
  bit_input_stream(bin_input_stream_base&);
  //--------------------------------------------------------------------------

  // bit read ops
  PFC_INLINE void read_bits(uint32&, unsigned num_bits_);
  PFC_INLINE void read_bits(int32&, unsigned num_bits_);
  //--------------------------------------------------------------------------

  // accessors and seeking
  PFC_INLINE usize_t length() const;
  PFC_INLINE bool is_eos() const;
  PFC_INLINE usize_t bit_pos() const;
  PFC_INLINE void skip_bits(usize_t num_bits_);
  //--------------------------------------------------------------------------

private:
  bit_input_stream(const bit_input_stream&); // not implemented
  void operator=(const bit_input_stream&); // not implemented
  void update_cache();
  //--------------------------------------------------------------------------

  enum {cache_size=32};
  enum {max_type_size=4};
  bin_input_stream_base &m_stream;
  const usize_t m_bit_stream_length;
  uint8 m_cache[cache_size];
  unsigned m_cache_start_bit_pos;
  unsigned m_cache_bit_pos;
};
//----------------------------------------------------------------------------


//============================================================================
// prop_enum_input_stream
//============================================================================
template<class S>
class prop_enum_input_stream: public prop_enum_interface_base<prop_enum_input_stream<S> >
{
public:
  // construction
  enum {pe_type=penum_input};
  PFC_INLINE prop_enum_input_stream(S&);
  PFC_INLINE S &stream() const;
  //--------------------------------------------------------------------------

  // property enumeration
  template<typename T> PFC_INLINE bool var(T&, unsigned flags_=0, const char *mvar_name_=0);
  template<typename T, class C> PFC_INLINE bool var(T&, unsigned flags_, const char *mvar_name_, C&);
  template<typename T, class C> PFC_INLINE bool var(T&, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*));
  template<typename T, class C> PFC_INLINE bool var(T&, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned var_index_), unsigned var_index_);
  template<typename T> PFC_INLINE bool avar(T*, usize_t size_, unsigned flags_=0, const char *mvar_name_=0);
  template<typename T, class C> PFC_INLINE bool avar(T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&);
  template<typename T, class C> PFC_INLINE bool avar(T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*));
  template<typename T, class C> PFC_INLINE bool avar(T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned index_, unsigned var_index_), unsigned var_index_);
  PFC_INLINE bool data(void*, usize_t num_bytes_);
  PFC_INLINE void skip(usize_t num_bytes_);
  //--------------------------------------------------------------------------

private:
  prop_enum_input_stream(const prop_enum_input_stream&); // not implemented
  void operator=(const prop_enum_input_stream&); // not implemented
  //--------------------------------------------------------------------------

  S &m_stream;
};
//----------------------------------------------------------------------------


//============================================================================
// prop_enum_output_stream
//============================================================================
template<class S>
class prop_enum_output_stream: public prop_enum_interface_base<prop_enum_output_stream<S> >
{
public:
  // construction
  enum {pe_type=penum_output};
  PFC_INLINE prop_enum_output_stream(S&);
  PFC_INLINE S &stream() const;
  //--------------------------------------------------------------------------

  // property enumeration
  template<typename T> PFC_INLINE bool var(const T&, unsigned flag_=0, const char *mvar_name_=0);
  template<typename T, class C> PFC_INLINE bool var(const T&, unsigned flags_, const char *mvar_name_, C&);
  template<typename T, class C> PFC_INLINE bool var(const T&, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*));
  template<typename T, class C> PFC_INLINE bool var(const T&, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned var_index_), unsigned var_index_);
  template<typename T> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flag_=0, const char *mvar_name_=0);
  template<typename T, class C> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&);
  template<typename T, class C> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*));
  template<typename T, class C> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned index_, unsigned var_index_), unsigned var_index_);
  PFC_INLINE bool data(const void*, usize_t num_bytes_);
  //--------------------------------------------------------------------------

private:
  prop_enum_output_stream(const prop_enum_output_stream&); // not implemented
  void operator=(const prop_enum_output_stream&); // not implemented
  //--------------------------------------------------------------------------

  S &m_stream;
};
//----------------------------------------------------------------------------

//============================================================================
#include "streams.inl"
} // namespace pfc
#endif

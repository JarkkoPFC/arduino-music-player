//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================


//============================================================================
// bin_input_stream_base
//============================================================================
bin_input_stream_base::bin_input_stream_base()
{
  init();
}
//----

bin_input_stream_base::~bin_input_stream_base()
{
}
//----------------------------------------------------------------------------

template<typename T>
bin_input_stream_base &bin_input_stream_base::operator>>(T &v_)
{
  stream((remove_volatile<T>::res&)v_, meta_bool<is_type_pod_stream<T>::res>());
  return *this;
}
//----

template<typename T>
void bin_input_stream_base::read(T *p_, usize_t count_)
{
  PFC_ASSERT_PEDANTIC(p_!=0 || !count_);
  stream((remove_volatile<T>::res*)p_, count_, meta_bool<is_type_pod_stream<T>::res>());
}
//----

usize_t bin_input_stream_base::read_bytes(void *p_, usize_t num_bytes_, bool exact_)
{
  // copy data from the buffer to the array
  PFC_ASSERT_PEDANTIC(p_!=0 || !num_bytes_);
  if((m_data+=num_bytes_)>m_end)
    return update_buffer_nv(p_, num_bytes_, exact_);
  mem_copy(p_, m_data-num_bytes_, num_bytes_);
  return num_bytes_;
}
//----

usize_t bin_input_stream_base::read_cstr(char *s_, usize_t max_bytes_)
{
  // read c-string (ascii-z)
  PFC_ASSERT(s_!=0);
  char c;
  char *dst=s_, *end=s_+max_bytes_;
  do
  {
    if(m_data==m_end)
      update_buffer_nv(0, 0, true);
    c=*m_data++;
    *dst++=c;
  } while(c && dst<end);
  *--dst=0;
  return usize_t(dst-s_);
}
//----------------------------------------------------------------------------

bool bin_input_stream_base::is_eos() const
{
  return m_is_last && m_data==m_end;
}
//----

usize_t bin_input_stream_base::pos() const
{
  return usize_t(m_data-m_begin)+m_begin_pos;
}
//----

void bin_input_stream_base::rewind()
{
  // rewind to the beginning
  if(!m_is_first)
    rewind_nv();
  else
    m_data=m_begin;
}
//----

void bin_input_stream_base::rewind(usize_t num_bytes_)
{
  // rewind bytes
  if(m_data<m_begin+num_bytes_)
    rewind_nv(num_bytes_);
  else
    m_data-=num_bytes_;
}
//----

void bin_input_stream_base::skip(usize_t num_bytes_)
{
  // skip bytes
  if((m_data+=num_bytes_)>m_end)
    skip_nv();
}
//----

void bin_input_stream_base::seek(usize_t abs_pos_)
{
  // absolute seek
  if(abs_pos_<m_begin_pos || abs_pos_>usize_t(m_begin_pos+m_end-m_begin))
    seek_nv(abs_pos_);
  else
    m_data=m_begin+abs_pos_-m_begin_pos;
}
//----------------------------------------------------------------------------

void bin_input_stream_base::init()
{
  // initialize members
  m_begin=m_end=m_data=0;
  m_is_first=true;
  m_is_last=true;
  m_begin_pos=0;
}
//----------------------------------------------------------------------------

template<typename T>
void bin_input_stream_base::stream(T &v_, meta_bool<true> is_pod_stream_)
{
  // copy data from the buffer to the value
  if((m_data+=sizeof(T))>m_end)
    update_buffer_nv(&v_, sizeof(T), true);
  else
    mem_copy(&v_, m_data-sizeof(T), sizeof(T));
}
//----

template<class T>
void bin_input_stream_base::stream(T &v_, meta_bool<false> is_pod_stream_)
{
  // read properties of an object from the stream
  prop_enum_input_stream<bin_input_stream_base> pe(*this);
  enum_props(pe, v_);
  post_load_function(&v_);
}
//----

template<typename T>
void bin_input_stream_base::stream(T *p_, usize_t count_, meta_bool<true> is_pod_stream_)
{
  // copy data from the buffer to the array
  usize_t num_bytes=sizeof(T)*count_;
  if((m_data+=num_bytes)>m_end)
    update_buffer_nv(p_, num_bytes, true);
  else
    mem_copy(p_, m_data-num_bytes, num_bytes);
}
//----

template<class T>
void bin_input_stream_base::stream(T *p_, usize_t count_, meta_bool<false> is_pod_stream_)
{
  // read properties of an array of objects from the stream
  prop_enum_input_stream<bin_input_stream_base> pe(*this);
  T *end=p_+count_;
  if(count_)
    do
    {
      enum_props_most_derived(pe, *p_);
      post_load_function(p_);
    } while(++p_!=end);
}
//----------------------------------------------------------------------------


//============================================================================
// bin_output_stream_base
//============================================================================
bin_output_stream_base::bin_output_stream_base()
{
  init();
}
//----

bin_output_stream_base::~bin_output_stream_base()
{
}
//----------------------------------------------------------------------------

template<typename T>
bin_output_stream_base &bin_output_stream_base::operator<<(const T &v_)
{
  stream((remove_volatile<T>::res&)v_, meta_bool<is_type_pod_stream<T>::res>());
  return *this;
}
//----

bin_output_stream_base &bin_output_stream_base::operator<<(const char *str_)
{
  // copy string to the buffer
  PFC_ASSERT(str_!=0);
  usize_t l=str_size(str_);
  if((m_data+=l)>m_end)
    flush_buffer_nv(str_, l);
  else
    mem_copy(m_data-l, str_, l);
  return *this;
}
//----

bin_output_stream_base &bin_output_stream_base::operator<<(char *str_)
{
  // copy string to the buffer
  PFC_ASSERT(str_!=0);
  usize_t l=str_size(str_);
  if((m_data+=l)>m_end)
    flush_buffer_nv(str_, l);
  else
    mem_copy(m_data-l, str_, l);
  return *this;
}
//----

bin_output_stream_base &bin_output_stream_base::operator<<(const ostream_cmd_flush&)
{
  flush_buffer_impl(0, 0);
  return *this;
}
//----

template<typename T>
void bin_output_stream_base::write(const T *p_, usize_t count_)
{
  PFC_ASSERT_PEDANTIC(p_!=0 || !count_);
  stream((remove_volatile<T>::res*)p_, count_, meta_bool<is_type_pod_stream<T>::res>());
}
//----

void bin_output_stream_base::write_bytes(const void *p_, usize_t num_bytes_)
{
  // copy data from the array to the buffer
  PFC_ASSERT_PEDANTIC(p_!=0 || !num_bytes_);
  if((m_data+=num_bytes_)>m_end)
    flush_buffer_nv(p_, num_bytes_);
  else
    mem_copy(m_data-num_bytes_, p_, num_bytes_);
}
//----

void bin_output_stream_base::flush()
{
  flush_buffer_impl(0, 0);
}
//----------------------------------------------------------------------------

usize_t bin_output_stream_base::pos() const
{
  return usize_t(m_data-m_begin)+m_begin_pos;
}
//----------------------------------------------------------------------------

void bin_output_stream_base::init()
{
  // initialize members
  m_begin=m_end=m_data=0;
  m_begin_pos=0;
}
//----------------------------------------------------------------------------

template<class T>
void bin_output_stream_base::stream(const T &v_, meta_bool<true> is_pod_stream_)
{
  // copy data from the value to the buffer
  if((m_data+=sizeof(T))>m_end)
    flush_buffer_nv(&v_, sizeof(T));
  else
    mem_copy(m_data-sizeof(T), &v_, sizeof(T));
}
//----

template<typename T>
void bin_output_stream_base::stream(const T &v_, meta_bool<false> is_pod_stream_)
{
  // write properties of an object to the stream
  prop_enum_output_stream<bin_output_stream_base> pe(*this);
  enum_props(pe, const_cast<T&>(v_));
}
//----

template<class T>
void bin_output_stream_base::stream(const T *p_, usize_t count_, meta_bool<true> is_pod_stream_)
{
  // copy data from the array to the buffer
  usize_t num_bytes=sizeof(T)*count_;
  if((m_data+=num_bytes)>m_end)
    flush_buffer_nv(p_, num_bytes);
  else
    mem_copy(m_data-num_bytes, p_, num_bytes);
}
//----

template<typename T>
void bin_output_stream_base::stream(const T *p_, usize_t count_, meta_bool<false> is_pod_stream_)
{
  // write properties of an array of objects to the stream
  prop_enum_output_stream<bin_output_stream_base> pe(*this);
  const T *end=p_+count_;
  if(count_)
    do
    {
      enum_props_most_derived(pe, const_cast<T&>(*p_));
    } while(++p_!=end);
}
//----------------------------------------------------------------------------


//============================================================================
// container_output_stream
//============================================================================
template<class Container>
container_output_stream<Container>::container_output_stream()
  :m_container(0)
{
}
//----

template<class Container>
container_output_stream<Container>::container_output_stream(Container &container_)
{
  init(container_);
}
//----

template<class Container>
void container_output_stream<Container>::init(Container &container_)
{
  // initialize stream with new container
  m_container=&container_;
  m_begin=m_data=m_buffer;
  m_end=m_buffer+buffer_size;
  m_begin_pos=0;
}
//----

template<class Container>
container_output_stream<Container>::~container_output_stream()
{
  flush_buffer_impl(0, 0);
}
//----------------------------------------------------------------------------

template<class Container>
void container_output_stream<Container>::flush_buffer_impl(const void *p_, usize_t num_bytes_)
{
  // flush content of the data written to the buffer and handle remaining data
  if(m_container)
    m_container->insert_back(usize_t(m_data-m_begin), m_buffer);
  if(num_bytes_>buffer_size)
  {
    // insert data directly to the container
    m_container->insert_back(num_bytes_, (const uint8*)p_);
    m_begin_pos+=usize_t(m_data-m_begin)+num_bytes_;
    m_begin=m_end=m_data=0;
  }
  else
  {
    // write data to the buffer
    mem_copy(m_buffer, p_, num_bytes_);
    m_begin_pos+=usize_t(m_data-m_begin);
    m_begin=m_buffer;
    m_end=m_buffer+buffer_size;
    m_data=m_begin+num_bytes_;
  }
}
//----------------------------------------------------------------------------


//============================================================================
// text_output_stream
//============================================================================
text_output_stream::text_output_stream(bin_output_stream_base &s_, char separator_)
  :m_stream(s_)
  ,m_separator(separator_)
  ,m_float_format("%f")
  ,m_int_format("%i")
  ,m_unsigned_format("%u")
{
}
//----------------------------------------------------------------------------

template<typename T>
text_output_stream &text_output_stream::operator<<(const T &v_)
{
  stream(v_, meta_case<is_type_float<T>::res?0:
                       is_type_signed<T>::res?1:
                       is_type_unsigned<T>::res?2:
                       is_type_class<T>::res?3:
                       -1>());
  return *this;
}
//----

text_output_stream &text_output_stream::operator<<(const char *str_)
{
  m_stream<<str_;
  return *this;
}
//----

text_output_stream &text_output_stream::operator<<(char *str_)
{
  m_stream<<str_;
  return *this;
}
//----

template<typename T>
void text_output_stream::write(const T *p_, usize_t count_)
{
  PFC_ASSERT(p_!=0 || count_);
  while(count_--)
    *this<<(*p_++);
}
//----

void text_output_stream::write_bytes(const void *p_, usize_t num_bytes_)
{
  m_stream.write_bytes(p_, num_bytes_);
}
//----------------------------------------------------------------------------

void text_output_stream::set_float_format(const char *format_)
{
  m_float_format=format_;
}
//----

void text_output_stream::set_int_format(const char *format_)
{
  m_int_format=format_;
}
//----

void text_output_stream::set_unsigned_format(const char *format_)
{
  m_unsigned_format=format_;
}
//----------------------------------------------------------------------------

usize_t text_output_stream::pos() const
{
  return m_stream.pos();
}
//----------------------------------------------------------------------------

template<typename T>
void text_output_stream::stream(T v_, meta_case<0> is_type_float_)
{
  // float formatting
  stack_str64 buf;
  buf.format(m_float_format.c_str(), v_);
  m_stream<<buf.c_str();
  if(m_separator)
    m_stream<<m_separator;
}
//----

template<typename T>
void text_output_stream::stream(T v_, meta_case<1> is_type_signed_)
{
  // signed int formatting
  stack_str64 buf;
  buf.format(m_int_format.c_str(), v_);
  m_stream<<buf.c_str();
  if(m_separator)
    m_stream<<m_separator;
}
//----

void text_output_stream::stream(char v_, meta_case<1> is_type_signed_)
{
  m_stream<<v_;
}
//----

template<typename T>
void text_output_stream::stream(T v_, meta_case<2> is_type_unsigned_)
{
  // unsigned int formatting
  stack_str64 buf;
  buf.format(m_unsigned_format.c_str(), v_, m_separator);
  m_stream<<buf.c_str();
  if(m_separator)
    m_stream<<m_separator;
}
//----

void text_output_stream::stream(bool v_, meta_case<2> is_type_unsigned_)
{
  m_stream<<(v_?"true":"false");
  if(m_separator)
    m_stream<<m_separator;
}
//----

template<class T>
void text_output_stream::stream(const T &v_, meta_case<3> is_type_class_)
{
  prop_enum_output_stream<text_output_stream> pe(*this);
  enum_props(pe, const_cast<T&>(v_));
}
//----

template<typename T>
void text_output_stream::stream(const T &v_, meta_case<-1> default_)
{
  PFC_CTF_ERROR(T, unable_to_serialize_the_type);
}
//----------------------------------------------------------------------------


//============================================================================
// bit_input_stream
//============================================================================
void bit_input_stream::read_bits(uint32 &res_, unsigned num_bits_)
{
  // extract num_bits_ from cache
  PFC_ASSERT_PEDANTIC(num_bits_<=24);
  if(m_cache_bit_pos>=(cache_size-max_type_size)*8)
    update_cache();
  uint32 v;
  memcpy(&v, m_cache+(m_cache_bit_pos>>3), 4);
  res_=(v>>(m_cache_bit_pos&7))&((1<<num_bits_)-1);
  m_cache_bit_pos+=num_bits_;
}
//----

void bit_input_stream::read_bits(int32 &res_, unsigned num_bits_)
{
  // extract num_bits_ from cache
  PFC_ASSERT_PEDANTIC(num_bits_<=24);
  if(m_cache_bit_pos>=(cache_size-max_type_size)*8)
    update_cache();
  int32 v;
  memcpy(&v, m_cache+(m_cache_bit_pos>>3), 4);
  unsigned sh=32-num_bits_;
  res_=int32(v<<(sh-(m_cache_bit_pos&7)))>>sh;
  m_cache_bit_pos+=num_bits_;
}
//----------------------------------------------------------------------------

usize_t bit_input_stream::length() const
{
  return m_bit_stream_length;
}
//----

bool bit_input_stream::is_eos() const
{
  return m_cache_start_bit_pos+m_cache_bit_pos>=m_bit_stream_length;
}
//----

usize_t bit_input_stream::bit_pos() const
{
  usize_t bpos=m_cache_start_bit_pos+m_cache_bit_pos;
  return bpos<m_bit_stream_length?bpos:m_bit_stream_length;
}
//----

void bit_input_stream::skip_bits(usize_t num_bits_)
{
  m_cache_bit_pos+=(unsigned)num_bits_;
}
//----------------------------------------------------------------------------


//============================================================================
// prop_enum_input_stream
//============================================================================
template<class S>
prop_enum_input_stream<S>::prop_enum_input_stream(S &s_)
  :m_stream(s_)
{
}
//----

template<class S>
S &prop_enum_input_stream<S>::stream() const
{
  return m_stream;
}
//----------------------------------------------------------------------------

template<class S>
template<typename T>
bool prop_enum_input_stream<S>::var(T &v_, unsigned flags_, const char*)
{
  m_stream>>v_;
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_input_stream<S>::var(T &v_, unsigned flags_, const char *mvar_name_, C&)
{
  m_stream>>v_;
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_input_stream<S>::var(T &v_, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*))
{
  m_stream>>v_;
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_input_stream<S>::var(T &v_, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned var_index_), unsigned var_index_)
{
  m_stream>>v_;
  return true;
}
//----

template<class S>
template<typename T>
bool prop_enum_input_stream<S>::avar(T *a_, usize_t size_, unsigned flags_, const char*)
{
  m_stream.read(a_, size_);
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_input_stream<S>::avar(T *a_, usize_t size_, unsigned flags_, const char*, C&)
{
  m_stream.read(a_, size_);
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_input_stream<S>::avar(T *a_, usize_t size_, unsigned flags_, const char*, C&, void(*post_mutate_func_)(C*))
{
  m_stream.read(a_, size_);
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_input_stream<S>::avar(T *a_, usize_t size_, unsigned flags_, const char*, C&, void(C::*mutate_func_)(const T&, unsigned index_, unsigned var_index_), unsigned var_index_)
{
  m_stream.read(a_, size_);
  return true;
}
//----

template<class S>
bool prop_enum_input_stream<S>::data(void *data_, usize_t num_bytes_)
{
  m_stream.read_bytes(data_, num_bytes_);
  return true;
}
//----

template<class S>
void prop_enum_input_stream<S>::skip(usize_t num_bytes_)
{
  m_stream.skip(num_bytes_);
}
//----------------------------------------------------------------------------


//============================================================================
// prop_enum_output_stream
//============================================================================
template<class S>
prop_enum_output_stream<S>::prop_enum_output_stream(S &s_)
  :m_stream(s_)
{
}
//----

template<class S>
S &prop_enum_output_stream<S>::stream() const
{
  return m_stream;
}
//----------------------------------------------------------------------------

template<class S>
template<typename T>
bool prop_enum_output_stream<S>::var(const T &v_, unsigned flags_, const char*)
{
  m_stream<<v_;
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_output_stream<S>::var(const T &v_, unsigned flags_, const char *mvar_name_, C&)
{
  m_stream<<v_;
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_output_stream<S>::var(const T &v_, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*))
{
  m_stream<<v_;
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_output_stream<S>::var(const T &v_, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned var_index_), unsigned var_index_)
{
  m_stream<<v_;
  return true;
}
//----

template<class S>
template<typename T>
bool prop_enum_output_stream<S>::avar(const T *a_, usize_t size_, unsigned flags_, const char*)
{
  m_stream.write(a_, size_);
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_output_stream<S>::avar(const T *a_, usize_t size_, unsigned flags_, const char*, C&)
{
  m_stream.write(a_, size_);
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_output_stream<S>::avar(const T *a_, usize_t size_, unsigned flags_, const char*, C&, void(*post_mutate_func_)(C*))
{
  m_stream.write(a_, size_);
  return true;
}
//----

template<class S>
template<typename T, class C>
bool prop_enum_output_stream<S>::avar(const T *a_, usize_t size_, unsigned flags_, const char*, C&, void(C::*mutate_func_)(const T&, unsigned index_, unsigned var_index_), unsigned var_index_)
{
  m_stream.write(a_, size_);
  return true;
}
//----

template<class S>
bool prop_enum_output_stream<S>::data(const void *data_, usize_t num_bytes_)
{
  m_stream.write_bytes(data_, num_bytes_);
  return true;
}
//----------------------------------------------------------------------------

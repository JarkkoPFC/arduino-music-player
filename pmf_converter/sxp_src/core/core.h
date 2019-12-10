//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_H
#define PFC_CORE_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "config.h"
#include <new>
#include <typeinfo>
#include <string.h>
#include PFC_STR(PFC_CAT3(sxp_src/platform/PFC_PLATFORM_SRC_STR/core/PFC_PLATFORM_SRC_STR, _core__, PFC_COMPILER_SRC_STR.h))  // expands to e.g. "sxp_src/platform/win/core/win_core__msvc.h"
namespace pfc
{

// new
// sized types
typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;
typedef float float32;
typedef float ufloat32;
typedef double float64;
typedef double ufloat64;
#if PFC_BIG_ENDIAN==0
struct uint128 {PFC_ALIGN(16) uint64 lo; uint64 hi;};
#else
struct uint128 {PFC_ALIGN(16) uint64 hi; uint64 lo;};
#endif
// ranged types
typedef float ufloat;     // [0, +inf]
typedef float float1;     // [-1, 1]
typedef float ufloat1;    // [0, 1]
typedef double udouble;   // [0, +inf]
typedef double double1;   // [-1, 1]
typedef double udouble1;  // [0, 1]
// aligned types
struct align_type_1 {PFC_ALIGN(1) int8 dummy;};
struct align_type_2 {PFC_ALIGN(2) int16 dummy;};
struct align_type_4 {PFC_ALIGN(4) int32 dummy;};
struct align_type_8 {PFC_ALIGN(8) int64 dummy;};
struct align_type_16 {PFC_ALIGN(16) int64 dummy[2];};
template<unsigned> struct align_type;
// pointer related types
#ifdef PFC_PLATFORM_32BIT
typedef uint32 usize_t;
typedef int32 ssize_t;
#elif defined(PFC_PLATFORM_64BIT)
typedef uint64 usize_t;
typedef int64 ssize_t;
#endif
// logging
void log(const char*);
void logf(const char*, ...);
void log_indention();
void warn(const char*);
void warnf(const char*, ...);
void warn_indention();
void error(const char*);
void errorf(const char*, ...);
void error_indention();
void set_logging_funcs(void(*logf_)(const char*, usize_t), void(*warnf_)(const char*, usize_t), void(*errorf_)(const char*, usize_t));
void default_logging_func(const char*, usize_t);
void indent_log();
void unindent_log();
// aborting
bool preabort();
void set_preabort_func(bool(*preabort_)());
#define PFC_ABORT() {if(pfc::preabort()) PFC_ABORT_FUNC();}
// low-level memory management
struct alloc_site_info;
PFC_INLINE void *mem_alloc(usize_t num_bytes_, const alloc_site_info *site_info_=0);
template<typename T> PFC_INLINE T *mem_alloc(const alloc_site_info *site_info_=0);
PFC_INLINE void mem_free(void*);
PFC_INLINE void mem_copy(void*, const void*, usize_t num_bytes_);
PFC_INLINE void mem_move(void*, const void*, usize_t num_bytes_);
PFC_INLINE void mem_zero(void*, usize_t num_bytes_);
PFC_INLINE void mem_set(void*, unsigned char, usize_t num_bytes_);
PFC_INLINE bool mem_eq(const void*, const void*, usize_t num_bytes_);
PFC_INLINE bool is_mem_zero(void*, usize_t num_bytes_);
PFC_INLINE int mem_diff(const void*, const void*, usize_t num_bytes_);
class memory_allocator_base;
class default_memory_allocator;
// pointer ops
template<typename T> T *ptr(T*);
template<typename T> bool is_valid(T*);
// object construction and destruction
template<typename T> PFC_INLINE T *array_new(usize_t num_items_, const alloc_site_info *site_info_=0);
template<typename T> PFC_INLINE void array_delete(const T*);
template<typename T> PFC_INLINE usize_t array_size(const T*);
template<typename T> PFC_INLINE void *destruct(const T*);
// type info
template<typename T> struct type_id;
template<typename T> struct static_type_id;
template<typename T> struct ptr_id;
template<typename T> struct ref_id;
// ownership "smart" pointers
template<typename T> class owner_ptr;
template<typename T> class owner_ref;
class owner_data;
// casting
template<typename T, typename U> PFC_INLINE T raw_cast(U);
// min/max
template<typename T> PFC_INLINE T min(T, T);
template<typename T> PFC_INLINE T min(T, T, T);
template<typename T> PFC_INLINE T min(T, T, T, T);
template<typename T> PFC_INLINE T max(T, T);
template<typename T> PFC_INLINE T max(T, T, T);
template<typename T> PFC_INLINE T max(T, T, T, T);
// exception handling constructs
template<typename> class eh_data;        // RAII data pointer
template<typename> struct eh_array_dtor; // RAII array destructor
// directory setup and accessors
void init_working_dir();
const char *executable_dir();
const char *executable_name();
const char *executable_filepath();
const char *working_dir();
//----------------------------------------------------------------------------


//============================================================================
// config check
//============================================================================
#ifndef PFC_BIG_ENDIAN
#error PFC_BIG_ENDIAN not defined (define in platform/compiler specific header)
#endif
//----------------------------------------------------------------------------


//============================================================================
// memory management & new/delete
//============================================================================
#define PFC_MEM_ALLOC(bytes__) pfc::mem_alloc(bytes__)
#define PFC_NEW(type__) new(pfc::mem_alloc<type__ >())type__
#define PFC_ARRAY_NEW(type__, num_items__) pfc::array_new<type__ >(num_items__)
#define PFC_STACK_MALLOC(bytes__) PFC_ALLOCA(bytes__)
#define PFC_ALIGNED_STACK_MALLOC(bytes__, alignment__) ((void*)((usize_t(PFC_ALLOCA(bytes__+alignment__))+alignment__)&-alignment__))
#define PFC_MEM_FREE(ptr__) pfc::mem_free(ptr__)
#define PFC_DELETE(ptr__) pfc::mem_free(pfc::destruct(ptr__))
#define PFC_ARRAY_DELETE(ptr__) pfc::array_delete(ptr__)
#define PFC_ARRAY_SIZE(ptr__) pfc::array_size(ptr__)
#define PFC_CARRAY_SIZE(array__) (sizeof(array__)/sizeof(*array__))
#define PFC_FWD_DELETE(ptr__) fwd_delete(ptr__)
#define PFC_PNEW(ptr__) new(ptr__)
//----------------------------------------------------------------------------


//============================================================================
// logging/warnings/errors
//============================================================================
// logging
#ifdef PFC_BUILDOP_LOGS
#define PFC_LOG(msg__) {pfc::log_indention(); pfc::logf msg__;}
#define PFC_INDENT_LOG() {pfc::indent_log();}
#define PFC_UNINDENT_LOG() {pfc::unindent_log();}
#else
#define PFC_LOG(msg__) (void*)0
#define PFC_INDENT_LOG() (void*)0
#define PFC_UNINDENT_LOG() (void*)0
#endif
// warnings
#ifdef PFC_BUILDOP_WARNINGS_FILEPATH
#define PFC_WARN_PREFIX(str__) {pfc::warn_indention(); pfc::warnf("%s(%i) : " str__, __FILE__, __LINE__);}
#else
#define PFC_WARN_PREFIX(str__) {pfc::warn_indention(); pfc::warn(str__);}
#endif
#ifdef PFC_BUILDOP_WARNINGS
#define PFC_WARN(msg__)      {PFC_WARN_PREFIX("warning : "); pfc::warnf msg__;}
#define PFC_WARN_ONCE(msg__) {static bool s_is_first=true; if(s_is_first) {s_is_first=false; PFC_WARN_PREFIX("warning : "); pfc::warnf msg__;}}
#else
#define PFC_WARN(msg__)      (void*)0
#define PFC_WARN_ONCE(msg__) (void*)0
#endif
// errors
#ifdef PFC_BUILDOP_ERRORS_FILEPATH
#define PFC_ERROR_PREFIX(str__) {pfc::error_indention(); pfc::errorf("%s(%i) : " str__, __FILE__, __LINE__);}
#else
#define PFC_ERROR_PREFIX(str__) {pfc::error_indention(); pfc::error(str__);}
#endif
#ifdef PFC_BUILDOP_ERRORS
#define PFC_ERROR(msg__)     {PFC_ERROR_PREFIX("error : "); pfc::errorf msg__; PFC_ABORT();}
#define PFC_ERROR_NOT_IMPL() {PFC_ERROR_PREFIX("error : Functionality not implemented\r\n"); PFC_ABORT();}
#else
#define PFC_ERROR(msg__)     (void*)0
#define PFC_ERROR_NOT_IMPL() (void*)0
#endif
//----------------------------------------------------------------------------


//============================================================================
// asserts/checks
//============================================================================
// compile-time asserts (CTF=function and CTC=class scope asserts)
#define PFC_CTF_ASSERT(e__)             {struct cterror {char compile_time_assert_failed:(e__);};}
#define PFC_CTF_ASSERT_MSG(e__, msg__)  {struct cterror {char msg__:(e__);};}
#define PFC_CTC_ASSERT(e__)             struct PFC_CAT2(ctassert_at_line_, __LINE__) {enum {is_ok=(e__)!=0}; char compile_time_assert_failed:is_ok;}
#define PFC_CTC_ASSERT_MSG(e__, msg__)  struct PFC_CAT2(ctassert_at_line_, __LINE__) {enum {is_ok=(e__)!=0}; char msg__:is_ok;}
#define PFC_CTF_ERROR(type__, msg__)    {struct cterror {char msg__:sizeof(type__)==0;};}
#define PFC_CTC_ERROR(type__, msg__)    static struct PFC_CAT2(ctassert_at_line_, __LINE__) {char msg__:sizeof(type__)==0;} PFC_CAT2(ctassert, __LINE__);
// run-time asserts
#ifdef PFC_BUILDOP_ASSERTS
#define PFC_ASSERT(e__)            {if(!(e__)) {PFC_ERROR_PREFIX("assert failed : "#e__"\r\n"); PFC_ABORT();}}
#define PFC_ASSERT_MSG(e__, msg__) {if(!(e__)) {PFC_ERROR_PREFIX("assert failed : "); pfc::errorf msg__; PFC_ABORT();}}
#define PFC_ASSERT_CALL(e__)       {e__;}
#else
#define PFC_ASSERT(e__)            {}
#define PFC_ASSERT_MSG(e__, msg__) {}
#define PFC_ASSERT_CALL(e__)       {}
#endif
// run-time checks
#ifdef PFC_BUILDOP_CHECKS
#define PFC_CHECK(e__)                   {if(!(e__)) {PFC_ERROR_PREFIX("check failed : "#e__"\r\n"); PFC_ABORT();}}
#define PFC_CHECK_MSG(e__, msg__)        {if(!(e__)) {PFC_ERROR_PREFIX("check failed : "); pfc::errorf msg__; PFC_ABORT();}}
#define PFC_CHECK_CALL(e__)              {e__;}
#define PFC_CHECK_WARN(e__, msg__)       {if(!(e__)) {PFC_WARN_PREFIX("warning : "); pfc::warnf msg__;}}
#define PFC_CHECK_WARN_ONCE(e__, msg__)  {static bool s_is_first=true; if(s_is_first && !(e__)) {s_is_first=false; PFC_WARN_PREFIX("warning : "); pfc::warnf msg__;}}
#define PFC_VERIFY(e__)                  PFC_CHECK(e__)
#define PFC_VERIFY_MSG(e__, msg__)       PFC_CHECK_MSG(e__, msg__)
#define PFC_VERIFY_WARN(e__, msg__)      PFC_CHECK_WARN(e__, msg__)
#define PFC_VERIFY_WARN_ONCE(e__, msg__) PFC_CHECK_WARN_ONCE(e__, msg__)
#else
#define PFC_CHECK(e__)                   {}
#define PFC_CHECK_MSG(e__, msg__)        {}
#define PFC_CHECK_CALL(e__)              {}
#define PFC_CHECK_WARN(e__, msg__)       {}
#define PFC_CHECK_WARN_ONCE(e__, msg__)  {}
#define PFC_VERIFY(e__)                  {(e__);}
#define PFC_VERIFY_MSG(e__, msg__)       {(e__);}
#define PFC_VERIFY_WARN(e__, msg__)      {(e__);}
#define PFC_VERIFY_WARN_ONCE(e__, msg__) {(e__);}
#endif
// pedantic asserts/checks
#ifdef PFC_BUILDOP_PEDANTIC
#define PFC_ASSERT_PEDANTIC(e__)            PFC_ASSERT(e__)
#define PFC_ASSERT_PEDANTIC_MSG(e__, msg__) PFC_ASSERT_MSG(e__, msg__)
#define PFC_ASSERT_PEDANTIC_CALL(e__)       PFC_ASSERT_CALL(e__)
#define PFC_CHECK_PEDANTIC(e__)             PFC_CHECK(e__)
#define PFC_CHECK_PEDANTIC_MSG(e__, msg__)  PFC_CHECK_MSG(e__, msg__)
#define PFC_CHECK_PEDANTIC_CALL(e__)        PFC_CHECK_CALL(e__)
#else
#define PFC_ASSERT_PEDANTIC(e__)            {}
#define PFC_ASSERT_PEDANTIC_MSG(e__, msg__) {}
#define PFC_ASSERT_PEDANTIC_CALL(e__)       {}
#define PFC_CHECK_PEDANTIC(e__)             {}
#define PFC_CHECK_PEDANTIC_MSG(e__, msg__)  {}
#define PFC_CHECK_PEDANTIC_CALL(e__)        {}
#endif
//----------------------------------------------------------------------------


//============================================================================
// misc
//============================================================================
#ifdef PFC_COMPILER_GCC
#define PFC_OFFSETOF(type__, mvar__) __builtin_offsetof(type__, mvar__)
#define PFC_OFFSETOF_MVARPTR(type__, mvarptr__) __builtin_offsetof(type__, mvar__)
#else
#define PFC_OFFSETOF(type__, mvar__) ((usize_t)&(((type__*)0)->mvar__))
#define PFC_OFFSETOF_MVARPTR(type__, mvarptr__) ((usize_t)&(((type__*)0)->*mvarptr__))   /* todo: temp workaround of a gcc bug */
#endif
#define PFC_NOTHROW throw()
//----------------------------------------------------------------------------


//============================================================================
// e_jobtype_id
//============================================================================
enum e_jobtype_id {jobtype_none};
//----------------------------------------------------------------------------


//============================================================================
// e_file_open_check
//============================================================================
enum e_file_open_check
{
  fopencheck_none,
  fopencheck_warn,
  fopencheck_abort,
};
//----------------------------------------------------------------------------


//============================================================================
// e_file_open_write_mode
//============================================================================
enum e_file_open_write_mode
{
  fopenwritemode_clear,
  fopenwritemode_keep,
};
//----------------------------------------------------------------------------


//============================================================================
// memory tracking
//============================================================================
enum {memory_align=16,
      max_memory_stack_depth=256};
//----------------------------------------------------------------------------

struct memory_info
{
  usize_t num_items;
};
//----

enum {memory_info_size=((sizeof(memory_info)+memory_align-1)/memory_align)*memory_align,
      memory_flag_typeless=0x80000000};
//----------------------------------------------------------------------------


//============================================================================
// memory_allocator_base
//============================================================================
class memory_allocator_base
{
public:
  // construction
  PFC_INLINE memory_allocator_base();
  virtual PFC_INLINE ~memory_allocator_base();
  virtual void check_allocator(usize_t num_bytes_, usize_t mem_align_)=0;
  //--------------------------------------------------------------------------

  // memory management
  virtual void *alloc(usize_t num_bytes_, usize_t mem_align_=memory_align)=0;
  virtual void free(void*)=0;
  //--------------------------------------------------------------------------

private:
  memory_allocator_base (const memory_allocator_base&); // not implemented
  void operator=(const memory_allocator_base&); // not implemented
};
//----------------------------------------------------------------------------


//============================================================================
// default_memory_allocator
//============================================================================
class default_memory_allocator: public memory_allocator_base
{
public:
  // construction
  static PFC_INLINE default_memory_allocator& inst();
  virtual void check_allocator(usize_t num_bytes_, usize_t mem_align_);
  //--------------------------------------------------------------------------

  // memory management
  virtual void *alloc(usize_t num_bytes_, usize_t mem_align_=memory_align);
  virtual void free(void*);
  //--------------------------------------------------------------------------

private:
  default_memory_allocator();
  virtual ~default_memory_allocator();
  default_memory_allocator(const default_memory_allocator&); // not implemented
  void operator=(const default_memory_allocator&); // not implemented
};
//----------------------------------------------------------------------------


//============================================================================
// type ID
//============================================================================
template<typename T>
struct type_id
{
  static const unsigned id;
};
//----

template<typename T>
struct ptr_id
{
  static const unsigned id;
  PFC_CTC_ERROR(T, ptr_id_can_not_be_queried_for_non_pointer_types);
};
//----

template<typename T>
struct ref_id
{
  static const unsigned id;
  PFC_CTC_ERROR(T, ref_id_can_not_be_queried_for_non_reference_types);
};
//----

#define PFC_STATIC_TYPEID(type__, id__) template<> struct static_type_id<type__ > {enum {id=id__};};\
                                        template<> struct static_type_id<const type__ > {enum {id=id__};};\
                                        template<> struct static_type_id<volatile type__ > {enum {id=id__};};\
                                        template<> struct static_type_id<const volatile type__ > {enum {id=id__};};\
                                        template<> struct type_id<type__ > {static const unsigned id;};\
                                        template<> struct type_id<const type__ > {static const unsigned id;};\
                                        template<> struct type_id<volatile type__ > {static const unsigned id;};\
                                        template<> struct type_id<const volatile type__ > {static const unsigned id;}
#include "typeid.inc"
#undef PFC_STATIC_TYPEID
//----------------------------------------------------------------------------


//============================================================================
// incomplete type forwarding
//============================================================================
// type, pointer and reference traits
template<typename> struct is_type_fwd_delete;     // rs: classes supporting deletion through pointers to incomplete types
template<typename> struct is_ptr_fwd_delete;      // pointers to objects supporting deletion through pointers to incomplete types
template<typename> struct is_ref_fwd_delete;      // references to objects supporting deletion through pointers to incomplete types
// forwarding registration and functions
template<typename T> void register_fwd_delete();
template<typename T> PFC_INLINE void fwd_delete(T*);
//----------------------------------------------------------------------------


//============================================================================
// introspection
//============================================================================
enum e_penum
{
  penum_none,
  penum_input,     // read data
  penum_output,    // write data
  penum_display,   // display data
  penum_type_info, // collect type info
};
//----

enum e_mvar_flag
{
  mvarflag_mutable     =0x00000001,
  mvarflag_mutable_ptr =0x00000002,
  mvarflag_hidden      =0x00000004,
  mvarflag_array_tail  =0x00000008,
};
//----

template<class PE>
class prop_enum_interface_base
{
public:
  // enumerator type
  enum {pe_type=penum_none};
  //--------------------------------------------------------------------------

  // streaming interface
  template<class T> PFC_INLINE bool subclass(T*); // returns true if sub-class should be processed
  template<class T> PFC_INLINE unsigned set_custom_streaming(T&, unsigned version_); // returns version number
  template<typename T> PFC_INLINE bool var(const T&, unsigned flags_=0, const char *mvar_name_=0); // returns true if property enumeration should continue
  template<typename T, class C> PFC_INLINE bool var(const T&, unsigned flags_, const char *mvar_name_, C&);
  template<typename T, class C> PFC_INLINE bool var(const T&, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*));
  template<typename T, class C> PFC_INLINE bool var(const T&, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned var_index_), unsigned var_index_);
  template<typename T> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_=0, const char *mvar_name_=0);
  template<typename T, class C> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&);
  template<typename T, class C> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(*post_mutate_func_)(C*));
  template<typename T, class C> PFC_INLINE bool avar(const T*, usize_t size_, unsigned flags_, const char *mvar_name_, C&, void(C::*mutate_func_)(const T&, unsigned index_, unsigned var_index_), unsigned var_index_);
  PFC_INLINE bool data(const void*, usize_t num_bytes_);
  PFC_INLINE void skip(usize_t num_bytes_);
  template<typename T> PFC_INLINE bool alias_var(const T&, unsigned flags_, const char *alias_);
  template<typename T> PFC_INLINE bool alias_avar(const T*, usize_t size_, unsigned flags_, const char *alias_);
  //--------------------------------------------------------------------------

  // editor specific exposure interface
  PFC_INLINE void group_begin(const char *category_name_);
  PFC_INLINE void group_end();
  PFC_INLINE void name(const char*);
  PFC_INLINE void desc(const char*);
  PFC_INLINE void color(uint32 rgb_);
  PFC_INLINE void expanded();
  template<typename T> PFC_INLINE void slider(const T &min_, const T &max_, const T &step_);
  //--------------------------------------------------------------------------

protected:
  // construction
  PFC_INLINE prop_enum_interface_base();
  PFC_INLINE ~prop_enum_interface_base();
  //--------------------------------------------------------------------------

private:
  prop_enum_interface_base(const prop_enum_interface_base&); // not implemented
  void operator=(const prop_enum_interface_base&); // not implemented
};
//----

// monomorphic class introspection
template<class PE, class T> void enum_props(PE&, T&) {PFC_CTF_ERROR(T, class_does_not_have_introspection_definition);}
template<class PE, class T> void enum_props_most_derived(PE&, T&) {PFC_CTF_ERROR(T, class_does_not_have_introspection_definition);}
PFC_INLINE void post_load_function(void*)  {}
template<class T> struct has_class_trait {};
// monomorphic class definition
#define PFC_MONO(class__) public:\
                          typedef class__ this_class_t;\
                          typedef void parent_class_t;\
                          friend int has_class_introspection(pfc::has_class_trait<const volatile this_class_t>) {return 0;}\
                          template<class PE> friend PFC_INLINE void enum_props(PE &pe_, this_class_t &v_) {enum_props_most_derived(pe_, v_);}\
                          template<class PE> friend PFC_INLINE void enum_props_most_derived(PE &pe_, this_class_t &v_) {if(pe_.subclass(&v_)) enum_props_this(pe_, v_);}\
                          template<class PE> friend PFC_INLINE void enum_props_this(PE &pe_, this_class_t &v_)
// introspection declaration & definition
#define PFC_INTROSPEC_DECL {v_.enum_props_impl(pe_, v_);} template<class PE> void enum_props_impl(PE&, this_class_t&)
#define PFC_INTROSPEC_INL_DEF(class__) template<class PE> void class__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF1(a0__, class__) template<a0__> template<class PE> void class__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF2(a0__, a1__, c0__, c1__) template<a0__, a1__> template<class PE> void c0__, c1__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF3(a0__, a1__, a2__, c0__, c1__, c2__) template<a0__, a1__, a2__> template<class PE> void c0__, c1__, c2__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF4(a0__, a1__, a2__, a3__, c0__, c1__, c2__, c3__) template<a0__, a1__, a2__, a3__> template<class PE> void c0__, c1__, c2__, c3__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF5(a0__, a1__, a2__, a3__, a4__, c0__, c1__, c2__, c3__, c4__) template<a0__, a1__, a2__, a3__, a4__> template<class PE> void c0__, c1__, c2__, c3__, c4__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF6(a0__, a1__, a2__, a3__, a4__, a5__, c0__, c1__, c2__, c3__, c4__, c5__) template<a0__, a1__, a2__, a3__, a4__, a5__> template<class PE> void c0__, c1__, c2__, c3__, c4__, c5__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF7(a0__, a1__, a2__, a3__, a4__, a5__, a6__, c0__, c1__, c2__, c3__, c4__, c5__, c6__) template<a0__, a1__, a2__, a3__, a4__, a5__, a6__> template<class PE> void c0__, c1__, c2__, c3__, c4__, c5__, c6__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF8(a0__, a1__, a2__, a3__, a4__, a5__, a6__, a7__, c0__, c1__, c2__, c3__, c4__, c5__, c6__, c7__) template<a0__, a1__, a2__, a3__, a4__, a5__, a6__, a7__> template<class PE> void c0__, c1__, c2__, c3__, c4__, c5__, c6__, c7__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF9(a0__, a1__, a2__, a3__, a4__, a5__, a6__, a7__, a8__, c0__, c1__, c2__, c3__, c4__, c5__, c6__, c7__, c8__) template<a0__, a1__, a2__, a3__, a4__, a5__, a6__, a7__, a8__> template<class PE> void c0__, c1__, c2__, c3__, c4__, c5__, c6__, c7__, c8__::enum_props_impl(PE &pe_, this_class_t &v_)
#define PFC_INTROSPEC_INL_TDEF10(a0__, a1__, a2__, a3__, a4__, a5__, a6__, a7__, a8__, a9__, c0__, c1__, c2__, c3__, c4__, c5__, c6__, c7__, c8__, c9__) template<a0__, a1__, a2__, a3__, a4__, a5__, a6__, a7__, a8__, a9__> template<class PE> void c0__, c1__, c2__, c3__, c4__, c5__, c6__, c7__, c8__, c9__::enum_props_impl(PE &pe_, this_class_t &v_)
//----

// member variable introspection
#define PFC_VAR(vname__) if(!pe_.var(v_.vname__, 0, #vname__, v_)) return
#define PFC_VAR2(vn0__, vn1__) if(!pe_.var(v_.vn0__, 0, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, 0, #vn1__, v_)) return
#define PFC_VAR3(vn0__, vn1__, vn2__) if(!pe_.var(v_.vn0__, 0, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, 0, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, 0, #vn2__, v_)) return
#define PFC_VAR4(vn0__, vn1__, vn2__, vn3__) if(!pe_.var(v_.vn0__, 0, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, 0, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, 0, #vn2__, v_)) return; if(!pe_.var(v_.vn3__, 0, #vn3__, v_)) return
#define PFC_VAR5(vn0__, vn1__, vn2__, vn3__, vn4__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR(vn4__)
#define PFC_VAR6(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR2(vn4__, vn5__)
#define PFC_VAR7(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR3(vn4__, vn5__, vn6__)
#define PFC_VAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR4(vn4__, vn5__, vn6__, vn7__)
#define PFC_VAR9(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__) PFC_VAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_VAR(vn8__)
#define PFC_VAR10(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, vn9__) PFC_VAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_VAR2(vn8__, vn9__)
//----

// custom flag member variable introspection
#define PFC_FVAR(vname__, flags__) if(!pe_.var(v_.vname__, flags__, #vname__, v_)) return
#define PFC_FVAR2(vn0__, vn1__, flags__) if(!pe_.var(v_.vn0__, flags__, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, flags__, #vn1__, v_)) return
#define PFC_FVAR3(vn0__, vn1__, vn2__, flags__) if(!pe_.var(v_.vn0__, flags__, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, flags__, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, flags__, #vn2__, v_)) return
#define PFC_FVAR4(vn0__, vn1__, vn2__, vn3__, flags__) if(!pe_.var(v_.vn0__, flags__, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, flags__, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, flags__, #vn2__, v_)) return; if(!pe_.var(v_.vn3__, flags__, #vn3__, v_)) return
#define PFC_FVAR5(vn0__, vn1__, vn2__, vn3__, vn4__, flags__) PFC_FVAR4(vn0__, vn1__, vn2__, vn3__, flags__); PFC_FVAR(vn4__, flags__)
#define PFC_FVAR6(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, flags__) PFC_FVAR4(vn0__, vn1__, vn2__, vn3__, flags__); PFC_FVAR2(vn4__, vn5__, flags__)
#define PFC_FVAR7(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, flags__) PFC_FVAR4(vn0__, vn1__, vn2__, vn3__, flags__); PFC_FVAR3(vn4__, vn5__, vn6__, flags__)
#define PFC_FVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, flags__) PFC_FVAR4(vn0__, vn1__, vn2__, vn3__, flags__); PFC_FVAR4(vn4__, vn5__, vn6__, vn7__, flags__)
#define PFC_FVAR9(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, flags__) PFC_FVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, flags__); PFC_FVAR(vn8__, flags__)
#define PFC_FVAR10(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, vn9__, flags__) PFC_FVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, flags__); PFC_FVAR2(vn8__, vn9__, flags__)
//----

// hidden (in editor) member variable introspection
#define PFC_HVAR(vname__) if(!pe_.var(v_.vname__, pfc::mvarflag_hidden, #vname__, v_)) return
#define PFC_HVAR2(vn0__, vn1__) if(!pe_.var(v_.vn0__, pfc::mvarflag_hidden, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_hidden, #vn1__, v_)) return
#define PFC_HVAR3(vn0__, vn1__, vn2__) if(!pe_.var(v_.vn0__, pfc::mvarflag_hidden, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_hidden, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_hidden, #vn2__, v_)) return
#define PFC_HVAR4(vn0__, vn1__, vn2__, vn3__) if(!pe_.var(v_.vn0__, pfc::mvarflag_hidden, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_hidden, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_hidden, #vn2__, v_)) return; if(!pe_.var(v_.vn3__, pfc::mvarflag_hidden, #vn3__, v_)) return
#define PFC_HVAR5(vn0__, vn1__, vn2__, vn3__, vn4__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR(vn4__)
#define PFC_HVAR6(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR2(vn4__, vn5__)
#define PFC_HVAR7(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR3(vn4__, vn5__, vn6__)
#define PFC_HVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__) PFC_VAR4(vn0__, vn1__, vn2__, vn3__); PFC_VAR4(vn4__, vn5__, vn6__, vn7__)
#define PFC_HVAR9(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__) PFC_VAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_VAR(vn8__)
#define PFC_HVAR10(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, vn9__) PFC_VAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_VAR2(vn8__, vn9__)
//----

// mutable (in editor) member variable introspection
#define PFC_MVAR(vname__) if(!pe_.var(v_.vname__, pfc::mvarflag_mutable, #vname__, v_)) return
#define PFC_MVAR2(vn0__, vn1__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable, #vn1__, v_)) return
#define PFC_MVAR3(vn0__, vn1__, vn2__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_mutable, #vn2__, v_)) return
#define PFC_MVAR4(vn0__, vn1__, vn2__, vn3__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_mutable, #vn2__, v_)) return; if(!pe_.var(v_.vn3__, pfc::mvarflag_mutable, #vn3__, v_)) return
#define PFC_MVAR5(vn0__, vn1__, vn2__, vn3__, vn4__) PFC_MVAR4(vn0__, vn1__, vn2__, vn3__); PFC_MVAR(vn4__)
#define PFC_MVAR6(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__) PFC_MVAR4(vn0__, vn1__, vn2__, vn3__); PFC_MVAR2(vn4__, vn5__)
#define PFC_MVAR7(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__) PFC_MVAR4(vn0__, vn1__, vn2__, vn3__); PFC_MVAR3(vn4__, vn5__, vn6__)
#define PFC_MVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__) PFC_MVAR4(vn0__, vn1__, vn2__, vn3__); PFC_MVAR4(vn4__, vn5__, vn6__, vn7__)
#define PFC_MVAR9(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__) PFC_MVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_MVAR(vn8__)
#define PFC_MVAR10(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, vn9__) PFC_MVAR8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_MVAR2(vn8__, vn9__)
//----

// mutable (in editor) pointer member variable introspection
#define PFC_VARMP(vname__) if(!pe_.var(v_.vname__, pfc::mvarflag_mutable_ptr, #vname__, v_)) return
#define PFC_VARMP2(vn0__, vn1__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable_ptr, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable_ptr, #vn1__, v_)) return
#define PFC_VARMP3(vn0__, vn1__, vn2__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable_ptr, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable_ptr, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_mutable_ptr, #vn2__, v_)) return
#define PFC_VARMP4(vn0__, vn1__, vn2__, vn3__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable_ptr, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable_ptr, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_mutable_ptr, #vn2__, v_)) return; if(!pe_.var(v_.vn3__, pfc::mvarflag_mutable_ptr, #vn3__, v_)) return
#define PFC_VARMP5(vn0__, vn1__, vn2__, vn3__, vn4__) PFC_VARMP4(vn0__, vn1__, vn2__, vn3__); PFC_VARMP(vn4__)
#define PFC_VARMP6(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__) PFC_VARMP4(vn0__, vn1__, vn2__, vn3__); PFC_VARMP2(vn4__, vn5__)
#define PFC_VARMP7(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__) PFC_VARMP4(vn0__, vn1__, vn2__, vn3__); PFC_VARMP3(vn4__, vn5__, vn6__)
#define PFC_VARMP8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__) PFC_VARMP4(vn0__, vn1__, vn2__, vn3__); PFC_VARMP4(vn4__, vn5__, vn6__, vn7__)
#define PFC_VARMP9(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__) PFC_VARMP8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_VARMP(vn8__)
#define PFC_VARMP10(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, vn9__) PFC_VARMP8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_VARMP2(vn8__, vn9__)

// mutable (in editor) pointer member variable pointing to mutable class
#define PFC_MVARMP(vname__) if(!pe_.var(v_.vname__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vname__, v_)) return
#define PFC_MVARMP2(vn0__, vn1__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn1__, v_)) return
#define PFC_MVARMP3(vn0__, vn1__, vn2__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn2__, v_)) return
#define PFC_MVARMP4(vn0__, vn1__, vn2__, vn3__) if(!pe_.var(v_.vn0__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn0__, v_)) return; if(!pe_.var(v_.vn1__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn1__, v_)) return; if(!pe_.var(v_.vn2__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn2__, v_)) return; if(!pe_.var(v_.vn3__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vn3__, v_)) return
#define PFC_MVARMP5(vn0__, vn1__, vn2__, vn3__, vn4__) PFC_MVARMP4(vn0__, vn1__, vn2__, vn3__); PFC_MVARMP(vn4__)
#define PFC_MVARMP6(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__) PFC_MVARMP4(vn0__, vn1__, vn2__, vn3__); PFC_MVARMP2(vn4__, vn5__)
#define PFC_MVARMP7(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__) PFC_MVARMP4(vn0__, vn1__, vn2__, vn3__); PFC_MVARMP3(vn4__, vn5__, vn6__)
#define PFC_MVARMP8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__) PFC_MVARMP4(vn0__, vn1__, vn2__, vn3__); PFC_MVARMP4(vn4__, vn5__, vn6__, vn7__)
#define PFC_MVARMP9(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__) PFC_MVARMP8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_MVARMP(vn8__)
#define PFC_MVARMP10(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__, vn8__, vn9__) PFC_MVARMP8(vn0__, vn1__, vn2__, vn3__, vn4__, vn5__, vn6__, vn7__); PFC_MVARMP2(vn8__, vn9__)
//----

// aliased member variable introspection
#define PFC_ALIAS_VAR(vname__, alias__) if(!pe_.alias_var(v_.vname__, 0, alias__)) return
#define PFC_ALIAS_AVAR(vname__, size__, alias__) if(!pe_.alias_avar(v_.vname__, size__, 0, alias__)) return
//----

// array variables
#define PFC_AVAR(vname__, size__) if(!pe_.avar(v_.vname__, size__, 0, #vname__, v_)) return
#define PFC_FAVAR(vname__, size__, flags__) if(!pe_.avar(v_.vname__, size__, flags__, #vname__, v_)) return
#define PFC_HAVAR(vname__, size__) if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_hidden, #vname__, v_)) return
#define PFC_MAVAR(vname__, size__) if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable, #vname__, v_)) return
#define PFC_AVARMP(vname__, size__) if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable_ptr, #vname__, v_)) return
#define PFC_MAVARMP(vname__, size__) if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vname__, v_)) return
//----

// mutable post-mutate function call variables
#define PFC_MVAR_MCALL(vname__, call__) {struct func_call {static void call(this_class_t *p_) {p_->call__;}}; if(!pe_.var(v_.vname__, pfc::mvarflag_mutable, #vname__, v_, &func_call::call)) return;}
#define PFC_MVARMP_MCALL(vname__, call__) {struct func_call {static void call(this_class_t *p_) {p_->call__;}}; if(!pe_.var(v_.vname__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vname__, v_, &func_call::call)) return;}
#define PFC_MAVAR_MCALL(vname__, size__, call__) {struct func_call {static void call(this_class_t *p_) {p_->call__;}}; if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable, #vname__, v_, &func_call::call)) return;}
#define PFC_MAVARMP_MCALL(vname__, call__) {struct func_call {static void call(this_class_t *p_) {p_->call__;}}; if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vname__, v_, &func_call::call)) return;}
//----

// mutable virtual variables (variable mutation is done through a class member function call)
#define PFC_MVVAR(vname__, func__, var_index__) if(!pe_.var(v_.vname__, pfc::mvarflag_mutable, #vname__, v_, &this_class_t::func__, var_index__)) return
#define PFC_MVVARMP(vname__, func__, var_index__) if(!pe_.var(v_.vname__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vname__, v_, &this_class_t::func__, var_index__)) return
#define PFC_MVAVAR(vname__, size__, func__, var_index__) if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable, #vname__, v_, &this_class_t::func__, var_index__)) return
#define PFC_MVAVARMP(vname__, size__, func__, var_index__) if(!pe_.avar(v_.vname__, size__, pfc::mvarflag_mutable|pfc::mvarflag_mutable_ptr, #vname__, v_, &this_class_t::func__, var_index__)) return
//----

// introspection function calls
#define PFC_CUSTOM_STREAMING(version__) pe_.set_custom_streaming(v_, version__)
#define PFC_POST_LOAD_CALL(call__) friend PFC_INLINE void post_load_function(this_class_t *v_) {post_load_function(static_cast<parent_class_t*>(v_)); v_->call__;}
#define PFC_SAVE_CALL(call__) if(PE::pe_type==pfc::penum_output) v_.call__
#define PFC_VAR_GROUP(group_name__) pe_.group_begin(group_name__)
#define PFC_VAR_GROUP_END() pe_.group_end()
//----

// special variable exposure macros
#ifdef PFC_BUILDOP_EDITOR
#define PFC_VEXP_N(name__) pe_.name(name__)
#define PFC_VEXP_D(desc__) pe_.desc(desc__)
#define PFC_VEXP_C(rgb__) pe_.color(rgb__)
#define PFC_VEXP_SLIDER(min__, max__, step__) pe_.slider(min__, max__, step__);
#define PFC_VEXP_ND(name__, desc__) pe_.name(name__); pe_.desc(desc__)
#define PFC_VEXP_NC(name__, rgb__) pe_.name(name__); pe_.color(rgb__)
#define PFC_VEXP_DC(desc__, rgb__) pe_.desc(desc__); pe_.color(rgb__)
#define PFC_VEXP_NDC(name__, desc__, rgb__) pe_.name(name__); pe_.desc(desc__); pe_.color(rgb__)
#define PFC_VEXP_EXPANDED() pe_.expanded()
#else
#define PFC_VEXP_N(name__) {}
#define PFC_VEXP_D(desc__) {}
#define PFC_VEXP_C(rgb__) {}
#define PFC_VEXP_ND(name__, desc__) {}
#define PFC_VEXP_NC(name__, rgb__) {}
#define PFC_VEXP_DC(desc__, rgb__) {}
#define PFC_VEXP_NDC(name__, desc__, rgb__) {}
#define PFC_VEXP_EXPANDED() {}
#endif
// short-hand versions for single variable macros (name)
#define PFC_VAR_N(vname__, name__) PFC_VAR(vname__); PFC_VEXP_N(name__)
#define PFC_FVAR_N(vname__, flags__, name__) PFC_FVAR(vname__, flags__); PFC_VEXP_N(name__)
#define PFC_VARMP_N(vname__, name__) PFC_VARMP(vname__); PFC_VEXP_N(name__)
#define PFC_MVAR_N(vname__, name__) PFC_MVAR(vname__); PFC_VEXP_N(name__)
#define PFC_MVARMP_N(vname__, name__) PFC_MVARMP(vname__); PFC_VEXP_N(name__)
#define PFC_AVAR_N(vname__, size__, name__) PFC_AVAR(vname__, size__); PFC_VEXP_N(name__)
#define PFC_FAVAR_N(vname__, size__, flags__, name__) PFC_FAVAR(vname__, size__, flags__); PFC_VEXP_N(name__)
#define PFC_MAVAR_N(vname__, size__, name__) PFC_MAVAR(vname__, size__); PFC_VEXP_N(name__)
#define PFC_AVARMP_N(vname__, size__, name__) PFC_AVARMP(vname__, size__); PFC_VEXP_N(name__)
#define PFC_MAVARMP_N(vname__, size__, name__) PFC_MAVARMP(vname__, size__); PFC_VEXP_N(name__)
#define PFC_MVAR_MCALL_N(vname__, call__, name__) PFC_MVAR_MCALL(vname__, call__); PFC_VEXP_N(name__)
#define PFC_MVARMP_MCALL_N(vname__, call__, name__) PFC_MVARMP_MCALL(vname__, call__); PFC_VEXP_N(name__)
#define PFC_MAVAR_MCALL_N(vname__, size__, call__, name__) PFC_MAVAR_MCALL(vname__, size__, call__); PFC_VEXP_N(name__)
#define PFC_MAVARMP_MCALL_N(vname__, size__, call__, name__) PFC_MAVARMP_MCALL(vname__, size__, call__); PFC_VEXP_N(name__)
#define PFC_MVVAR_N(vname__, func__, var_idx__, name__) PFC_MVVAR(vname__, func__, var_idx__); PFC_VEXP_N(name__)
#define PFC_MVVARMP_N(vname__, func__, var_idx__, name__) PFC_MVVARMP(vname__, func__, var_idx__); PFC_VEXP_N(name__)
#define PFC_MVAVAR_N(vname__, size__, func__, var_idx__, name__) PFC_MVAVAR(vname__, size__, func__, var_idx__); PFC_VEXP_N(name__)
#define PFC_MVAVARMP_N(vname__, size__, func__, var_idx__, name__) PFC_MVAVARMP(vname__, size__, func__, var_idx__); PFC_VEXP_N(name__)
// short-hand versions for single variable macros (description)
#define PFC_VAR_D(vname__, desc__) PFC_VAR(vname__); PFC_VEXP_D(desc__)
#define PFC_FVAR_D(vname__, flags__, desc__) PFC_FVAR(vname__, flags__); PFC_VEXP_D(desc__)
#define PFC_VARMP_D(vname__, desc__) PFC_VARMP(vname__); PFC_VEXP_D(desc__)
#define PFC_MVAR_D(vname__, desc__) PFC_MVAR(vname__); PFC_VEXP_D(desc__)
#define PFC_MVARMP_D(vname__, desc__) PFC_MVARMP(vname__); PFC_VEXP_D(desc__)
#define PFC_AVAR_D(vname__, size__, desc__) PFC_AVAR(vname__, size__); PFC_VEXP_D(desc__)
#define PFC_FAVAR_D(vname__, size__, flags__, desc__) PFC_FAVAR(vname__, size__, flags__); PFC_VEXP_D(desc__)
#define PFC_MAVAR_D(vname__, size__, desc__) PFC_MAVAR(vname__, size__); PFC_VEXP_D(desc__)
#define PFC_AVARMP_D(vname__, size__, desc__) PFC_AVARMP(vname__, size__); PFC_VEXP_D(desc__)
#define PFC_MAVARMP_D(vname__, size__, desc__) PFC_MAVARMP(vname__, size__); PFC_VEXP_D(desc__)
#define PFC_MVAR_MCALL_D(vname__, call__, desc__) PFC_MVAR_MCALL(vname__, call__); PFC_VEXP_D(desc__)
#define PFC_MVARMP_MCALL_D(vname__, call__, desc__) PFC_MVARMP_MCALL(vname__, call__); PFC_VEXP_D(desc__)
#define PFC_MAVAR_MCALL_D(vname__, size__, call__, desc__) PFC_MAVAR_MCALL(vname__, size__, call__); PFC_VEXP_D(desc__)
#define PFC_MAVARMP_MCALL_D(vname__, size__, call__, desc__) PFC_MAVARMP_MCALL(vname__, size__, call__); PFC_VEXP_D(desc__)
#define PFC_MVVAR_D(vname__, func__, var_idx__, desc__) PFC_MVVAR(vname__, func__, var_idx__); PFC_VEXP_D(desc__)
#define PFC_MVVARMP_D(vname__, func__, var_idx__, desc__) PFC_MVVARMP(vname__, func__, var_idx__); PFC_VEXP_D(desc__)
#define PFC_MVAVAR_D(vname__, size__, func__, var_idx__, desc__) PFC_MVAVAR(vname__, size__, func__, var_idx__); PFC_VEXP_D(desc__)
#define PFC_MVAVARMP_D(vname__, size__, func__, var_idx__, desc__) PFC_MVAVARMP(vname__, size__, func__, var_idx__); PFC_VEXP_D(desc__)
// short-hand versions for single variable macros (name+description)
#define PFC_VAR_ND(vname__, name__, desc__) PFC_VAR(vname__); PFC_VEXP_ND(name__, desc__)
#define PFC_FVAR_ND(vname__, flags__, name__, desc__) PFC_FVAR(vname__, flags__); PFC_VEXP_ND(name__, desc__)
#define PFC_VARMP_ND(vname__, name__, desc__) PFC_VARMP(vname__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVAR_ND(vname__, name__, desc__) PFC_MVAR(vname__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVARMP_ND(vname__, name__, desc__) PFC_MVARMP(vname__); PFC_VEXP_ND(name__, desc__)
#define PFC_AVAR_ND(vname__, size__, name__, desc__) PFC_AVAR(vname__, size__); PFC_VEXP_ND(name__, desc__)
#define PFC_FAVAR_ND(vname__, size__, flags__, name__, desc__) PFC_FAVAR(vname__, size__, flags__); PFC_VEXP_ND(name__, desc__)
#define PFC_MAVAR_ND(vname__, size__, name__, desc__) PFC_MAVAR(vname__, size__); PFC_VEXP_ND(name__, desc__)
#define PFC_AVARMP_ND(vname__, size__, name__, desc__) PFC_AVARMP(vname__, size__); PFC_VEXP_ND(name__, desc__)
#define PFC_MAVARMP_ND(vname__, size__, name__, desc__) PFC_MAVARMP(vname__, size__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVAR_MCALL_ND(vname__, call__, name__, desc__) PFC_MVAR_MCALL(vname__, call__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVARMP_MCALL_ND(vname__, call__, name__, desc__) PFC_MVARMP_MCALL(vname__, call__); PFC_VEXP_ND(name__, desc__)
#define PFC_MAVAR_MCALL_ND(vname__, size__, call__, name__, desc__) PFC_MAVAR_MCALL(vname__, size__, call__); PFC_VEXP_ND(name__, desc__)
#define PFC_MAVARMP_MCALL_ND(vname__, size__, call__, name__, desc__) PFC_MAVARMP_MCALL(vname__, size__, call__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVVAR_ND(vname__, func__, var_idx__, name__, desc__) PFC_MVVAR(vname__, func__, var_idx__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVVARMP_ND(vname__, func__, var_idx__, name__, desc__) PFC_MVVARMP(vname__, func__, var_idx__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVAVAR_ND(vname__, size__, func__, var_idx__, name__, desc__) PFC_MVAVAR(vname__, size__, func__, var_idx__); PFC_VEXP_ND(name__, desc__)
#define PFC_MVAVARMP_ND(vname__, size__, func__, var_idx__, name__, desc__) PFC_MVAVARMP(vname__, size__, func__, var_idx__); PFC_VEXP_ND(name__, desc__)
//----------------------------------------------------------------------------


//============================================================================
// owner_ptr
//============================================================================
template<typename T>
class owner_ptr
{ PFC_MONO(owner_ptr) {pe_.var(v_.data, mvarflag_mutable|mvarflag_mutable_ptr, 0, v_);/*todo: add "collapse" flag for proper implementation*/}
public:
  // nested types
  typedef T type;
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE owner_ptr();
  PFC_INLINE owner_ptr(T*);
  PFC_INLINE owner_ptr(const owner_ptr&);
  template<typename U> PFC_INLINE owner_ptr(const owner_ptr<U>&);
  template<typename U> PFC_INLINE owner_ptr(const owner_ref<U>&);
  PFC_INLINE void operator=(T*);
  PFC_INLINE void operator=(const owner_ptr&);
  template<typename U> PFC_INLINE void operator=(const owner_ptr<U>&);
  template<typename U> PFC_INLINE void operator=(const owner_ref<U>&);
  PFC_INLINE ~owner_ptr();
  //--------------------------------------------------------------------------
  
  // accessors
  PFC_INLINE friend bool is_valid(const owner_ptr &ptr_)  {return ptr_.data!=0;}
  PFC_INLINE T *operator->() const;
  PFC_INLINE T &operator*() const;
  PFC_INLINE friend T *ptr(const owner_ptr &ptr_)         {return ptr_.data;}
  //--------------------------------------------------------------------------

  mutable T *data;
};
//----------------------------------------------------------------------------


//============================================================================
// owner_ref
//============================================================================
template<typename T>
class owner_ref
{ PFC_MONO(owner_ref) {pe_.var(v_.data, mvarflag_mutable|mvarflag_mutable_ptr, 0, v_);/*todo: add "collapse" flag for proper implementation*/}
public:
  // nested types
  typedef T type;
  //--------------------------------------------------------------------------

  // construction
  PFC_INLINE owner_ref(T*);
  PFC_INLINE owner_ref(const owner_ref&);
  template<typename U> PFC_INLINE owner_ref(const owner_ref<U>&);
  template<typename U> PFC_INLINE owner_ref(const owner_ptr<U>&);
  PFC_INLINE ~owner_ref();
  //--------------------------------------------------------------------------

  // accessors
  PFC_INLINE T *operator->() const;
  PFC_INLINE T &operator*() const;
  PFC_INLINE friend T *ptr(const owner_ref &ref_)  {return ref_.data;}
  //--------------------------------------------------------------------------

  mutable T *data;
  //--------------------------------------------------------------------------

private:
  void operator=(const owner_ref&); // not implemented
};
//----------------------------------------------------------------------------


//============================================================================
// owner_data
//============================================================================
class owner_data
{
public:
  // construction
  PFC_INLINE owner_data();
  PFC_INLINE owner_data(void*);
  PFC_INLINE owner_data(const owner_data&);
  PFC_INLINE void operator=(void*);
  PFC_INLINE void operator=(const owner_data&);
  PFC_INLINE ~owner_data();
  //--------------------------------------------------------------------------

  // accessors
  PFC_INLINE friend bool is_valid(const owner_data &data_)  {return data_.data!=0;}
  PFC_INLINE friend void *ptr(const owner_data &data_)      {return data_.data;}
  //--------------------------------------------------------------------------

  mutable void *data;
};
//----------------------------------------------------------------------------


//============================================================================
// eh_data
//============================================================================
template<typename T>
class eh_data
{
public:
  // construction
  PFC_INLINE eh_data(memory_allocator_base&, usize_t size_, usize_t align_); /* todo: instead of passing alignment, should use meta_alignof<T>::res in implementation, but not done due to meta.h dependency*/
  PFC_INLINE ~eh_data();
  PFC_INLINE void reset();
  //--------------------------------------------------------------------------

  T *data;
  //--------------------------------------------------------------------------

private:
  eh_data(const eh_data&); // not implemented
  void operator=(const eh_data&); // not implemented
  //--------------------------------------------------------------------------

#ifdef PFC_BUILDOP_EXCEPTIONS
  memory_allocator_base &m_allocator;
#endif
};
//----------------------------------------------------------------------------


//============================================================================
// eh_dtor
//============================================================================
#ifdef PFC_BUILDOP_EXCEPTIONS
#define PFC_EDTOR(dtor__, method__) pfc::eh_dtor exception_destructor(dtor__, method__)
#define PFC_EDTOR_RESET() {exception_destructor.reset();}
#else
#define PFC_EDTOR(dtor__, method__) {}
#define PFC_EDTOR_RESET() {}
#endif
//----------------------------------------------------------------------------

class eh_dtor
{
public:
  // construction
  template<typename T> PFC_INLINE eh_dtor(T &dtor_, void(T::*method_)());
  PFC_INLINE ~eh_dtor();
  PFC_INLINE void reset();
  //--------------------------------------------------------------------------

private:
#ifdef PFC_BUILDOP_EXCEPTIONS
  eh_dtor *m_dtor;
  void(eh_dtor::*m_method)();
#endif
};
//----------------------------------------------------------------------------


//============================================================================
// eh_array_dtor
//============================================================================
template<typename T>
struct eh_array_dtor
{
  // destruction
  PFC_INLINE eh_array_dtor();
  PFC_INLINE eh_array_dtor(T *begin_, T *dst_);
  PFC_INLINE ~eh_array_dtor();
  //----

  T *begin, *dst;
};
//----------------------------------------------------------------------------


//============================================================================
// enumerated value string
//============================================================================
#define PFC_ENUM(etype__) PFC_INLINE const char *enum_type_name(etype__) {return #etype__;}\
                          const char *const*enum_strings(etype__);\
                          const char *const*enum_display_strings(etype__);\
                          const etype__ *enum_values(etype__);\
                          unsigned enum_string_index(etype__);\
                          bool enum_dep_value(etype__&, const char *enum_str_)
#define PFC_CLASS_ENUM(etype__) PFC_INLINE friend const char *enum_type_name(etype__) {return #etype__;}\
                                friend const char *const*enum_strings(etype__);\
                                friend const char *const*enum_display_strings(etype__);\
                                friend const etype__ *enum_values(etype__);\
                                friend unsigned enum_string_index(etype__);\
                                friend bool enum_dep_value(etype__&, const char *enum_str_)
template<typename T> PFC_INLINE const char *enum_string(T);
template<typename T> PFC_INLINE bool enum_value(T&, const char*);
//----

template<typename T> PFC_INLINE const char *enum_type_name(T)             {PFC_CTF_ERROR(T, use_of_undefined_enum_type); return 0;} // if you get the error here, the enum type isn't defined with PFC_ENUM() macro
template<typename T> PFC_INLINE const char *const*enum_strings(T)         {PFC_CTF_ERROR(T, use_of_undefined_enum_type); return 0;}
template<typename T> PFC_INLINE const char *const*enum_display_strings(T) {PFC_CTF_ERROR(T, use_of_undefined_enum_type); return 0;}
template<typename T> PFC_INLINE const T *enum_values(T)                   {PFC_CTF_ERROR(T, use_of_undefined_enum_type); return 0;}
template<typename T> PFC_INLINE unsigned enum_string_index(T)             {PFC_CTF_ERROR(T, use_of_undefined_enum_type); return 0;}
template<typename T> PFC_INLINE bool enum_dep_value(T&, const char*)      {PFC_CTF_ERROR(T, use_of_undefined_enum_type); return 0;}
//----------------------------------------------------------------------------


//============================================================================
// timing and sleeping
//============================================================================
PFC_INLINE uint64 get_thread_cycles();
udouble get_global_time();
PFC_INLINE void thread_nap();
PFC_INLINE void thread_sleep(ufloat time_);
//----------------------------------------------------------------------------

//============================================================================
#include "core.inl"
} // namespace pfc
#endif

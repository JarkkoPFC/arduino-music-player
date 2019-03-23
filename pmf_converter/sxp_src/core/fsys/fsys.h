//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_FSYS_H
#define PFC_CORE_FSYS_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "sxp_src/core/streams.h"
#include "sxp_src/core/utils.h"
namespace pfc
{

// new
struct file_time;
class directory_monitor_base;
class file_system_base;
owner_ref<file_system_base> create_default_file_system(bool set_active_=false);
bool is_absolute_filepath(const char *filepath_);
const char *get_filename(const char *filepath_); // returns pointer to the filename (e.g. "foo/bar/zap.ext" => "zap.ext")
char *get_filename(char *filepath_);
const char *get_fileext(const char *filename_); // returns pointer to the file extension (e.g. "foo/bar/zap.ext" => "ext")
char *get_fileext(char *filename_);
unsigned common_path_size(const char *path0_, const char *path1_, bool case_sensitive_=false); // length of the common path between two paths
char *split_filepath(char *filepath_); // returns pointer to the filename and replace the last path separator with 0 (e.g. "foo/bar/zap.ext" => replace input with "foo/bar" and return "zap.ext")
char *split_filename(char *filename_); // returns pointer to the file extension and replace the extension separator with 0 (e.g. "foo.ext" => replace input with "foo" and return "ext")
void convert_to_compatible_filepath(char *filepath_, char replacement_='~'); // replaces invalid filepath characters in the filepath with given character
unsigned collapse_relative_dirs(char *filepath_); // collapses relative directories in the filepath (e.g. "foo/bar/../zap" => "foo/zap")
void build_temp_filepath(filepath_str &res_, const filepath_str &input_);
bool read_file(file_system_base&, heap_str&, const char *filename_, const char *path_=0, e_file_open_check=fopencheck_warn);
// active file system free-functions
owner_ptr<bin_input_stream_base> afs_open_read(const char *filename_, const char *path_=0, e_file_open_check=fopencheck_warn);
owner_ptr<bin_output_stream_base> afs_open_write(const char *filename_, const char *path_=0, e_file_open_write_mode=fopenwritemode_clear, uint64 fpos_=uint64(-1), bool makedir_=true, e_file_open_check=fopencheck_warn);
filepath_str afs_complete_path(const char *filename_, const char *path_=0, bool collapse_relative_dirs_=true); // return full filepath constructed from active file system current directory, given optional path and name
filepath_str afs_complete_system_path(const char *filename_, const char *path_=0, bool collapse_relative_dirs_=true); // return full system filepath (working directory + complete_path)
// file time operators
PFC_INLINE bool operator==(const file_time&, const file_time&);
PFC_INLINE bool operator!=(const file_time&, const file_time&);
PFC_INLINE bool operator<(const file_time&, const file_time&);
PFC_INLINE bool operator>(const file_time&, const file_time&);
PFC_INLINE bool operator<=(const file_time&, const file_time&);
PFC_INLINE bool operator>=(const file_time&, const file_time&);
//----------------------------------------------------------------------------


//============================================================================
// e_fsys_find
//============================================================================
enum e_fsys_find
{
  fsysfind_all,
  fsysfind_dirs,
  fsysfind_files,
};
//----------------------------------------------------------------------------


//============================================================================
// e_file_notify
//============================================================================
enum e_file_notify
{
  filenotify_write,
};
//----------------------------------------------------------------------------


//============================================================================
// file_time
//============================================================================
struct file_time
{ PFC_MONO(file_time) {PFC_VAR6(year, month, day, hour, minute, second);}
  // construction
  PFC_INLINE file_time();
  PFC_INLINE file_time(uint16 year_, uint8 month_, uint8 day_, uint8 hour_, uint8 minute_, uint8 second_);
  //--------------------------------------------------------------------------

  uint16 year;
  uint8 month;
  uint8 day;
  uint8 hour;
  uint8 minute;
  uint8 second;
};
//----------------------------------------------------------------------------


//============================================================================
// directory_monitor_base
//============================================================================
class directory_monitor_base
{
public:
  // construction
  directory_monitor_base();
  virtual ~directory_monitor_base()=0;
  //--------------------------------------------------------------------------

  // update
  virtual void update(const functor<void(const char *filename_, e_file_notify)>&)=0;
  //--------------------------------------------------------------------------

private:
  directory_monitor_base(const directory_monitor_base&); // not implemented
  void operator=(const directory_monitor_base&); // not implemented
};
//----------------------------------------------------------------------------


//============================================================================
// file_system_base
//============================================================================
class file_system_base
{
public:
  // nested types
  class iterator;
  class iterator_impl_base;
  //--------------------------------------------------------------------------

  // construction
  file_system_base(bool set_active_=false);
  virtual ~file_system_base()=0;
  static PFC_INLINE void set_active(file_system_base*);
  static PFC_INLINE file_system_base &active();
  //--------------------------------------------------------------------------

  // generic operations
  virtual iterator find_first(e_fsys_find, const char *dirname_, const char *path_=0) const=0;
  //--------------------------------------------------------------------------

  // file operations
  virtual bool exists(const char *filename_, const char *path_=0) const=0;
  virtual usize_t file_size(const char *filename_, const char *path_=0) const=0;
  virtual bool is_writable(const char *filename_, const char *path_=0) const=0;
  virtual file_time mod_time(const char *filename_, const char *path_=0) const=0;
  virtual owner_ptr<bin_input_stream_base> open_read(const char *filename_, const char *path_=0, e_file_open_check=fopencheck_warn) const=0;
  virtual owner_ptr<bin_output_stream_base> open_write(const char *filename_, const char *path_=0, e_file_open_write_mode=fopenwritemode_clear, uint64 fpos_=uint64(-1), bool makedir_=true, e_file_open_check=fopencheck_warn)=0;
  virtual bool delete_file(const char *filename_, const char *path_=0)=0;
  virtual bool rename_file(const char *filename_, const char *new_filename_, const char *path_=0, bool overwrite_existing_=false)=0;
  virtual void touch_file(const char *filename_, const char *path_=0)=0;
  //--------------------------------------------------------------------------

  // directory operations and accessors
  virtual void set_directory(const char *dirname_);
  virtual bool make_directory(const char *dirname_, const char *path_=0)=0;
  virtual bool delete_directory(const char *dirname_, const char *path_=0, bool delete_content_=false)=0;
  virtual bool rename_directory(const char *dirname_, const char *new_dirname_, const char *path_=0)=0;
  virtual owner_ptr<directory_monitor_base> create_directory_monitor(const char *dirname_, bool notify_subdirs_=false) const=0;
  PFC_INLINE const stack_str256 &directory() const;
  filepath_str complete_path(const char *filename_, const char *path_=0, bool collapse_relative_dirs_=true) const;
  filepath_str complete_system_path(const char *filename_, const char *path_=0, bool collapse_relative_dirs_=true) const;
  //--------------------------------------------------------------------------

protected:
  void operator=(const file_system_base&); // not implemented
  void set_system_root_directory(const char *root_dir_);
  //--------------------------------------------------------------------------

  static file_system_base *s_active;
  stack_str256 m_system_root_dir;
  stack_str256 m_directory;
};
//----------------------------------------------------------------------------

//============================================================================
// file_system_base::iterator
//============================================================================
class file_system_base::iterator
{
public:
  // construction
  PFC_INLINE iterator();
  //--------------------------------------------------------------------------

  // file iteration
  PFC_INLINE friend bool is_valid(const iterator&);
  PFC_INLINE const char *name() const;
  PFC_INLINE bool is_dir() const;
  PFC_INLINE bool is_file() const;
  PFC_INLINE void operator++();
  //--------------------------------------------------------------------------

private:
  friend class iterator_impl_base;
  PFC_INLINE void reset();
  //--------------------------------------------------------------------------

  owner_ptr<iterator_impl_base> m_iterator;
  const char *m_name;
  bool m_is_dir;
  bool m_is_file;
};
//----------------------------------------------------------------------------

//============================================================================
// file_system_base::iterator_impl_base
//============================================================================
class file_system_base::iterator_impl_base
{
public:
  // construction
  PFC_INLINE iterator_impl_base();
  virtual PFC_INLINE ~iterator_impl_base();
  //--------------------------------------------------------------------------

  // iteration
  virtual bool advance(iterator&)=0;
  //--------------------------------------------------------------------------

protected:
  iterator_impl_base(const iterator_impl_base&); // not implemented
  void operator=(const iterator_impl_base&); // not implemented
  PFC_INLINE void init_iterator(iterator&);
  PFC_INLINE void update(iterator&, const char *name_, bool is_dir_, bool is_file_);
};
//----------------------------------------------------------------------------

//============================================================================
#include "fsys.inl"
} // namespace pfc
#endif

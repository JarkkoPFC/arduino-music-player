//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#include "win_fsys.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// helper functions
//============================================================================
namespace
{
  bool move_file(const char *src_filename_, const char *dst_filename_, unsigned flags_)
  {
#ifdef PFC_USE_MOVEFILEEXW
    wchar_t src_path[MAX_PATH];
    wchar_t dst_path[MAX_PATH];
    cstr_to_wcstr(src_path, src_filename_, sizeof(src_path));
    cstr_to_wcstr(dst_path, dst_filename_, sizeof(dst_path));
    return MoveFileExW(src_path, dst_path, flags_)==TRUE;
#else
    return MoveFileExA(src_filename_, dst_filename_, flags_)==TRUE;
#endif
  }
} // namespace <anonymous>
//----------------------------------------------------------------------------


//============================================================================
// create_default_file_system
//============================================================================
owner_ref<file_system_base> pfc::create_default_file_system(bool set_active_)
{
  return PFC_NEW(win_file_system)(set_active_);
}
//----------------------------------------------------------------------------


//============================================================================
// win_file_system::input_stream
//============================================================================
class win_file_system::input_stream: public bin_input_stream_base
{
public:
  // construction
  input_stream(HANDLE, const char *filename_);
  virtual ~input_stream();
  //--------------------------------------------------------------------------

private:
  virtual usize_t update_buffer_impl(void*, usize_t num_bytes_, bool exact_);
  virtual void rewind_impl();
  virtual void rewind_impl(usize_t num_bytes_);
  virtual void skip_impl();
  virtual void seek_impl(usize_t abs_pos_);
  //--------------------------------------------------------------------------

  enum {buffer_size=4096};
  const HANDLE m_handle;
  usize_t m_size;
  usize_t m_fptr_pos;
  uint8 m_buffer[buffer_size];
};
//----------------------------------------------------------------------------

win_file_system::input_stream::input_stream(HANDLE h_, const char *filename_)
  :m_handle(h_)
{
  // setup file
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  PFC_VERIFY_MSG(GetFileAttributesEx(filename_, GetFileExInfoStandard, &attribs), ("Failed to get size of file \"%s\"\r\n", filename_));
  m_size=attribs.nFileSizeLow;
  m_fptr_pos=0;
  m_is_last=m_size==0;
}
//----

win_file_system::input_stream::~input_stream()
{
  CloseHandle(m_handle);
}
//----------------------------------------------------------------------------

usize_t win_file_system::input_stream::update_buffer_impl(void *p_, usize_t num_bytes_, bool exact_)
{
  // read the rest of the data
  if(num_bytes_>=buffer_size)
  {
    // read data directly to the pointer
    DWORD num_read=0;
    PFC_VERIFY_MSG(ReadFile(m_handle, p_, (unsigned)num_bytes_, &num_read, 0), ("Reading the file failed\r\n"));
    PFC_CHECK_MSG(!exact_ || num_read==num_bytes_, ("Trying to read beyond the end of the file\r\n"));
    m_fptr_pos+=num_read;
    m_is_first=false;
    m_is_last=m_fptr_pos==m_size;
    m_begin_pos+=usize_t(m_end-m_begin)+num_read;
    m_begin=m_end=m_data=0;
    return num_read;
  }

  // read data to buffer and copy it to the pointer
  DWORD num_read=0;
  PFC_VERIFY_MSG(ReadFile(m_handle, m_buffer, buffer_size, &num_read, 0), ("Reading the file failed\r\n"));
  PFC_CHECK_MSG(!exact_ || num_read>=num_bytes_, ("Trying to read beyond the end of the file\r\n"));
  usize_t num_copied=min(usize_t(num_read), num_bytes_);
  mem_copy(p_, m_buffer, num_copied);
  m_fptr_pos+=num_read;
  m_is_first=m_is_first&&m_begin!=0;
  m_is_last=m_size==m_fptr_pos;
  m_begin_pos+=usize_t(m_end-m_begin);
  m_begin=m_buffer;
  m_end=m_buffer+num_read;
  m_data=m_begin+num_copied;
  return num_copied;
}
//----

void win_file_system::input_stream::rewind_impl()
{
  // seek to the beginning of the file
  PFC_ASSERT(m_is_first==false);
  LARGE_INTEGER pos={0, 0};
  PFC_VERIFY_MSG(SetFilePointerEx(m_handle, pos, 0, FILE_BEGIN),
                 ("Failed to set the file pointer to the beginning of the file\r\n"));
  m_fptr_pos=0;
  m_is_first=true;
  m_is_last=false;
  m_begin_pos=0;
  m_begin=m_end=m_data=0;
}
//----

void win_file_system::input_stream::rewind_impl(usize_t num_bytes_)
{
  // rewind file stream
  PFC_ASSERT(num_bytes_);
  usize_t rewind=num_bytes_+usize_t(m_end-m_data);
  LARGE_INTEGER pos={DWORD(-long(rewind)), -1};
  PFC_VERIFY_MSG(SetFilePointerEx(m_handle, pos, 0, FILE_CURRENT)!=0xffffffff,
                 ("Trying to seek beyond beginning of the file\r\n"));
  m_fptr_pos-=rewind;
  m_begin_pos-=num_bytes_-usize_t(m_data-m_begin);
  m_is_first=m_begin_pos==0;
  m_is_last=false;
  m_begin=m_end=m_data=0;
}
//----

void win_file_system::input_stream::skip_impl()
{
  // advance file stream
  usize_t skip=usize_t(m_data-m_end);
  m_fptr_pos+=skip;
  PFC_CHECK_MSG(m_fptr_pos<=m_size, ("Trying to seek beyond the end of the file\r\n"));
  LARGE_INTEGER pos={(unsigned)skip, 0};
  SetFilePointerEx(m_handle, pos, 0, FILE_CURRENT);
  m_is_first=false;
  m_is_last=m_fptr_pos==m_size;
  m_begin_pos+=usize_t(m_data-m_begin);
  m_begin=m_end=m_data=0;
}
//----

void win_file_system::input_stream::seek_impl(usize_t abs_pos_)
{
  // set file pointer position
  PFC_CHECK_MSG(m_fptr_pos<=m_size, ("Trying to seek beyond the end of the file\r\n"));
  LARGE_INTEGER pos={(unsigned)abs_pos_, 0};
  SetFilePointerEx(m_handle, pos, 0, FILE_BEGIN);
  m_fptr_pos=abs_pos_;
  m_is_first=abs_pos_==0;
  m_is_last=m_fptr_pos==m_size;
  m_begin_pos=abs_pos_;
  m_begin=m_end=m_data=0;
}
//--------------------------------------------------------------------------


//==========================================================================
// win_file_system::output_stream
//==========================================================================
class win_file_system::output_stream: public bin_output_stream_base
{
public:
  // construction
  output_stream(HANDLE, bool use_temp_, const filepath_str &filepath_);
  virtual ~output_stream();
  //------------------------------------------------------------------------

private:
  virtual void flush_buffer_impl(const void*, usize_t num_bytes_);
  //------------------------------------------------------------------------

  enum {buffer_size=4096};
  const HANDLE m_handle;
  const bool m_use_temp;
  const filepath_str m_filepath;
  uint8 m_buffer[buffer_size];
};
//--------------------------------------------------------------------------

win_file_system::output_stream::output_stream(HANDLE h_, bool use_temp_, const filepath_str &filepath_)
  :m_handle(h_)
  ,m_use_temp(use_temp_)
  ,m_filepath(filepath_)
{
  // get file pointer position
  LARGE_INTEGER pos={0, 0};
  SetFilePointerEx(h_, pos, &pos, FILE_CURRENT);
  m_begin_pos=usize_t(pos.QuadPart);
}
//----

win_file_system::output_stream::~output_stream()
{
  flush_buffer_impl(0, 0);
  CloseHandle(m_handle);
  if(m_use_temp)
  {
    // construct temp filename and rename file to target filename
    stack_str1024 fpath_temp;
    build_temp_filepath(fpath_temp, m_filepath);
    PFC_VERIFY_MSG(move_file(fpath_temp.c_str(), m_filepath.c_str(), MOVEFILE_REPLACE_EXISTING),
                   ("Unable to rename file \"%s\" to \"%s\"\r\n", fpath_temp.c_str(), m_filepath.c_str()));
  }
}
//----------------------------------------------------------------------------

void win_file_system::output_stream::flush_buffer_impl(const void *p_, usize_t num_bytes_)
{
  // flush content of the data written to the buffer and handle remaining data
  DWORD num_write=0;
  PFC_VERIFY_MSG(WriteFile(m_handle, m_buffer, (unsigned)usize_t(m_data-m_begin), &num_write, 0), ("Writing to the file failed\r\n"));
  PFC_VERIFY_MSG(num_write==usize_t(m_data-m_begin), ("Failed to write all requested data to the file\r\n"));
  if(num_bytes_>=buffer_size)
  {
    // write data directly to the file
    PFC_VERIFY_MSG(WriteFile(m_handle, p_, (unsigned)num_bytes_, &num_write, 0), ("Writing to the file failed\r\n"));
    PFC_VERIFY_MSG(num_write==num_bytes_, ("Failed to write all requested data to the file\r\n"));
    m_begin_pos+=usize_t(m_data-m_begin)+num_write;
    m_begin=m_end=m_data=0;
  }
  else
  {
    // copy data to the buffer
    mem_copy(m_buffer, p_, num_bytes_);
    m_begin_pos+=usize_t(m_data-m_begin);
    m_begin=m_buffer;
    m_end=m_buffer+buffer_size;
    m_data=m_begin+num_bytes_;
  }
}
//----------------------------------------------------------------------------


//============================================================================
// win_file_system::iterator_impl
//============================================================================
class win_file_system::iterator_impl: public file_system_base::iterator_impl_base
{
public:
  // construction
  iterator_impl(iterator&, e_fsys_find, HANDLE, const WIN32_FIND_DATA&);
  virtual ~iterator_impl();
  //--------------------------------------------------------------------------

  // iteration
  virtual bool advance(file_system_base::iterator&);
  //--------------------------------------------------------------------------

private:
  void operator=(const iterator_impl&); // not implemented
  //--------------------------------------------------------------------------

  const e_fsys_find m_find_type;
  HANDLE m_handle;
  WIN32_FIND_DATA m_find_data;
};
//----------------------------------------------------------------------------

win_file_system::iterator_impl::iterator_impl(file_system_base::iterator &it_, e_fsys_find find_, HANDLE handle_, const WIN32_FIND_DATA &find_data_)
  :m_find_type(find_)
  ,m_handle(handle_)
  ,m_find_data(find_data_)
{
  init_iterator(it_);
  bool is_dir=(m_find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
  update(it_, m_find_data.cFileName, is_dir, !is_dir);
}
//----

win_file_system::iterator_impl::~iterator_impl()
{
  FindClose(m_handle);
}
//----

bool win_file_system::iterator_impl::advance(file_system_base::iterator &it_)
{
  // search for proper archive type
  while(FindNextFile(m_handle, &m_find_data))
  {
    // check if found correct archive type (skip "." and ".." dirs)
    bool is_dir=(m_find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
    bool is_file=!is_dir;
    if(   (   m_find_type==fsysfind_all
           || (m_find_type==fsysfind_dirs && is_dir)
           || (m_find_type==fsysfind_files && is_file))
       && (   m_find_data.cFileName[0]!='.'
           || (m_find_data.cFileName[1]!=0 && m_find_data.cFileName[1]!='.')
           || (m_find_data.cFileName[1]=='.' && m_find_data.cFileName[2]!=0)))
    {
      update(it_, m_find_data.cFileName, is_dir, is_file);
      return true;
    }
  }

  // no more archives
  return false;
}
//----------------------------------------------------------------------------


//============================================================================
// win_file_system
//============================================================================
win_file_system::win_file_system(bool set_active_)
  :file_system_base(set_active_)
{
  m_temp_write=true;
}
//----

void win_file_system::enable_temp_write(bool enable_)
{
  m_temp_write=enable_;
}
//----------------------------------------------------------------------------

win_file_system::iterator win_file_system::find_first(e_fsys_find find_, const char *dirname_, const char *path_) const
{
  // find first file
  filepath_str search=complete_path(dirname_, path_);
  search+="/*";
  WIN32_FIND_DATA find_data;
  HANDLE handle=FindFirstFileEx(search.c_str(), FindExInfoStandard, &find_data, find_==fsysfind_dirs?FindExSearchLimitToDirectories:FindExSearchNameMatch, 0, 0);
  if(handle==INVALID_HANDLE_VALUE)
    return iterator();

  // search for proper archive type
  do
  {
    // check if found correct archive type (skip "." and ".." dirs)
    bool is_dir=(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
    bool is_file=!is_dir;
    if(   (   find_==fsysfind_all
           || (find_==fsysfind_dirs && is_dir)
           || (find_==fsysfind_files && is_file))
       && (   find_data.cFileName[0]!='.'
           || (find_data.cFileName[1]!=0 && find_data.cFileName[1]!='.')
           || (find_data.cFileName[1]=='.' && find_data.cFileName[2]!=0)))
    {
      iterator it;
      PFC_NEW(iterator_impl)(it, find_, handle, find_data);
      return it;
    }

    // proceed to the next archive
  } while(FindNextFile(handle, &find_data));

  // no proper archive found
  return iterator();
}
//----------------------------------------------------------------------------

bool win_file_system::exists(const char *filename_, const char *path_) const
{
  // check if the file exists
  PFC_ASSERT(filename_);
  filepath_str fn=complete_path(filename_, path_);
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  return GetFileAttributesEx(fn.c_str(), GetFileExInfoStandard, &attribs)==TRUE;
}
//----

usize_t win_file_system::file_size(const char *filename_, const char *path_) const
{
  // get size of the file
  PFC_ASSERT(filename_);
  filepath_str fn=complete_path(filename_, path_);
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  mem_zero(&attribs, sizeof(attribs));
  PFC_VERIFY_WARN(GetFileAttributesEx(fn.c_str(), GetFileExInfoStandard, &attribs)==TRUE,
                  ("Unable to get file size for file \"%s\"\r\n", fn.c_str()));
  return attribs.nFileSizeLow;
}
//----

bool win_file_system::is_writable(const char *filename_, const char *path_) const
{
  // check if given file is writable
  PFC_ASSERT(filename_);
  filepath_str fn=(filename_, path_);
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  if(GetFileAttributesEx(fn.c_str(), GetFileExInfoStandard, &attribs)==FALSE)
    return true;
  return !(attribs.dwFileAttributes&FILE_ATTRIBUTE_READONLY);
}
//----

file_time win_file_system::mod_time(const char *filename_, const char *path_) const
{
  // get file attributes
  PFC_ASSERT(filename_);
  filepath_str fn=complete_path(filename_, path_);
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  if(GetFileAttributesEx(fn.c_str(), GetFileExInfoStandard, &attribs)==FALSE)
    return file_time();

  // convert to file time
  SYSTEMTIME utc_time, local_time;
  FileTimeToSystemTime(&attribs.ftLastWriteTime, &utc_time);
  SystemTimeToTzSpecificLocalTime(0, &utc_time, &local_time);
  return file_time(local_time.wYear, uint8(local_time.wMonth), uint8(local_time.wDay), uint8(local_time.wHour), uint8(local_time.wMinute), uint8(local_time.wSecond));
}
//----

owner_ptr<bin_input_stream_base> win_file_system::open_read(const char *filename_, const char *path_, e_file_open_check fopen_check_) const
{
  // open file for reading
  PFC_ASSERT(filename_);
  filepath_str fn=complete_path(filename_, path_);
#ifdef PFC_USE_CREATEFILE2
  wchar_t wfn[filepath_str::str_capacity];
  cstr_to_wcstr(wfn, fn.c_str(), filepath_str::str_capacity);
  HANDLE f=CreateFile2(wfn, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0);
#else
  HANDLE f=CreateFile(fn.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
#endif
  if(f==INVALID_HANDLE_VALUE)
  {
    PFC_CHECK_MSG(fopen_check_!=fopencheck_abort, ("Unable to open file \"%s\" for reading\r\n", complete_path(filename_, path_).c_str()));
    if(fopen_check_==fopencheck_warn)
      PFC_WARN(("Unable to open file \"%s\" for reading\r\n", complete_path(filename_, path_).c_str()));
  }
  return f!=INVALID_HANDLE_VALUE?PFC_NEW(input_stream)(f, fn.c_str()):0;
}
//----

owner_ptr<bin_output_stream_base> win_file_system::open_write(const char *filename_, const char *path_, e_file_open_write_mode mode_, uint64 fpos_, bool makedir_, e_file_open_check fopen_check_)
{
  // setup file path and target file path
  PFC_ASSERT(filename_);
  filepath_str fpath=complete_path(filename_, path_);
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  if(GetFileAttributesEx(fpath.c_str(), GetFileExInfoStandard, &attribs)==TRUE &&
     (attribs.dwFileAttributes&FILE_ATTRIBUTE_READONLY))
  {
    PFC_CHECK_MSG(fopen_check_!=fopencheck_abort, ("Unable to open file \"%s\" for writing\r\n", complete_path(filename_, path_).c_str()));
    if(fopen_check_==fopencheck_warn)
      PFC_WARN(("Unable to open file \"%s\" for writing\r\n", complete_path(filename_, path_).c_str()));
    return 0;
  }
  filepath_str fpath_target;
  bool temp_write=m_temp_write && (mode_==fopenwritemode_clear || !exists(filename_, path_));
  if(temp_write)
    build_temp_filepath(fpath_target, fpath);
  else
    fpath_target=fpath;

  // open file for writing
  if(makedir_)
  {
    filepath_str fp=filename_;
    const char *fname=split_filepath(fp.c_str());
    make_directory(fp.c_str()!=fname?fp.c_str():"", path_);
  }
#ifdef PFC_USE_CREATEFILE2
  wchar_t wfp[filepath_str::str_capacity];
  cstr_to_wcstr(wfp, fpath_target.c_str(), filepath_str::str_capacity);
  HANDLE f=CreateFile2(wfp, GENERIC_WRITE, FILE_SHARE_WRITE, mode_==fopenwritemode_clear?CREATE_ALWAYS:OPEN_ALWAYS, 0);
#else
  HANDLE f=CreateFile(fpath_target.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, mode_==fopenwritemode_clear?CREATE_ALWAYS:OPEN_ALWAYS, 0, 0);
#endif
  if(f==INVALID_HANDLE_VALUE)
  {
    PFC_CHECK_MSG(fopen_check_!=fopencheck_abort, ("Unable to open file \"%s\" for writing\r\n", complete_path(filename_, path_).c_str()));
    if(fopen_check_==fopencheck_warn)
      PFC_WARN(("Unable to open file \"%s\" for writing\r\n", complete_path(filename_, path_).c_str()));
    return 0;
  }

  // set file pointer and return file output stream
  LARGE_INTEGER pos={0, 0};
  if(mode_==fopenwritemode_keep && fpos_==uint64(-1))
    SetFilePointerEx(f, pos, 0, FILE_END);
  else if(fpos_ && fpos_!=uint64(-1))
  {
    pos.QuadPart=fpos_;
    SetFilePointerEx(f, pos, 0, FILE_BEGIN);
  }
  return PFC_NEW(output_stream)(f, temp_write, fpath);
}
//----

bool win_file_system::delete_file(const char *filename_, const char *path_)
{
  // delete file
  PFC_ASSERT(filename_);
  filepath_str fpath=complete_path(filename_, path_);
  return DeleteFile(fpath.c_str())==TRUE;
}
//----

bool win_file_system::rename_file(const char *filename_, const char *new_filename_, const char *path_, bool overwrite_existing_)
{
  // rename given filename to the new filename
  PFC_ASSERT(filename_);
  PFC_ASSERT(new_filename_);
  filepath_str fpath=complete_path(filename_, path_);
  filepath_str fpath_new=complete_path(new_filename_, path_);
  return move_file(fpath.c_str(), fpath_new.c_str(), overwrite_existing_?MOVEFILE_REPLACE_EXISTING:0);
}
//----

void win_file_system::touch_file(const char *filename_, const char *path_)
{
  // try to open file
  PFC_ASSERT(filename_);
  filepath_str fpath=complete_path(filename_, path_);
  const char *fp=fpath.c_str();
#ifdef PFC_USE_CREATEFILE2
  wchar_t wfp[filepath_str::str_capacity];
  cstr_to_wcstr(wfp, fp, filepath_str::str_capacity);
  HANDLE h=CreateFile2(wfp, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING, 0);
#else
  HANDLE h=CreateFile(fp, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
#endif
  if(h!=INVALID_HANDLE_VALUE)
  {
    // get current system time and convert to file time
    SYSTEMTIME time;
    FILETIME file_time;
    GetSystemTime(&time);
    PFC_VERIFY_MSG(SystemTimeToFileTime(&time, &file_time), ("Converting system time to file time failed\r\n"));

    // update file access, write and change times
    FILE_BASIC_INFO info;
    PFC_VERIFY_MSG(GetFileInformationByHandleEx(h, FileBasicInfo, &info, sizeof(info)), ("Unable to get information for file \"%s\"\r\n", fpath.c_str()));
    LARGE_INTEGER t={file_time.dwLowDateTime, LONG(file_time.dwHighDateTime)};
    info.LastAccessTime=t;
    info.LastWriteTime=t;
    info.ChangeTime=t;
    PFC_VERIFY_MSG(SetFileInformationByHandle(h, FileBasicInfo, &info, sizeof(info)), ("Unable to set information for file \"%s\"\r\n", fpath.c_str()));
    CloseHandle(h);
  }
}
//----------------------------------------------------------------------------

bool win_file_system::make_directory(const char *dirname_, const char *path_)
{
  // setup full directory name and get attributes
  PFC_ASSERT(dirname_);
  filepath_str dn=complete_path(dirname_, path_);
  if(!dn.size())
    return true;
  WIN32_FILE_ATTRIBUTE_DATA attribs;
  if(!GetFileAttributesEx(dn.c_str(), GetFileExInfoStandard, &attribs))
  {
    // strip off potential trailing slash
    filepath_str parent_dn=dn;
    unsigned parent_dnsize=(unsigned)parent_dn.size();
    if(parent_dn[parent_dnsize-1]=='/')
    {
      parent_dn[--parent_dnsize]=0;
      dn.resize(parent_dnsize);
    }

    // try to create parent directories backwards starting from the target dir
    while(parent_dnsize && !CreateDirectory(parent_dn.c_str(), 0))
    {
      while(--parent_dnsize && parent_dn[parent_dnsize]!='/');
      parent_dn[parent_dnsize]=0;
    }
    if(!parent_dnsize)
    {
      PFC_ERROR(("Unable to create directory \"%s\"\r\n", dn.c_str()));
      return false;
    }

    // try to create directories forward from the first successfully created parent dir
    unsigned dnsize=(unsigned)dn.size();
    while(parent_dnsize<dnsize)
    {
      parent_dn[parent_dnsize]='/';
      while(++parent_dnsize<dnsize && parent_dn[parent_dnsize]);
      if(!CreateDirectory(parent_dn.c_str(), 0))
      {
        PFC_ERROR(("Unable to create directory \"%s\"\r\n", dn.c_str()));
        return false;
      }
    }
  }
  else
  {
    // verify that the existing file isn't a directory
    if(!(attribs.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
      PFC_ERROR(("Unable to create directory \"%s\" because a file with that name already exists\r\n", dn.c_str()));
      return false;
    }
  }

  // directory successfully created
  return true;
}
//----

bool win_file_system::delete_directory(const char *dirname_, const char *path_, bool delete_content_)
{
  PFC_ASSERT(dirname_);
  filepath_str dname=complete_path(dirname_, path_);
  if(delete_content_)
  {
    // recursively delete the directory content
    iterator it=find_first(fsysfind_all, dirname_, path_);
    while(is_valid(it))
    {
      if(it.is_dir())
      {
        // build full directory name and delete the directory
        filepath_str path=dname;
        char last_char=path[path.size()-1];
        if(last_char!='/' && last_char!='\\')
          path+='\\';
        path+=it.name();
        delete_directory(path.c_str(), 0, true);
      }
      else
        delete_file(it.name(), dname.c_str());
      ++it;
    }
  }

  // try to remove empty directory
  filepath_str dpath=directory();
  dpath+=dname;
  return RemoveDirectory(dpath.c_str())==TRUE;
}
//----

bool win_file_system::rename_directory(const char *dirname_, const char *new_dirname_, const char *path_)
{
  // rename given directory name to the new directory name
  PFC_ASSERT(dirname_);
  PFC_ASSERT(new_dirname_);
  filepath_str dname=complete_path(dirname_, path_);
  filepath_str dname_new=complete_path(new_dirname_, path_);
  return move_file(dname.c_str(), dname_new.c_str(), 0);
}
//----

owner_ptr<directory_monitor_base> win_file_system::create_directory_monitor(const char *dirname_, bool notify_subdirs_) const
{
  /*todo*/
  PFC_ERROR_NOT_IMPL();
  return 0;
}
//----------------------------------------------------------------------------

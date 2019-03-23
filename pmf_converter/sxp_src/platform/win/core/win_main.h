//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_WIN_MAIN_H
#define PFC_WIN_MAIN_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include "sxp_src/core/core.h"
namespace pfc
{

// new
int win_main(int argc_, const char *argv_[]);
int win_main(HINSTANCE inst_, HINSTANCE prev_inst_, LPSTR cmd_, int show_flags_);
#define PFC_MAIN(args__, num_args__)\
  int __cdecl main(int argc_, const char *argv_[]) {return pfc::win_main(argc_, argv_);}\
  int __stdcall WinMain(HINSTANCE inst_, HINSTANCE prev_inst_, LPSTR cmd_, int show_flags_) {return pfc::win_main(inst_, prev_inst_, cmd_, show_flags_);}\
  int pfc::entry(args__, num_args__)
//----------------------------------------------------------------------------

//============================================================================
} // namespace pfc
#endif

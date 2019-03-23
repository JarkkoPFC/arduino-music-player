//============================================================================
// Spin-X Platform
//
// Copyright (c) 2016, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_LINUX_INET_H
#define PFC_LINUX_INET_H
//----------------------------------------------------------------------------
                 

//============================================================================
// interface
//============================================================================
// external
#include "sxp_src/core/core.h"
namespace pfc
{

// new
int linux_main(int argc_, const char *argv_[]);
#define PFC_MAIN(args__, num_args__)\
  int main(int argc_, const char *argv_[]) {return pfc::linux_main(argc_, argv_);}\
  int pfc::entry(args__, num_args__)
//----------------------------------------------------------------------------

//============================================================================
} // namespace pfc
#endif

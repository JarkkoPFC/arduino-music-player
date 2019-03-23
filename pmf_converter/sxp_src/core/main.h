//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_MAIN_H
#define PFC_CORE_MAIN_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
#include "sxp_src/core/core.h"
#include PFC_STR(PFC_CAT2(sxp_src/platform/PFC_PLATFORM_SRC_STR/core/PFC_PLATFORM_SRC_STR, _main.h))  // expands to e.g. "sxp_src/platform/win/core/win_main.h"
namespace pfc
{

// new
int entry(const char *args_[], unsigned num_args_);
//----------------------------------------------------------------------------

//============================================================================
} // namespace pfc
#endif

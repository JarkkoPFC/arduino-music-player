//============================================================================
// Spin-X Platform
//
// Copyright (c) 2019, Profoundic Technologies, Inc.
// All rights reserved.
//============================================================================

#ifndef PFC_CORE_CONFIG_H
#define PFC_CORE_CONFIG_H
//----------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
namespace pfc
{
//============================================================================
// build option config
//============================================================================
#define PFC_BUILDOP_INTRINSICS               // use compiler intrinsics
// per build config
#define PFC_BUILDOP_EXCEPTIONS               // support exceptions
#define PFC_BUILDOP_LOGS                     // enable logging
#define PFC_BUILDOP_WARNINGS                 // enable warnings
#define PFC_BUILDOP_WARNINGS_FILEPATH        // write file-path in warning messages
#define PFC_BUILDOP_ERRORS                   // enable errors
#define PFC_BUILDOP_ERRORS_FILEPATH          // write file-path in error messages
#define PFC_BUILDOP_ASSERTS                  // enable asserts
#define PFC_BUILDOP_CHECKS                   // enable checks
#define PFC_BUILDOP_PEDANTIC                 // enable pedantic asserts & checks
//----
#ifdef PFC_RETAIL
// retail build options
#undef PFC_BUILDOP_EXCEPTIONS
#undef PFC_BUILDOP_LOGS
#undef PFC_BUILDOP_WARNINGS
#undef PFC_BUILDOP_ERRORS
#undef PFC_BUILDOP_ASSERTS
#undef PFC_BUILDOP_CHECKS
#undef PFC_BUILDOP_PEDANTIC
#endif
// debug options
#define PFC_BUILDOP_HEAP_CHECKS


//============================================================================
// build, platform and compiler strings
//============================================================================
#ifdef PFC_DEBUG
#define PFC_BUILD_STR debug
#elif defined(PFC_RELEASE)
#define PFC_BUILD_STR release
#elif defined(PFC_RETAIL)
#define PFC_BUILD_STR retail
#else
#error Target build not specified.
#endif
//----

#ifdef PFC_PLATFORM_WIN32
#define PFC_PLATFORM_STR win32
#define PFC_PLATFORM_SRC_STR win
#define PFC_PLATFORM_32BIT
#elif defined(PFC_PLATFORM_WIN64)
#define PFC_PLATFORM_STR win64
#define PFC_PLATFORM_SRC_STR win
#define PFC_PLATFORM_64BIT
#elif defined(PFC_PLATFORM_LINUX32)
#undef linux
#define PFC_PLATFORM_STR linux32
#define PFC_PLATFORM_SRC_STR linux
#define PFC_PLATFORM_32BIT
#elif defined(PFC_PLATFORM_LINUX64)
#undef linux
#define PFC_PLATFORM_STR linux64
#define PFC_PLATFORM_SRC_STR linux
#define PFC_PLATFORM_64BIT
#else
#error Target platform not specified.
#endif
//----

#ifdef PFC_COMPILER_MSVC2008
#define PFC_COMPILER_MSVC
#define PFC_COMPILER_STR vs2008
#define PFC_COMPILER_SRC_STR msvc
#define PFC_COMPILER_LIB_EXT .lib
#undef PFC_ENGINEOP_D3D12
#elif defined(PFC_COMPILER_MSVC2010)
#define PFC_COMPILER_MSVC
#define PFC_COMPILER_STR vs2010
#define PFC_COMPILER_SRC_STR msvc
#define PFC_COMPILER_LIB_EXT .lib
#undef PFC_ENGINEOP_D3D12
#elif defined(PFC_COMPILER_MSVC2012)
#define PFC_COMPILER_MSVC
#define PFC_COMPILER_STR vs2012
#define PFC_COMPILER_SRC_STR msvc
#define PFC_COMPILER_LIB_EXT .lib
#undef PFC_ENGINEOP_D3D12
#elif defined(PFC_COMPILER_MSVC2013)
#define PFC_COMPILER_MSVC
#define PFC_COMPILER_STR vs2013
#define PFC_COMPILER_SRC_STR msvc
#define PFC_COMPILER_LIB_EXT .lib
#elif defined(PFC_COMPILER_MSVC2015)
#define PFC_COMPILER_MSVC
#define PFC_COMPILER_STR vs2015
#define PFC_COMPILER_SRC_STR msvc
#define PFC_COMPILER_LIB_EXT .lib
#elif defined(PFC_COMPILER_MSVC2017)
#define PFC_COMPILER_MSVC
#define PFC_COMPILER_STR vs2017
#define PFC_COMPILER_SRC_STR msvc
#define PFC_COMPILER_LIB_EXT .lib
#elif defined(PFC_COMPILER_GCC)
#define PFC_COMPILER_STR gcc
#define PFC_COMPILER_SRC_STR gcc
#define PFC_COMPILER_LIB_EXT .a
#else
#error Target compiler not supported.
#endif
//----------------------------------------------------------------------------


//============================================================================
// pre-compiler macros
//============================================================================
#define PFC_COMMA ,
// stringify, e.g. PFC_STR(x) => "x"
#define PFC_STR(x__) PFC_STR_DO(x__)
#define PFC_STR_DO(x__) #x__
// concatenation of 2-10 values, e.g. PFC_CAT3(a, b, c) => abc
#define PFC_CAT2(a__, b__) PFC_CAT2_DO(a__, b__)
#define PFC_CAT2_DO(a__, b__) a__##b__
#define PFC_CAT3(a__, b__, c__) PFC_CAT3_DO(a__, b__, c__)
#define PFC_CAT3_DO(a__, b__, c__) a__##b__##c__
#define PFC_CAT4(a__, b__, c__, d__) PFC_CAT4_DO(a__, b__, c__, d__)
#define PFC_CAT4_DO(a__, b__, c__, d__) a__##b__##c__##d__
#define PFC_CAT5(a__, b__, c__, d__, e__) PFC_CAT5_DO(a__, b__, c__, d__, e__)
#define PFC_CAT5_DO(a__, b__, c__, d__, e__) a__##b__##c__##d__##e__
#define PFC_CAT6(a__, b__, c__, d__, e__, f__) PFC_CAT6_DO(a__, b__, c__, d__, e__, f__)
#define PFC_CAT6_DO(a__, b__, c__, d__, e__, f__) a__##b__##c__##d__##e__##f__
#define PFC_CAT7(a__, b__, c__, d__, e__, f__, g__) PFC_CAT7_DO(a__, b__, c__, d__, e__, f__, g__)
#define PFC_CAT7_DO(a__, b__, c__, d__, e__, f__, g__) a__##b__##c__##d__##e__##f__##g__
#define PFC_CAT8(a__, b__, c__, d__, e__, f__, g__, h__) PFC_CAT8_DO(a__, b__, c__, d__, e__, f__, g__, h__)
#define PFC_CAT8_DO(a__, b__, c__, d__, e__, f__, g__, h__) a__##b__##c__##d__##e__##f__##g__##h__
#define PFC_CAT9(a__, b__, c__, d__, e__, f__, g__, h__, i__) PFC_CAT9_DO(a__, b__, c__, d__, e__, f__, g__, h__, i__)
#define PFC_CAT9_DO(a__, b__, c__, d__, e__, f__, g__, h__, i__) a__##b__##c__##d__##e__##f__##g__##h__##i__
#define PFC_CAT10(a__, b__, c__, d__, e__, f__, g__, h__, i__, j__) PFC_CAT10_DO(a__, b__, c__, d__, e__, f__, g__, h__, i__, j__)
#define PFC_CAT10_DO(a__, b__, c__, d__, e__, f__, g__, h__, i__, j__) a__##b__##c__##d__##e__##f__##g__##h__##i__##j__
//----------------------------------------------------------------------------

//============================================================================
} // namespace pfc
#endif

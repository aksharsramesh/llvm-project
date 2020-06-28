//===----------------------------QEDOptions.h--------------------------------//
//                                                                           //
//                     The LLVM Compiler Infrastructure                      //
//                                                                           //
// This file is distributed under the Licence of Compiler Tree Technologies  //
//                                                                           //
//===---------------------------------------------------------------------===//
//                                                                           //
// This file contains required QED command line options.                     //
// Few options are required in more than one passes.                         //
// Hence declared in a single place                                          //
//                                                                           //
//===---------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"

namespace llvm {

/// \brief Enable or disable eddiv
extern cl::opt<bool> EnableEddiv;

/// \brief Enable or disable plc
extern cl::opt<bool> EnablePlc;

/// \brief Enable or disable cfcss
extern cl::opt<bool> EnableCfcss;

/// \brief Enable or disable cftss
extern cl::opt<bool> EnableCftss;

/// \brief number of alloca to consider in Eddiv or/and
/// number loads to consider in PLC.
extern cl::opt<int> QedDepth;

/// \brief enable or disable verify routine
extern cl::opt<bool> Verify;

/// \brief reserving all the caller save
extern cl::opt<bool> ReserveRegs;

/// \brief Print static function counter in CFTSS
extern cl::opt<bool> PrintFuncSignature;
}

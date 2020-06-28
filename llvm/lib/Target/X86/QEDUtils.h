//--------------------------QEDUtils.h-------------------------------//
//                                                                   //
//                     The LLVM Compiler Infrastructure              //
//                                                                   //
// This file is distributed under the Compiler Tree Technology       //
// License. See LICENSE.TXT for details.                             //
//                                                                   //
//-------------------------------------------------------------------//
//                                                                   //
// This file is header file for QEDUtils.cpp                         //
//                                                                   //
//===================================================================//

#include "QEDOptions.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

namespace llvm {

class QEDU {

public:

  /// \brief Map to keep track of all local variables promoted to global
  /// in EDDI-V transformation.
  static std::map<GlobalVariable*, GlobalVariable*> GlobalsMap;

  /// \brief Verify whether the added instrucions have proper metadata and
  /// debug location.
  static void verify(std::vector<Instruction *> Inst, Function &F);

  /// \brief Check whether this is a library call
  static bool isQEDLibCall(Function *F);

  /// \brief Check whether this is a library call
  static bool isQEDLibCall(MachineFunction *MF);

  /// \brief Add a unique debug location to the instructions added for QED.
  /// This is required to identify these instructions uniquely in MIR.
  static void addDebugLoc(Instruction *I);

  /// \brief Add metadata to the instructions innserted.
  static void addMetaData(Instruction *I);

  /// \brief Add metadata to the instructions innserted, metadata of 
  /// \param MDString kind.
  static void addMetaData(Instruction *I,StringRef MDString);

  /// Add debug location to the instructions inserted.
  static void addDebugLoc(MachineInstr *MI);

  /// \brief Check whether this instruction has a valid Metadata
  static bool hasValidMetadata(Instruction *I);

  /// \brief Check wether this instruction has valid metada 
  /// of \param MDString kind.
  static bool hasValidMetadata(Instruction *I, StringRef MDString);

  /// \brief Check whether this function has no QED attribute
  static bool hasNoQEDAttribute(Function *F);
  static bool hasNoQEDAttribute(MachineFunction *MF);

  /// \brief check whether this instruction has a debug location
  /// pointing to location 99, 99;
  static bool hasValidDL(Instruction *I);
};
}

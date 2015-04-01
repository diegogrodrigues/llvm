//===-- MachineFunctionAnalysis.cpp ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the definitions of the MachineFunctionAnalysis members.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/GCMetadata.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
using namespace llvm;

char MachineFunctionAnalysis::ID = 0;

MachineFunctionAnalysis::MachineFunctionAnalysis(const TargetMachine &tm) :
  FunctionPass(ID), TM(tm), machineFunctionTmp(0) {
  initializeMachineModuleInfoPass(*PassRegistry::getPassRegistry());
}

MachineFunctionAnalysis::~MachineFunctionAnalysis() {
  releaseMemory();
  assert(!machineFunctionTmp && "MachineFunctionAnalysis left initialized!");
}

void MachineFunctionAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineModuleInfo>();
}

bool MachineFunctionAnalysis::doInitialization(Module &M) {
  MachineModuleInfo *MMI = getAnalysisIfAvailable<MachineModuleInfo>();
  assert(MMI && "MMI not around yet??");
  MMI->setModule(&M);
  NextFnNum = 0;
  return false;
}


bool MachineFunctionAnalysis::runOnFunction(Function &F) {
  assert(!machineFunctionTmp && "MachineFunctionAnalysis already initialized!");
  machineFunctionTmp = new MachineFunction(&F, TM, NextFnNum++,
                           getAnalysis<MachineModuleInfo>(),
                           getAnalysisIfAvailable<GCModuleInfo>());

  return false;
}


void MachineFunctionAnalysis::releaseMemory() {
  delete machineFunctionTmp;
  machineFunctionTmp = 0;
}

void MachineFunctionAnalysis::countInstruction() {

	int count =0;
	for (MachineFunction::const_iterator I = machineFunctionTmp->begin(), E = machineFunctionTmp->end(); I != E; ++I) {
	    for (MachineBasicBlock::const_iterator II = I->begin(), IE = I->end(); II != IE; ++II) {
	    	count++;
	    }
	}
	errs() <<">>>>> total Machine instruction:" << count <<"\n";
}

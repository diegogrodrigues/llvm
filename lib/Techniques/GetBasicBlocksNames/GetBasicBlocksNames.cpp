

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"


#include <fstream>


using namespace llvm;
using namespace std;

//STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct GetBasicBlocksNames : public ModulePass {
    static char ID; // Pass identification, replacement for typeid


    GetBasicBlocksNames() : ModulePass(ID) {
    }



    bool runOnModule(Module &M){
		string input = M.getModuleIdentifier();

		int pos = input.find_last_of(".");
		string output = input.substr(0,pos);
		output = output + ".basicBlocks";
		///errs()<<output<<"\n";
    	ofstream ofs;
    	ofs.open (output.c_str());
		//Steps through each module function.
		for(Module::iterator funAux = M.begin(); funAux != M.end(); funAux++){
			if(!funAux->isDeclaration()&&!funAux->hasFnAttribute("error-handler")){
				Function *F = funAux;
				for (Function::iterator bbAux = F->begin(); bbAux != F->end(); bbAux++){
					BasicBlock *BB = bbAux;
					//errs() << funAux->getName()<<":" << BB->getName() << "\n";
					string name =funAux->getName();
					name+=":";
					name+= BB->getName();
					ofs << name.c_str();

					for (succ_iterator SC = succ_begin(BB); SC != succ_end(BB); ++SC) {
						BasicBlock * succ = *SC;
						ofs << "|";
						string sucessor = succ->getName();
						ofs << sucessor;
					}
					ofs << "\n";

				}
			}
		}
		ofs.close();
    }
  };
}

char GetBasicBlocksNames::ID = 0;
static RegisterPass<GetBasicBlocksNames> X("GetBasicBlocksNames", "Get basic blocks names.", false, true);



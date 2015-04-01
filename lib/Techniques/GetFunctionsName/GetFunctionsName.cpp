

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>


using namespace llvm;
using namespace std;

//STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct GetFunName : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    GetFunName() : ModulePass(ID) {
    }

    bool runOnModule(Module &M){
		string input = M.getModuleIdentifier();
		int pos = input.find_last_of(".");
		string output = input.substr(0,pos);
		output = output + ".functions";
		///errs()<<output<<"\n";
    	ofstream ofs;
    	ofs.open (output.c_str());
		//Steps through each module function.
		for(Module::iterator funAux = M.begin(); funAux != M.end(); funAux++){
			if(!funAux->isDeclaration()&&!funAux->hasFnAttribute("error-handler")){
				string name =funAux->getName();
				ofs << name.c_str();
				ofs << "\n";
			}
		}
		ofs.close();
    }
  };
}

char GetFunName::ID = 0;
static RegisterPass<GetFunName> X("GetFunName", "Get functions name pass.", false, true);



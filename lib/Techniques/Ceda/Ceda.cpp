#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "Node.h"
#include "Signatures.h"
#include "../Utils/Utils32_cfgs.h"


using namespace llvm;
using namespace std;
using namespace SignaturesSpace;


#define NUM_BITS 64

namespace NewCFC{

struct CEDA: public ModulePass  {
	static char ID;

	GlobalVariable *signature; // Signature register. "A run-time variable which is continuously updated to monitor the executions of the program.
	Function *errHandler;  //Error handler function.

	//Define a vector of nodes.
	vector<Node*> vNodes;


	CEDA() : ModulePass(ID) {

	}

	bool runOnModule(Module &M){
		LLVMContext &ctx = M.getContext();
  		return module(M, ctx);
	}

	//
	bool module(Module &M, LLVMContext &ctx){
		bool Changed = false;

		Node nodes;
		Signatures sign;

		//Get the vector of nodes with its informations.
		vNodes = nodes.defineNodesInformation(M);
		//Creates the necessary sets Liks Nets,Nets_Pred and A_Sig.
		sign.createSets(vNodes);
		//sign.printNets();
		//sign.printNetPreds();
		//sign.printA_Sig();
		//nodes.printNodes(vNodes);
		//Calc the signatures for each node.
		sign.calcSignatureValues(vNodes);
		//nodes.printNodes(vNodes);


		if(sign.verifySignaturesD1(vNodes) && sign.verifySignaturesD2(vNodes)){

			//Create global signature and error handler.
			addGlobals(M, ctx);

			//Add assertions to functions
			addAssertionsFunctions(M,vNodes, ctx);
		}
		return Changed;
	}

	//Add globals variables.
	void addGlobals(Module& M, LLVMContext& ctx){

		//Add a global variable named "Sreg" with the initial value of 0.
		signature = new GlobalVariable(IntegerType::get(ctx,NUM_BITS), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,NUM_BITS), 0), "signature");
		M.getGlobalList().push_back(signature);

		//Create Error handler functions
		errHandler = CreateErrorHandlerFunction(M, ctx);
	}
	//Run on basic blocks
	bool addAssertionsFunctions(Module &M,vector<Node*> vNodes, LLVMContext& ctx){
		bool Changed = false;

		//Put the pointer of all functions in a vector.
		vector<Function*> vecfun;
		for(Module::iterator Faux = M.begin(); Faux != M.end(); Faux++){
			//*** IMPORTANT: Do not put this kind of function of the vector.
			if((!Faux->isDeclaration())&&!Faux->hasFnAttribute("error-handler")){
			vecfun.push_back(Faux);
			}
		}

		for(vector<Function*>::iterator itFun = vecfun.begin(); itFun != vecfun.end(); itFun++){
			Function* F = *itFun;

			//Put all basc block in a vector.
			vector<BasicBlock*> vecBBs;
			for (Function::iterator bbAux = F->begin(); bbAux != F->end(); bbAux++){
				vecBBs.push_back(bbAux);
			}

			//Create block into function to jump to error handler. Put it after the filling of 'functBBs'.
			BasicBlock *bbErrorHandler = BasicBlock::Create(ctx, "bbErrorHandler", F);
			IRBuilder<> builder1(bbErrorHandler);
			builder1.CreateCall(errHandler,ConstantInt::get(IntegerType::get(ctx,32), 0));
			builder1.CreateUnreachable();


			bool first = true;
			for (vector<BasicBlock*>::iterator itBB = vecBBs.begin(); itBB != vecBBs.end(); itBB++){
				BasicBlock *BB = *itBB;


				Node *node;
				node=node->getNode(vNodes,BB);
				if(node!=NULL){
					addAssertionNodes(node, bbErrorHandler, ctx,first);
					first =false;
				}else{
					errs() <<"Não achou!  "<<BB->getName()<<"\n";
				}
			}

		}
		//

		//for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		//}

		return Changed;
	}


	bool insertSignatureUpdateCall(LLVMContext& ctx, Function *F, BasicBlock *bb,  GlobalVariable *signature){
		//Find a call
		for(BasicBlock::iterator itBB=bb->begin(); itBB !=bb->end(); itBB++){
			Instruction *inst = itBB;
			if (CallInst *call = dyn_cast<CallInst>(inst)){
				Function *fcall = call->getCalledFunction();
				if(fcall !=0){
					if(!fcall->isDeclaration() || callingModuleFunction(F,call)){
					//if(!fcall->isDeclaration() || fcall->getName()=="qsort"){
						Instruction *header= &bb->front();
						AllocaInst *tempSignature = new AllocaInst (IntegerType::get(ctx,NUM_BITS), 0, "signature_"+F->getName()+"_"+bb->getName(), header);
						//errs() <<F->getName()<<"  " <<bb->getName()<< " call:" <<call->getName() <<"\n";
						bool found = false;
						Instruction *nextInst;
						for(BasicBlock::iterator next=bb->begin(); next !=bb->end(); next++){
							//errs() << "   Instructions:" <<next->getOpcodeName() <<"\n";
							if(found){
								nextInst = next;
								//errs() << "Next instruction(1):" <<nextInst->getOpcodeName() <<"\n";
								break;
							}
							if(inst == next){
								//errs() << "call(1):" <<next->getName() <<"\n";
								found=true;
							}
						}
						LoadInst *loadSignature = new LoadInst(signature, "load_sig", call);
						StoreInst *storeSig = new StoreInst(loadSignature, tempSignature, call);
						LoadInst *loadFunSignature = new LoadInst(tempSignature, "load_temp_sig", nextInst);
						StoreInst *storeFuncSig = new StoreInst(loadFunSignature, signature, nextInst);

					}
				}
			}
		}
		return true;
	}

	void addAssertionNodes(Node *node, BasicBlock* bbErrorHandler, LLVMContext& ctx, bool first){

		LoadInst *loadSignature;
		Value *valueD1, *valueD2;
		Instruction* opResult;
		StoreInst *storeNew;

		BasicBlock *BB = node->getBB();
		Function *F = BB->getParent();


		IRBuilder<> builder(BB);
		//errs() << ">Function name: " << F->getName() << "" << F->isDeclaration() <<" "<< F->getType() <<"\n";
		//get first instruction of the basic block.
		Instruction *header= &BB->front();


		if(first){
			valueD1 = ConstantInt::get(IntegerType::get(ctx,NUM_BITS), node->getSignature());
			storeNew = new StoreInst(valueD1, signature, header);
		}
		else{
			loadSignature = new LoadInst(signature, "load_sig", header);
			valueD1 = ConstantInt::get(IntegerType::get(ctx,NUM_BITS), node->getd1Value());

			//Add head instruction
			if(node->getType()==typeA){
				opResult = BinaryOperator::CreateAnd(loadSignature,valueD1, "sig_and_d1", header);
			}else{
				opResult = BinaryOperator::CreateXor(loadSignature,valueD1, "sig_xor_d1", header);

			}
			storeNew = new StoreInst(opResult, signature, header);
		}

		//Get the last instruction of the BB
		Instruction *tail = BB->getTerminator();

		loadSignature = new LoadInst(signature, "load_sig", tail);
		valueD2 = ConstantInt::get(IntegerType::get(ctx,NUM_BITS), node->getd2Value());
		opResult = BinaryOperator::CreateXor(loadSignature,valueD2, "sig_and_d2", tail);
		storeNew = new StoreInst(opResult, signature, tail);

		BasicBlock *originalCondBranch;
		Value *expectedSig;

		expectedSig = ConstantInt::get(IntegerType::get(ctx,NUM_BITS), node->getExitSignature());
		originalCondBranch = createOriginalBranch(F,BB, ctx,tail);

		if(originalCondBranch!=NULL){
			//Verify signature.
			//loadSignature = builder.CreateLoad(signature, "load_sig");
			Value *compSig = builder.CreateICmpEQ(opResult, expectedSig,"comp_sig");
			builder.CreateCondBr(compSig, originalCondBranch, bbErrorHandler);
		}
		insertSignatureUpdateCall(ctx, F, BB, signature);
	}

};

char CEDA::ID = 0;
static RegisterPass<CEDA> X ("CEDA","Control-flow Error Detection through Assertion", false,false);

}





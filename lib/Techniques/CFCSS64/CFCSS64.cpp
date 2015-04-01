#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#include <vector>
#include <map>

#include "../Utils64/Utils64_cfgs.h"


using namespace llvm;
using namespace std;


namespace {

struct CFCSS64: public ModulePass  {

	static char ID;

	Function *printf_func;
	Function *errHandler;

	mapFunction mapFunc;
	mapBB basicBlocks;

	GlobalVariable *signature;
	GlobalVariable *upperD;

	CFCSS64() : ModulePass(ID) {

	}

	bool runOnModule(Module &M){
  		return module(M);
	}

	bool module(Module &M){
		bool Changed = false;

		LLVMContext &ctx = M.getContext();

		//Add a global variable called "Signature" with the initial value 0.
		signature = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature");
		M.getGlobalList().push_back(signature);

		upperD = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD");
		M.getGlobalList().push_back(upperD);

		//bbNumber = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "bbNumber");
		//M.getGlobalList().push_back(bbNumber);

		//Read file content.
		string input = M.getModuleIdentifier();
		int pos = input.find_last_of(".");
		string input_SBL = input.substr(0,pos);
		input_SBL = input_SBL + "_SBL.txt";
		//errs()<< input_SBL<<"\n";
		//if(!readFileContent(&mapFunc, input_SBL)){
		//	return false;
		//}
		//displayMapContent(mapFunc);

		generateBBIDs(&basicBlocks, M);
		//displayBBsData(basicBlocks);
		calcDvalue(&basicBlocks);
		//displayBBsData(basicBlocks);

		errHandler = CreateErrorHandlerFunction(M, ctx); //Error handler functions

		//Steps through each module function.
		for(Module::iterator funAux = M.begin(); funAux != M.end(); funAux++){
			Changed = functions(M,funAux,ctx);
		}
		return Changed;
	}

	bool functions(Module &M, Function *F,  LLVMContext& ctx){
		bool Changed = true;
		if(!F->isDeclaration()&&!F->hasFnAttribute("error-handler")){
		//if((!F->isDeclaration())&&(F->getName()!="ErrorHandler")){

			//Put all basic blocks of the function into a vector.
			vector <BasicBlock *> functBBs;
			for (Function::iterator bbAuxVec = F->begin(); bbAuxVec != F->end(); bbAuxVec++){
				functBBs.push_back(bbAuxVec);
			}
			//Create block into function to jump to error handler. Put it after the filling of 'functBBs'.
			BasicBlock *bbErrorHandler = BasicBlock::Create(ctx, "bbErrorHandler", F);
			IRBuilder<> builder1(bbErrorHandler);
			builder1.CreateCall(errHandler,ConstantInt::get(IntegerType::get(ctx,64), 0));
			builder1.CreateUnreachable();

			AllocaInst *tempSignature;
			AllocaInst *tempUpperD;
			bool firstBB =  true;

			for (vector<BasicBlock *>::iterator itVecBB = functBBs.begin(); itVecBB != functBBs.end(); itVecBB++){
				mapBB::iterator itBB = basicBlocks.find(*itVecBB);
				if(itBB ==basicBlocks.end() ){
					//BasicBlock *bbAux2 = itBB->first;
					//errs() <<  bbAux2->getName() <<"\n";
					errs()<<"Error! Basic block not found!\n";
					continue;
				}
				BasicBlock *bbChange = itBB->first;
				//Get extra data for current basic block.
				NodeExtraData dataBB  = itBB->second;
				IRBuilder<> builderbbChange(bbChange);
				//errs()<<"Function: "<<F->getName() <<"  BB Name: "<<bbChange->getName() <<"\n";

				/*
				 * Step 1: Change header.
				 */
				//Verify if there is any successor is in mapFunc
				Instruction *header= &bbChange->front();
				//StoreInst *storID = new StoreInst(ConstantInt::get(Type::getInt64Ty(ctx), dataBB.signatureID), bbNumber, header);
				if(firstBB){
					tempSignature = new AllocaInst (IntegerType::get(ctx,64), 0, "signature_"+F->getName(), header);
					tempUpperD = new AllocaInst (IntegerType::get(ctx,64), 0, "upperD_"+F->getName(), header);
				}


				if(firstBB){
					Value *valSignature = ConstantInt::get(IntegerType::get(ctx,64), dataBB.signatureID);
					StoreInst *storeNew = new StoreInst(valSignature, signature, header);
				}else
				if(dataBB.hasMultPred){
					LoadInst *loadSignature = new LoadInst(signature, "load_sig", header);
					Value *lowerD = ConstantInt::get(IntegerType::get(ctx,64), dataBB.lowerd);
					Instruction* xorlowerD = BinaryOperator::CreateXor(loadSignature,lowerD, "sig_d_xor", header);
					LoadInst *loadUpperD = new LoadInst(upperD, "load_D", header);
					Instruction* xorUpperD = BinaryOperator::CreateXor(xorlowerD, loadUpperD, "sig_D_xor", header);
					StoreInst *storeNew = new StoreInst(xorUpperD, signature, header);
				}else{
					LoadInst *loadSignature = new LoadInst(signature, "load_sig", header);
					Value *lowerD = ConstantInt::get(IntegerType::get(ctx,64), dataBB.lowerd);
					Instruction* xorlowerD = BinaryOperator::CreateXor(loadSignature,lowerD, "sig_d_xor", header);
					StoreInst *storeNew = new StoreInst(xorlowerD, signature, header);

				}
				if(dataBB.upperD !=-1){
					StoreInst *storeNew = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD, header);
				}
				insertSignatureUpdateCall(true, ctx, F,bbChange, signature, upperD, tempSignature,tempUpperD, NULL, NULL);

				/*
				 *
				 * Step 2:Change tail
				 *
				 */
				BasicBlock *originalCondBranch = NULL;
				Value *expectedSig;

				expectedSig = ConstantInt::get(IntegerType::get(ctx,64), dataBB.signatureID);

				originalCondBranch = createOriginalBranch(F, bbChange, ctx, bbChange->getTerminator());
				if(originalCondBranch!=NULL){
					//Verify signature.
					Value *loadSignarure = builderbbChange.CreateLoad(signature, "load_sig");
					Value *compSig = builderbbChange.CreateICmpEQ(loadSignarure, expectedSig,"comp_sig");
					builderbbChange.CreateCondBr(compSig, originalCondBranch, bbErrorHandler);
				}else{
					errs()<<"   Instruction type not supported (2)! Module:"<<M.getModuleIdentifier() <<" Fun:"<< F->getName()<< "' BB:'"<<bbChange->getName() << "'\n";
					return false;
				}
				firstBB = false;

			}
		}
		return Changed;
	}

};

char CFCSS64::ID = 0;
static RegisterPass<CFCSS64> X ("CFCSS64","CFCSS64  - Control Flow Checking by Software Signatures 64 bits", false,false);

}

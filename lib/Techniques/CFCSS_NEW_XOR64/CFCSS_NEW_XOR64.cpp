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


struct CFCSS_NEW_XOR64: public ModulePass  {

	static char ID;

	Function *printf_func;
	Function *errHandler;

	mapFunction mapFunc;
	mapBB basicBlocks;

	GlobalVariable *signature1;
	GlobalVariable *signature2;
	GlobalVariable *signature3;
	GlobalVariable *signature4;
	GlobalVariable *signature5;
	GlobalVariable *signature6;
	GlobalVariable *signature7;
	GlobalVariable *signature8;
	GlobalVariable *signature9;

	GlobalVariable *upperD1;
	GlobalVariable *upperD2;
	GlobalVariable *upperD3;
	GlobalVariable *upperD4;
	GlobalVariable *upperD5;
	GlobalVariable *upperD6;
	GlobalVariable *upperD7;
	GlobalVariable *upperD8;
	GlobalVariable *upperD9;


	GlobalVariable *gl_check;
	GlobalVariable *bbNumber;

	CFCSS_NEW_XOR64() : ModulePass(ID) {

	}

	bool runOnModule(Module &M){
  		return module(M);
	}

	bool module(Module &M){
		bool Changed = false;


		LLVMContext &ctx = M.getContext();

		//Add a global variable called "Signature" with the initial value 0.
		signature1 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature1");
		M.getGlobalList().push_back(signature1);

		signature2 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature2");
		M.getGlobalList().push_back(signature2);

		signature3 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature3");
		M.getGlobalList().push_back(signature3);

		signature4 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature4");
		M.getGlobalList().push_back(signature4);

		signature5 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature5");
		M.getGlobalList().push_back(signature5);

		signature6 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature6");
		M.getGlobalList().push_back(signature6);

		signature7 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature7");
		M.getGlobalList().push_back(signature7);

		signature8 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature8");
		M.getGlobalList().push_back(signature8);

		signature9 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "signature9");
		M.getGlobalList().push_back(signature9);


		upperD1 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD1");
		M.getGlobalList().push_back(upperD1);

		upperD2 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD2");
		M.getGlobalList().push_back(upperD2);

		upperD3 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD3");
		M.getGlobalList().push_back(upperD3);

		upperD4 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD4");
		M.getGlobalList().push_back(upperD4);

		upperD5 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD5");
		M.getGlobalList().push_back(upperD5);

		upperD6 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD6");
		M.getGlobalList().push_back(upperD6);

		upperD7 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD7");
		M.getGlobalList().push_back(upperD7);

		upperD8 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD8");
		M.getGlobalList().push_back(upperD8);

		upperD9 = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "upperD9");
		M.getGlobalList().push_back(upperD9);


		gl_check = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "gl_check");
		M.getGlobalList().push_back(gl_check);

		bbNumber = new GlobalVariable(IntegerType::get(ctx,64), false, GlobalValue::ExternalLinkage, ConstantInt::get(IntegerType::get(ctx,64), 0), "bbNumber");
		M.getGlobalList().push_back(bbNumber);

		//Read file content.
		//string input = M.getModuleIdentifier();
		//int pos = input.find_last_of(".");
		//string input_SBL = input.substr(0,pos);
		//input_SBL = input_SBL + "_SBL.txt";
		//errs()<< input_SBL<<"\n";
		//if(!readFileContent(&mapFunc, input_SBL)){
		//	return false;
		//}

		//displayMapContent(mapFunc);
		generateBBIDs(&basicBlocks, M);

		calcDvalue(&basicBlocks);

		//displayBBsData(basicBlocks);
		defineSigNumber(&basicBlocks, M);

		//errs()<<"Signumber calculated!\n";
		displayBBsData(basicBlocks);
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
			//Put all basic blocks of the function into a vector.
			vector <BasicBlock *> functBBs;
			for (Function::iterator bbAuxVec = F->begin(); bbAuxVec != F->end(); bbAuxVec++){
				functBBs.push_back(bbAuxVec);
			}

			bool firstBB =  true;
			AllocaInst *tempSignature;
			AllocaInst *tempUpperD;
			BasicBlock *bbErrorHandler = NULL;
			BasicBlock *bbErroCompare = NULL;
			Value *strErrorCompare= NULL;
			for (vector<BasicBlock *>::iterator itVecBB = functBBs.begin(); itVecBB != functBBs.end(); itVecBB++){

				mapBB::iterator itBB = basicBlocks.find(*itVecBB);
				if(itBB ==basicBlocks.end()){
					BasicBlock *bbAux2 = *itVecBB;
					if(bbAux2->getName()!="bbErrorHandler"){
						errs() <<  bbAux2->getName() <<"\n";
						errs()<<"Error! Basic block not found!\n";
						return false;
					}
				}
				BasicBlock *bbChange = itBB->first;
				//Get extra data for current basic block.
				NodeExtraData dataBB  = itBB->second;
				IRBuilder<> builderbbChange(bbChange);

				/*
				 * Step 1: Change header.
				 */
				Instruction *header= &bbChange->front();
				//Alocate the variables for temp_signature
				if(firstBB){
					tempSignature = new AllocaInst (IntegerType::get(ctx,64), 0, "signature_"+F->getName(), header);
					tempUpperD = new AllocaInst (IntegerType::get(ctx,64), 0, "upperD_"+F->getName(), header);
				}

				//Load the signature value
				Value *expectedSig;
				LoadInst *loadSignature;
				StoreInst *storeSignature;
				if(firstBB){
					Value *valSignature = ConstantInt::get(IntegerType::get(ctx,64), dataBB.signatureID);
					StoreInst *storeNew = new StoreInst(valSignature, signature1, header);
					expectedSig = ConstantInt::get(IntegerType::get(ctx,64), dataBB.signatureID);
				}else
				if(dataBB.hasMultPred){
					LoadInst *loadUpperD;
					Instruction* xorlowerD;
					Instruction* xorUpperD;
					Value *lowerD = ConstantInt::get(IntegerType::get(ctx,64), dataBB.lowerd);
					switch (dataBB.sigNumber){
						case 1 : loadSignature = new LoadInst(signature1, "load_sig", header); break;
						case 2 : loadSignature = new LoadInst(signature2, "load_sig", header); break;
						case 3 : loadSignature = new LoadInst(signature3, "load_sig", header); break;
						case 4 : loadSignature = new LoadInst(signature4, "load_sig", header); break;
						case 5 : loadSignature = new LoadInst(signature5, "load_sig", header); break;
						case 6 : loadSignature = new LoadInst(signature6, "load_sig", header); break;
						case 7 : loadSignature = new LoadInst(signature7, "load_sig", header); break;
						case 8 : loadSignature = new LoadInst(signature8, "load_sig", header); break;
						case 9 : loadSignature = new LoadInst(signature9, "load_sig", header); break;
					};
					switch (dataBB.sigNumber){
						case 1 : loadUpperD = new LoadInst(upperD1, "load_D1", header); break;
						case 2 : loadUpperD = new LoadInst(upperD2, "load_D2", header); break;
						case 3 : loadUpperD = new LoadInst(upperD3, "load_D3", header); break;
						case 4 : loadUpperD = new LoadInst(upperD4, "load_D4", header); break;
						case 5 : loadUpperD = new LoadInst(upperD5, "load_D5", header); break;
						case 6 : loadUpperD = new LoadInst(upperD6, "load_D6", header); break;
						case 7 : loadUpperD = new LoadInst(upperD7, "load_D7", header); break;
						case 8 : loadUpperD = new LoadInst(upperD8, "load_D8", header); break;
						case 9 : loadUpperD = new LoadInst(upperD9, "load_D9", header); break;
					};
					xorlowerD = BinaryOperator::CreateXor(loadSignature,lowerD, "sig_d_xor", header);
					xorUpperD = BinaryOperator::CreateXor(xorlowerD, loadUpperD, "sig_D_xor", header);
					switch (dataBB.sigNumber){
						case 1 : storeSignature= new StoreInst(xorUpperD, signature1, header); break;
						case 2 : storeSignature= new StoreInst(xorUpperD, signature2, header); break;
						case 3 : storeSignature= new StoreInst(xorUpperD, signature3, header); break;
						case 4 : storeSignature= new StoreInst(xorUpperD, signature4, header); break;
						case 5 : storeSignature= new StoreInst(xorUpperD, signature5, header); break;
						case 6 : storeSignature= new StoreInst(xorUpperD, signature6, header); break;
						case 7 : storeSignature= new StoreInst(xorUpperD, signature7, header); break;
						case 8 : storeSignature= new StoreInst(xorUpperD, signature8, header); break;
						case 9 : storeSignature= new StoreInst(xorUpperD, signature9, header); break;
					};
					expectedSig = ConstantInt::get(IntegerType::get(ctx,64), dataBB.signatureID);
				}else{
					expectedSig = ConstantInt::get(IntegerType::get(ctx,64), dataBB.predID);
					//errs() <<" Option C\n";
				}

				switch (dataBB.sigNumber){
					case 1 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature1, upperD1, tempSignature,tempUpperD, NULL, NULL); break;
					case 2 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature2, upperD2, tempSignature,tempUpperD, NULL, NULL); break;
					case 3 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature3, upperD3, tempSignature,tempUpperD, NULL, NULL); break;
					case 4 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature4, upperD4, tempSignature,tempUpperD, NULL, NULL); break;
					case 5 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature5, upperD5, tempSignature,tempUpperD, NULL, NULL); break;
					case 6 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature6, upperD6, tempSignature,tempUpperD, NULL, NULL); break;
					case 7 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature7, upperD7, tempSignature,tempUpperD, NULL, NULL); break;
					case 8 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature8, upperD8, tempSignature,tempUpperD, NULL, NULL); break;
					case 9 : insertSignatureUpdateCall(true, ctx, F,bbChange, signature9, upperD9, tempSignature,tempUpperD, NULL, NULL); break;
				};

				/*
				 *
				 * Step 2:Change tail
				 *
				 */
				bool unreachable = false;
				Instruction *tail;
				if (dyn_cast<UnreachableInst>(bbChange->getTerminator())){//if (UnreachableInst *UI = dyn_cast<UnreachableInst>(origInstr)){
					tail = bbChange->getTerminator()->getPrevNode();
				}else{
					tail= bbChange->getTerminator();
				}

				//errs()<<"Function: "<<F->getName() <<"  BB Name: "<<bbChange->getName() <<"\n";
				//verify successor SigId.
				int sigSuccAux=0,sigSuccAnt=0;
				for (succ_iterator SC = succ_begin(bbChange); SC != succ_end(bbChange); ++SC) {
						BasicBlock * succ = *SC;
						//errs()<<"succ Function: "<<succ->getParent()->getName() <<"  Succ BB Name: "<<succ->getName() <<"\n";
						mapBB::iterator itSucc = basicBlocks.find(succ);
						NodeExtraData dataSucc = itSucc->second;
						sigSuccAux = dataSucc.sigNumber;
						if(sigSuccAnt!=0 && sigSuccAnt != sigSuccAux){
							errs()<<"Error: sigSuccAnt successor differ.\n";
							return -1;
						}
						sigSuccAnt = sigSuccAux;
				}
				//Store the values of the signature for the next basic block
				Value *signatureID = ConstantInt::get(IntegerType::get(ctx,64), dataBB.signatureID);
				switch (sigSuccAux){
					case 1 : storeSignature= new StoreInst(signatureID, signature1, tail); break;
					case 2 : storeSignature= new StoreInst(signatureID, signature2, tail); break;
					case 3 : storeSignature= new StoreInst(signatureID, signature3, tail); break;
					case 4 : storeSignature= new StoreInst(signatureID, signature4, tail); break;
					case 5 : storeSignature= new StoreInst(signatureID, signature5, tail); break;
					case 6 : storeSignature= new StoreInst(signatureID, signature6, tail); break;
					case 7 : storeSignature= new StoreInst(signatureID, signature7, tail); break;
					case 8 : storeSignature= new StoreInst(signatureID, signature8, tail); break;
					case 9 : storeSignature= new StoreInst(signatureID, signature9, tail); break;
				};
				if(dataBB.upperD !=-1){
					StoreInst *storeUpperD;
					switch (sigSuccAux){
						case 1 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD1, tail); break;
						case 2 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD2, tail); break;
						case 3 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD3, tail); break;
						case 4 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD4, tail); break;
						case 5 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD5, tail); break;
						case 6 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD6, tail); break;
						case 7 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD7, tail); break;
						case 8 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD8, tail); break;
						case 9 : storeUpperD = new StoreInst(ConstantInt::get(IntegerType::get(ctx,64), dataBB.upperD), upperD9, tail); break;
					};
				}

				//Load the signature value
				loadSignature= NULL;
				switch (dataBB.sigNumber){
					case 1 : loadSignature = new LoadInst(signature1, "load_sig", tail); break;
					case 2 : loadSignature = new LoadInst(signature2, "load_sig", tail); break;
					case 3 : loadSignature = new LoadInst(signature3, "load_sig", tail); break;
					case 4 : loadSignature = new LoadInst(signature4, "load_sig", tail); break;
					case 5 : loadSignature = new LoadInst(signature5, "load_sig", tail); break;
					case 6 : loadSignature = new LoadInst(signature6, "load_sig", tail); break;
					case 7 : loadSignature = new LoadInst(signature7, "load_sig", tail); break;
					case 8 : loadSignature = new LoadInst(signature8, "load_sig", tail); break;
					case 9 : loadSignature = new LoadInst(signature9, "load_sig", tail); break;
				};

				//Check the values of the global check agains the local signatues.
				LoadInst *loadCheck = new LoadInst(gl_check, "load_check", tail);
				Instruction *xorExpectSig;
				if(dataBB.hasMultPred)
					xorExpectSig = BinaryOperator:: CreateXor(loadSignature,expectedSig, "sig_d_xor", tail);
				else
					xorExpectSig = BinaryOperator:: CreateXor(loadSignature,expectedSig, "sig_d_xor", tail);

				xorExpectSig = BinaryOperator:: CreateXor(xorExpectSig,loadCheck, "sig_d_xor", tail);
				StoreInst *storeNew = new StoreInst(xorExpectSig, gl_check, tail);

				if(F->getName() == "main" && (hasExitOrReturAndPred(bbChange))){
					//errs()<<"Has exit\n";
					//errs()<< "BB name:" <<bbChange->getName()<<"\n";
					BasicBlock * originalCondBranch = createOriginalBranch(F, bbChange, ctx, bbChange->getTerminator());
					if(bbErrorHandler == NULL){
						//Create block into function to jump to error handler.
						bbErrorHandler = BasicBlock::Create(ctx, "bbErrorHandler", F);
						IRBuilder<> buildError(bbErrorHandler);
						buildError.CreateCall(errHandler,ConstantInt::get(IntegerType::get(ctx,64), 0));
						buildError.CreateUnreachable();
					}

					//Create block into function to jump to error handler.
					bbErroCompare = BasicBlock::Create(ctx, "bbErroCompare", F);
					IRBuilder<> buildCompare(bbErroCompare);
					Constant *print = createFunctionPrintf(M, buildCompare);
					if(strErrorCompare == NULL){
						strErrorCompare = buildCompare.CreateGlobalStringPtr("Error compare\n", "strComp");
					}
				    buildCompare.CreateCall(print, strErrorCompare);

					Value *loadCheck = buildCompare.CreateLoad(gl_check, "load_check");
					Value *expectedCheck = ConstantInt::get(IntegerType::get(ctx,64), 0);
					Value *compSig = buildCompare.CreateICmpEQ(loadCheck, expectedCheck,"comp_sig");
					buildCompare.CreateCondBr(compSig, originalCondBranch, bbErrorHandler);

					if(originalCondBranch!=NULL){
						//Verify signature.
						builderbbChange.CreateBr(bbErroCompare);
					}else{
						errs()<<"   Instruction type not supported (2)! Module:"<<M.getModuleIdentifier() <<" Fun:"<< F->getName()<< "' BB:'"<<bbChange->getName() << "'\n";
						return false;
					}
				}
				firstBB = false;
			}
		}
		return Changed;
	}

};

char CFCSS_NEW_XOR64::ID = 0;
static RegisterPass<CFCSS_NEW_XOR64> X ("CFCSS_NEW_XOR64","CFCSS_NEW_XOR64 - New Control Flow Checking by Software Signatures (modified with XOR) with 64 bits", false,false);

}

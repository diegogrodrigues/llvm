/*
 * Utils.h
 *
 *  Created on: Dec 9, 2013
 *      Author: diego
 */

#ifndef UTILS_H_
#define UTILS_H_

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
#include <fstream>
#include <vector>
#include <map>


using namespace llvm;
using namespace std;


struct NodeExtraData {
	int signatureID;
	int predID;
	int lowerd;
	int upperD;
	bool hasMultPred;
};


typedef vector<string> vecFileBB;
typedef map<string, vecFileBB> mapFunction;
typedef map<BasicBlock*, NodeExtraData> mapBB;


// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

static Constant* createFunctionPrintf(Module &M, IRBuilder<> builder){
	std::vector<llvm::Type *> printfArgs;
	printfArgs.push_back(builder.getInt8Ty()->getPointerTo());
	llvm::ArrayRef<llvm::Type*> argsRef(printfArgs);
	FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), argsRef, true);
	Constant *printfFunc = M.getOrInsertFunction("printf", printfType);
	return printfFunc;
}

static Function* printf_prototype(LLVMContext& ctx, Module *mod)
{
    std::vector<Type*> printf_arg_types;
    printf_arg_types.push_back(Type::getInt8PtrTy(ctx));

    FunctionType* printf_type = FunctionType::get(  Type::getInt32Ty(ctx), printf_arg_types, true);

    Function *func = Function::Create( printf_type, Function::ExternalLinkage, Twine("printf"), mod );
    func->setCallingConv(CallingConv::C);
    return func;
}

static Constant* createFunctionExit(Module &M){

	Constant *cteExit = M.getOrInsertFunction ("exit",Type::getVoidTy(M.getContext()), IntegerType::get(M.getContext(),32),NULL);
	Function *exit = cast<Function>(cteExit);
	exit->addAttribute(~0, Attribute::NoReturn);
	exit->addAttribute(~0, Attribute::NoUnwind);

	return cteExit;
}

static Function *CreateErrorHandlerFunction(Module &M, LLVMContext &Context) {

	Function *handlerF = cast<Function>(M.getOrInsertFunction("ErrorHandler", Type::getInt32Ty(Context), Type::getInt32Ty(Context), (Type *)0));

	Attribute attr = Attribute::get(M.getContext(),"error-handler");
	AttrBuilder NewAttributes;
    NewAttributes.addAttribute(attr);
    const AttributeSet &NPAL = AttributeSet::get(M.getContext(), AttributeSet::FunctionIndex, NewAttributes);
    handlerF->addAttributes(AttributeSet::FunctionIndex, NPAL);

	// Add a basic block to the function.
	BasicBlock *entry = BasicBlock::Create(Context, "entry", handlerF);
	IRBuilder<> builder(entry);

    Constant *printfFunc = createFunctionPrintf(M, builder);

    Value *strUnhandledError = builder.CreateGlobalStringPtr("Error detected\n", "strError");
    builder.CreateCall(printfFunc, strUnhandledError);

    Constant *fExit = createFunctionExit(M);
    builder.CreateCall(fExit, ConstantInt::get(IntegerType::get(Context,32), 0));
    builder.CreateUnreachable();

	return handlerF;
}


bool callingModuleFunction(Function *F, CallInst *call){

	//Module *M = F->getParent();

	int npar = call->getNumOperands();
	//errs()<< "(2) Function:"<< call->getParent()->getParent()->getName() <<"  BB Name:"<<call->getParent()->getName()<<"  Operand:ZERO\n";
	if(npar==0)
		return false;

	for(int i=0; i< npar;i++){
		if(	Function* CI = dyn_cast<Function>(call->getOperand(i))){
			//errs()<< "Function:"<< call->getParent()->getParent()->getName() <<"  BB Name:"<<call->getParent()->getName()<<"  Operand:" << call->getOperand(i)->getName() << "\n";
			if(CI!=F && !CI->isDeclaration()){
				//errs()<< "#> found call of function:"<<call->getOperand(i)->getName() << "\n";
				return true;
			}
		}
	}

	return false;
}

void insertNops(LLVMContext& ctx,Instruction *I, int number){

	NopInst *nop;
	for(int i=0; i < number; i++){
		nop = new NopInst(ctx, I);
	}
}

bool insertSignatureUpdateCall(bool inserted_for_nop,LLVMContext &ctx, Function *F, BasicBlock *bb, GlobalVariable *signature, GlobalVariable *upperD, AllocaInst *tempSignature, AllocaInst *tempUpperD, GlobalVariable *bbNumber, AllocaInst *tempbbNumber){
	//Find a call
	for(BasicBlock::iterator itBB=bb->begin(); itBB !=bb->end(); itBB++){
		Instruction *inst = itBB;
		if (CallInst *call = dyn_cast<CallInst>(inst)){
			Function *fcall = call->getCalledFunction();
			if(fcall !=0){
				if(!fcall->isDeclaration() || callingModuleFunction(F,call)){
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
					//errs() << "Next instruction(2):" <<nextInst->getName() <<"\n";
					//errs() <<"Found a call:"<< fcall->getName()<<"\n";
					if(inserted_for_nop){
						LoadInst *loadSignature = new LoadInst(signature, "load_sig", call);	//8 nops
						LoadInst *loadUpperD = new LoadInst(upperD, "load_D", call);  			//8 nops
						StoreInst *storeSig = new StoreInst(loadSignature, tempSignature, call); //4 nops
						StoreInst *storeUpperd = new StoreInst(loadUpperD, tempUpperD, call);	  //4 nops
						/*if (bbNumber!=NULL){
							LoadInst *loadbbNumber = new LoadInst(bbNumber, "load_bbNumber", call);
							StoreInst *storebbNumber = new StoreInst(loadbbNumber, tempbbNumber, call);
						}*/
						LoadInst *loadFunSignature = new LoadInst(tempSignature, "load_temp_sig", nextInst);	//3 nops
						LoadInst *loadFunUpperD = new LoadInst(tempUpperD, "load_temp_D", nextInst);			//4 nops
						StoreInst *storeFuncSig = new StoreInst(loadFunSignature, signature, nextInst);			//7 nops
						StoreInst *storeFuncUpperD = new StoreInst(loadFunUpperD, upperD, nextInst);			//8 nops
						/*if (bbNumber!=NULL){
							LoadInst *loadtempbbNumber = new LoadInst(tempbbNumber, "load_temp_bbNumber", nextInst);
							StoreInst *storetempbbNumber  = new StoreInst(loadtempbbNumber, bbNumber, nextInst);
						}*/
					}else{
						insertNops(ctx, call, 20);
						insertNops(ctx, nextInst, 20);
					}
				}
			}
		}
	}
	return true;
}


void generateBBIDs(mapBB *basicBlocks, Module &M){
	int ID =1;
	//Steps through each module function.
	for(Module::iterator funAux = M.begin(); funAux != M.end(); funAux++){
		if((!funAux->isDeclaration())&&(funAux->getName()!="ErrorHandler")){
			for (Function::iterator bbAux = funAux->begin(); bbAux != funAux->end(); bbAux++){
				BasicBlock *bb = bbAux;
				NodeExtraData nd;
				nd.signatureID = ID;
				nd.predID = -1;
				nd.lowerd = 0;
				nd.upperD = -1;
				nd.hasMultPred = false;
				basicBlocks->insert( std::pair<BasicBlock*, NodeExtraData>(bb, nd) );
				ID++;
			}
		}
	}
}

void displayBBsData(mapBB basicBlocks){
	for (mapBB::iterator it=basicBlocks.begin(); it!=basicBlocks.end(); ++it){
		BasicBlock * bb = it->first;
		string bbName = bb->getName();

		Function *f =bb->getParent();
		string funName = f->getName();

		while(bbName.size() <20){
			bbName = bbName + " ";
		}
		bbName =bbName.substr(0,20);
		while(funName.size() <15){
			funName = funName + " ";
		}

		NodeExtraData nd = it->second;
		outs() <<"Fun Name: " << funName;
		outs() <<"\tBB Name: " << bbName;
		outs() <<"\tsignatureID:"<< nd.signatureID <<"\t d value:"<<nd.lowerd<<"\t D value:"<<nd.upperD<<"\t pred ID: "<<nd.predID<<"\t Multiple pred:"<<(nd.hasMultPred?"True":"False")<<"\n";
	}
}

void displayMapContent(mapFunction map){
	int i =1;
	outs()<<"\ndisplayContent\n";
	for (mapFunction::iterator it=map.begin(); it!=map.end(); ++it){
		//outs()<<it->first << "\n";
		vecFileBB vector = it->second;
		for (vecFileBB::iterator itV=vector.begin(); itV!=vector.end(); ++itV){
			//string value = *itV;
			outs()<< it->first  << ":"<< *itV <<","<<i++<<"\n";
		}
	}
}

BasicBlock * createOriginalBranch(Function *F, BasicBlock *bb, LLVMContext& ctx, Instruction *origInstr){

	string name;
	Instruction *LastInstruction;
	bool unreachable = false;
	if (dyn_cast<BranchInst>(origInstr)){//if (BranchInst *BI = dyn_cast<BranchInst>(origInstr)){
		name = "origBranch";
	}else
	if (dyn_cast<ReturnInst>(origInstr)){//if (ReturnInst *RI = dyn_cast<ReturnInst>(origInstr)){
		name = "origReturn";
	}else
	if (dyn_cast<UnreachableInst>(origInstr)){//if (UnreachableInst *UI = dyn_cast<UnreachableInst>(origInstr)){
		name = "origCall";
		//Erase UnreachableInst
		BasicBlock *bbAux = origInstr->getParent();
		bbAux->getTerminator()->eraseFromParent();
		for(BasicBlock::iterator itBB=bbAux->begin(); itBB !=bbAux->end(); itBB++){
			LastInstruction = itBB;
		}
		unreachable = true;
	}else
	if (dyn_cast<SwitchInst>(origInstr)){
		name = "origSwitch";
	}else{
		return NULL;
	}
	//Create a basic block to receive the original branch data.
	BasicBlock *originalBranch = BasicBlock::Create(ctx, name, F);
	IRBuilder<> builderOriginal(originalBranch);

	if(unreachable){
		Instruction *I = LastInstruction->getParent()->getInstList().remove(LastInstruction);
		builderOriginal.Insert(I,"");
		builderOriginal.CreateUnreachable();
	}else{
		//Move branch from a BB to another.
		Instruction *I = origInstr->getParent()->getInstList().remove(origInstr);
		builderOriginal.Insert(I,"");
	}
	return originalBranch;
}

BasicBlock * createOriginalBranchBeforeNextBB(Function *F, BasicBlock *bb, LLVMContext& ctx, Instruction *origInstr){

	string name;
	Instruction *LastInstruction;
	bool unreachable = false;
	if (dyn_cast<BranchInst>(origInstr)){//if (BranchInst *BI = dyn_cast<BranchInst>(origInstr)){
		name = "origBranch";
	}else
	if (dyn_cast<ReturnInst>(origInstr)){//if (ReturnInst *RI = dyn_cast<ReturnInst>(origInstr)){
		name = "origReturn";
	}else
	if (dyn_cast<UnreachableInst>(origInstr)){//if (UnreachableInst *UI = dyn_cast<UnreachableInst>(origInstr)){
		name = "origCall";
		//Erase UnreachableInst
		BasicBlock *bbAux = origInstr->getParent();
		bbAux->getTerminator()->eraseFromParent();
		for(BasicBlock::iterator itBB=bbAux->begin(); itBB !=bbAux->end(); itBB++){
			LastInstruction = itBB;
		}
		unreachable = true;
	}else
	if (dyn_cast<SwitchInst>(origInstr)){
		name = "origSwitch";
	}else{
		return NULL;
	}
	//Create a basic block to receive the original branch data.
	//OBS: It will be created between the original basic block and its next basic block. (perfect! ;D ;D)
	BasicBlock *originalBranch = BasicBlock::Create(ctx, name, F, bb->getNextNode());
	IRBuilder<> builderOriginal(originalBranch);

	if(unreachable){
		Instruction *I = LastInstruction->getParent()->getInstList().remove(LastInstruction);
		builderOriginal.Insert(I,"");
		builderOriginal.CreateUnreachable();
	}else{
		//Move branch from a BB to another.
		Instruction *I = origInstr->getParent()->getInstList().remove(origInstr);
		builderOriginal.Insert(I,"");
	}
	return originalBranch;
}

bool readFileContent(mapFunction *map, string location){

	//Read the file contet and put it in the map
	ifstream myfile (location.c_str());
	string line;
	string funPred = "";
	string funName = "";
	string bbName  = "";
	string tmp1    = "";
	vecFileBB vectorBBs;

	if (myfile.is_open()){
		int pos;
		vectorBBs.clear();
		while ( getline (myfile,line) )
		{
			line = trim(line);
			//errs()<<"T:'"<<line<<"'\n";
			pos = line.find_first_of(':');
			funName = line.substr(0, pos);

			tmp1 = line.substr(pos+1, line.size());
			pos = tmp1.find_first_of(':');
			bbName = tmp1.substr(0, pos);
			//cout <<"BbName:"<<bbName << endl << endl;
			if(funPred == "" || funPred == funName){
				//cout <<"Function Name:"<<funName << endl;
				vectorBBs.push_back(bbName);
			}else{
				//errs()<<"funPred:"<<funPred<< "\n";
				map->insert( std::pair<string, vecFileBB>(funPred, vectorBBs) );
				vectorBBs.clear();
				vectorBBs.push_back(bbName);

			}
			funPred = funName;
		}
		myfile.close();//
		//cout <<"Function Name:"<<funName << endl;
		//errs()<<"funPred:"<<funPred<< "\n";
		map->insert( std::pair<string, vecFileBB>(funPred, vectorBBs) );
	}else{
		errs() << "File no found:"<<location <<"\n";
		return false;
	}
	return true;
}

bool calcDvalue(mapBB *bbIds){

	int firstPredID;

	for (mapBB::iterator itBB= bbIds->begin(); itBB!=bbIds->end(); ++itBB){
		BasicBlock *bb = itBB->first;
		NodeExtraData dataBB = itBB->second;


		BasicBlock * pred = bb->getSinglePredecessor();
		if(pred != NULL){
			//errs() <<"(single) Function:"<<bb->getParent()->getName() <<"  BB Name:"<< bb->getName() << "\n";
			mapBB::iterator itPred = bbIds->find(pred);
			if (itPred == bbIds->end()){
				return false;
			}
			NodeExtraData dataPred = itPred->second;
			dataBB.lowerd = dataBB.signatureID ^ dataPred.signatureID;
			dataBB.predID = dataPred.signatureID;
			itBB->second = dataBB;
			continue;
		}

		bool first = true;
		//errs() <<"(multiple/zero)Function:"<<bb->getParent()->getName() <<"  BB Name:"<< bb->getName() << "\n";
		int predCount = 0;
		//Iterate over BasickBlocks predecessors.
		for (pred_iterator PD = pred_begin(bb), EA = pred_end(bb); PD != EA; ++PD) {
			predCount++;
			BasicBlock *pred = *PD;
			//errs() << "  Pred: " << pred->getName();
			//search for pred ID in map.
			mapBB::iterator itPred = bbIds->find(pred);
			if (itPred == bbIds->end()){
				return false;
			}
			NodeExtraData dataPred = itPred->second;

			if(first){
				first = false;
				firstPredID = dataPred.signatureID;
				dataPred.upperD = 0;
			}else{
				dataPred.upperD = dataPred.signatureID ^ firstPredID;
			}
			itPred->second = dataPred;
			//errs() << "  ID Pred: " << dataPred.predID << "  D value:" <<dataPred.upperD << "\n";
		}

		if(predCount == 0){
			dataBB.lowerd = dataBB.signatureID;
			dataBB.predID = -1;
		}else
		if(predCount>=1){
			dataBB.lowerd = dataBB.signatureID ^ firstPredID;
			//Basic Block has many predecessors.
			if(predCount>1){
				dataBB.hasMultPred = true;
			}else{
				dataBB.predID = firstPredID;
			}
		}
		itBB->second = dataBB;

	}
	return true;
}

bool isFunInMap(mapFunction map, string funName){
	mapFunction::iterator it = map.find(funName);//verify if the function is in the map.
	if ( it == map.end())
		return false;
	return true;
}

bool isInMap(mapFunction map, string funName, string bbName){
	mapFunction::iterator it = map.find(funName);//verify if the function is in the map.
	if ( it == map.end())
		return false;

	vecFileBB vec = it->second;//verify if the basic block is in the funtion's vector.
	vecFileBB::iterator itVec =  find (vec.begin(), vec.end(), bbName);
	if (itVec == vec.end())
		return false;

	return true;
}

bool isPredInMap(mapFunction map, string funName, BasicBlock * bb){

	mapFunction::iterator it = map.find(funName);//verify if the function is in the map.
	if (it == map.end())
		return false;

	vecFileBB vec = it->second;//verify if the basic block is in the funtion's vector.

	for (pred_iterator PD = pred_begin(bb); PD != pred_end(bb); ++PD) {
		BasicBlock * succ = *PD;
		vecFileBB::iterator itVec =  find (vec.begin(), vec.end(), succ->getName());

		//If at least one successor is found, return true.
		if (itVec != vec.end())
			return true;
	}
	return false;
}

bool isSuccInMap(mapFunction map, string funName, BasicBlock * bb){

	mapFunction::iterator it = map.find(funName);//verify if the function is in the map.
	if (it == map.end())
		return false;

	vecFileBB vec = it->second;//verify if the basic block is in the funtion's vector.

	for (succ_iterator SC = succ_begin(bb); SC != succ_end(bb); ++SC) {
		BasicBlock * succ = *SC;
		vecFileBB::iterator itVec =  find (vec.begin(), vec.end(), succ->getName());

		//If at least one successor is found, return true.
		if (itVec != vec.end())
			return true;
	}
	return false;
}

BasicBlock * lastBB(Function *F)
{
	BasicBlock *bb;

	return bb;
}

bool hasExitOrReturAndPred(BasicBlock* bb){

	int count =0;
	for (pred_iterator PD = pred_begin(bb), EA = pred_end(bb); PD != EA; ++PD) {
		count++;
	}
	//if (count==0)
		//return false;

	if (dyn_cast<ReturnInst>(bb->getTerminator())){//if (ReturnInst *RI = dyn_cast<ReturnInst>(origInstr)){

		return true;
	}

	if (dyn_cast<UnreachableInst>(bb->getTerminator())){//if (UnreachableInst *UI = dyn_cast<UnreachableInst>(origInstr)){
			return true;
	}
	return false;
}


#endif /* UTILS_H_ */

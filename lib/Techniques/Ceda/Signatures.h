/*
 * Signature.h
 *
 *  Created on: Nov 12, 2013
 *      Author: diego
 */
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "Node.h"
#include "Util.h"
#include <vector>
#include <set>

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#define EMPTY_SIGNATURE -1

using namespace std;
using namespace llvm;

typedef set<Node*> SetNode;

namespace SignaturesSpace {

class Signatures {

private:

	vector<SetNode> nets;
	vector<SetNode> nets_pred;
	vector<SetNode> A_sig_NS;
	vector<SetNode> A_sig_NES;

	__uint64_t valueUpperCounter;
	__uint64_t valueLowerCounter;


	void insertInNet(Node *a, Node * b);
	bool haveCommonPred(Node *a,Node *b);
	bool isInNetPred(Node* node,int netIndex);
	void createNet(vector<Node*> vNodes);
	void createNetPred(vector<Node*> vNodes);
	void createA_Sig(vector<Node*> vNodes);

	bool calcUpperValues(vector<Node*> vNodes);
	bool calcLowValues(vector<Node*> vNodes);
	vector<SetNode> getCommonPredsOfSuccX(SetNode nodes);

	bool getZlPred(Node *node, set<__uint64_t> *result);
	bool calcSignatures(vector<Node*> vNodes);
	bool calcd1d2(vector<Node*> vNodes);
	bool isSetNodeInVecNode(vector<SetNode> vec, SetNode setSearch, int &pos);

	string iniValUpper;
	string iniValLower;
	string iniOnes;
	string iniZeros;

public:
	Signatures();
	void createSets(vector<Node*> vNodes);
	bool calcSignatureValues(vector<Node*> vNodes);
	void printNets();
	void printNetPreds();
	void printA_Sig();
	bool verifySignaturesD1(vector<Node*> vNodes);
	bool verifySignaturesD2(vector<Node*> vNodes);

	void dummy();

};

} /* namespace Ceda */

#endif /* SIGNATURE_H_ */

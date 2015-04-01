/*
 * Node.h
 *
 *  Created on: Nov 11, 2013
 *      Author: diego
 */
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <set>
#include "Util.h"


#ifndef NODE_H_
#define NODE_H_

using namespace std;
using namespace llvm;
using namespace util;

//Define the basic block type.
enum NodeType {typeX, typeA };

typedef set <BasicBlock*> SetBB;


class Node {


private:
	/*OBS: Value holds an unique ID who identifies each basic block */

	BasicBlock *node;   //Node Basic Block
	NodeType type;      //node type
	__uint64_t nodeId;//node id

	__uint64_t upperNodeSignatue;    //NSu
	__uint64_t upperNodeExitSignatue;//NSEu

	__uint64_t lowerNodeSignatue;      //NSl
	__uint64_t lowerNodeExitSignatue;  //NSEl

	__uint64_t nodeSignature;
	__uint64_t nodeExitSignature;

	__uint64_t d1Value;
	__uint64_t d2Value;


	void definePreds(vector<Node*> vNodes);
	void defineSuccs(vector<Node*> vNodes);

	set<Node*> preds;
	set<Node*> succs;


public:
	Node();

	void setId(__uint64_t id);
	__uint64_t getId();
	void setBB (BasicBlock *BB);
	BasicBlock* getBB();
	void setType (NodeType type);
	NodeType getType();
	string getTypeStr();
	void setUpperNS(__uint64_t);
	__uint64_t getUpperNS();
	void setUpperNES(__uint64_t);
	__uint64_t getUpperNES();
	void setLowerNS (__uint64_t value);
	__uint64_t getLowerNS ();
	void setLowerNES (__uint64_t value);
	__uint64_t getLowerNES ();
	set<Node*> getPreds();
	set<Node*> getSuccs();

	void setSignature(__uint64_t);
	__uint64_t getSignature();

	void setExitSignature(__uint64_t);
	__uint64_t getExitSignature();

	void setd1Value(__uint64_t);
	__uint64_t getd1Value();

	void setd2Value(__uint64_t);
	__uint64_t getd2Value();

	//void setPreds(BasicBlock *BB);
	void printId();

	static NodeType defineNodeType(BasicBlock *BB);
	static vector<Node*> defineNodesInformation(Module &M);
	static void printNodes(vector<Node*> vNodes);
	static void printLowerSignatures(vector<Node*> vNodes);

	string print(vector<__uint64_t>);

	Node* getNode(vector<Node*> vNodes, BasicBlock *BB);


};


#endif /* NODE_H_ */

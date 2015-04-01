/*
 * Node.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: diego
 */

#include "Node.h"


Node::Node() {
	// TODO Auto-generated constructor stub

	upperNodeSignatue = -1;
	upperNodeExitSignatue = -1;
	lowerNodeSignatue = -1;
	lowerNodeExitSignatue = -1;
	nodeSignature = -1;
	nodeExitSignature = -1;
	d1Value = -1;
	d2Value = -1;

}

/*
 * Set section
 */
void Node::setId(__uint64_t id){
	this->nodeId =id;
}

void Node::setBB (BasicBlock *BB){
	node = BB;
}

void Node::setType (NodeType type){
	this->type = type;
}
void Node::setUpperNS (__uint64_t value){
	this->upperNodeSignatue = value;
}
void Node::setUpperNES (__uint64_t value){
	this->upperNodeExitSignatue = value;
}
void Node::setLowerNS (__uint64_t value){
	this->lowerNodeSignatue = value;
}
void Node::setLowerNES (__uint64_t value){
	this->lowerNodeExitSignatue = value;
}
void Node::setSignature (__uint64_t value){
	this->nodeSignature = value;
}
void Node::setExitSignature (__uint64_t value){
	this->nodeExitSignature = value;
}
void Node::setd1Value (__uint64_t value){
	this->d1Value = value;
}
void Node::setd2Value (__uint64_t value){
	this->d2Value = value;
}

/*
 * Get section
 */
__uint64_t Node::getId(){
	return this->nodeId;
}

NodeType Node::getType(){
	return this->type;
}
string Node::getTypeStr(){

	if(this->type == typeX)
		return "X";
	if(this->type == typeA)
		return "A";
	return "Undefined!";

}

BasicBlock* Node::getBB(){
	return this->node;
}
__uint64_t Node::getUpperNS(){
	return this->upperNodeSignatue;
}
__uint64_t Node::getUpperNES(){
	return this->upperNodeExitSignatue;
}
__uint64_t Node::getLowerNS(){
	return this->lowerNodeSignatue;
}
__uint64_t Node::getLowerNES(){
	return this->lowerNodeExitSignatue;
}
__uint64_t Node::getSignature(){
	return this->nodeSignature;
}
__uint64_t Node::getExitSignature(){
	return this->nodeExitSignature;
}
__uint64_t Node::getd1Value(){
	return this->d1Value;
}
__uint64_t Node::getd2Value(){
	return this->d2Value;
}

set<Node*> Node::getPreds(){
	return preds;
}
set<Node*> Node::getSuccs(){
	return succs;
}

/*
 * define section
 */

void Node::definePreds(vector<Node*> vNodes){

	BasicBlock *BB =this->getBB();
	for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
		BasicBlock *predBB = *PI;
		for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
			Node* node=*it;
			if(node->getBB() == predBB)
				preds.insert(node);
		}
	}
}

void Node::defineSuccs(vector<Node*> vNodes){

	BasicBlock *BB =this->getBB();
	for (succ_iterator SI = succ_begin(BB), E = succ_end(BB); SI != E; ++SI) {
		BasicBlock *succBB = *SI;
		for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
			Node* node=*it;
			//outs()<< "  Name: "<<succBB->getName()<<"  Value Name:"<<succBB->getValueName()<<"\n";
			if(node->getBB() == succBB)
				succs.insert(node);
		}
	}
}
/*Get all informations of the nodes in a module.
 * The informations are:
 * -
 */
vector<Node*> Node::defineNodesInformation(Module &M){

	__uint64_t nodeId=0;
	vector<Node*> vNodes;

	//Steps through each module function.
	for(Module::iterator funAux = M.begin(); funAux != M.end(); funAux++){
		//Steps through each function Basic Block
		for (Function::iterator bbAux = funAux->begin(); bbAux != funAux->end(); bbAux++){
			BasicBlock *BB = bbAux; //OBS: This casting is necessary to avoid errors!!!!!
			//outs()<< "Name: "<<BB->getName()<<"  Value Name:"<<BB->getValueName()<<"\n";
			Node *n = new Node;
			n->setBB(BB);
			n->setId(nodeId++);
			n->setType(defineNodeType(bbAux));
			vNodes.push_back(n);
		}
	}
	for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		Node* node=*it;

		node->definePreds(vNodes);
		node->defineSuccs(vNodes);
	}

	return vNodes;
}


//Define the type of the Basic BLock. The types are A or X. A node is of type "A" if it has multiple
//predecessors and at least one of its predecessors has multiple successors. A node is of type X if it is not of type A.
NodeType Node::defineNodeType(BasicBlock *BB){

	NodeType bbType = typeX;
	//If this basic block has a single predecessor block,than the type is 'X'
	if (BB->getSinglePredecessor() != NULL)
		return bbType;

	for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
		BasicBlock *Pred = *PI;
		//outs()<< "  Previous: "<< Pred->getName() <<"\n";
		int i =0;
		for (succ_iterator SI = succ_begin(Pred), E = succ_end(Pred); SI != E; ++SI) {
			//BasicBlock *Succ = *SI;
			//outs()<< "    Successor: "<< Succ->getName() <<"\n";
			i++;
		}
		if (i>1)
			bbType=typeA;
	}
	return bbType;
}

void Node::printId(){
	outs()<<"Id:"<<this->getId()<<"\n";
}

void Node::printLowerSignatures(vector<Node*> vNodes){
	outs()<<"Lower Signatures\n";
	for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		Node *n = *it;
		outs()<<"<--> Id: "<<n->getId();
		BasicBlock *BB =n->getBB();
		outs()<<" Name: "<<BB->getName()<<" ";
		outs() << "\n";
		outs()<<" LowerNS:"<<n->getLowerNS()<< " bin:"<< dec2bin(n->getLowerNS())<<" LowerNES:"<<n->getLowerNES()<<" bin:"<< dec2bin(n->getLowerNES())<<"\n";
	}
	outs()<<"\n";

}

Node* Node::getNode(vector<Node*> vNodes, BasicBlock *BB){

	for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		Node *node = *it;

		if(node->getBB() == BB)
			return node;
	}
	return NULL;
}

void Node::printNodes(vector<Node*> vNodes){
	outs()<<">>>>>>>>>>>>>>>>>>>>>>\n";
	outs()<<">>>> Nodes properties\n";
	outs()<<"Type\n";
	for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		Node *n = *it;
		outs()<<"<--> Id: "<<n->getId();
		BasicBlock *BB =n->getBB();
		outs()<<" Name: "<<BB->getName()<<" ";
		outs() <<" TYPE: "<< n->getTypeStr();
		set<Node*> succ = n->getSuccs();
		outs() <<" has successors:"<< (succ.empty() ? " Not": "Yes");
		outs()<<"\n";
	}
	/*outs()<<"\n";
	outs()<<"Upper Signatures\n";
	for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		Node *n = *it;
		outs()<<"<--> Id: "<<n->getId();
		BasicBlock *BB =n->getBB();
		outs()<<" Name: "<<BB->getName()<<" ";
		outs() << "\n";
		outs()<<" UpperNS:"<<n->getUpperNS()<< " bin:"<< dec2bin(n->getUpperNS())<<" UpperNES:"<<n->getUpperNES()<<" bin:"<< dec2bin(n->getUpperNES())<<"\n";
	}*/
	outs()<<"\n";

	outs()<<"Signatures\n";
	for (vector<Node*>::iterator it = vNodes.begin() ; it != vNodes.end(); ++it){
		Node *n = *it;
		outs()<<"<--> Id:"<<n->getId();
		BasicBlock *BB =n->getBB();
		outs()<<"  Name: "<<BB->getName()<<" ";
		outs() << "\n";
		outs()<<" UpperNS:       "<<n->getUpperNS()      <<"  bin:"<< dec2bin(n->getUpperNS())<<"\n";
        outs()<<" UpperNES:      "<<n->getUpperNES()     <<"  bin:"<< dec2bin(n->getUpperNES())<<"\n";
		outs()<<" LowerNS:       "<<n->getLowerNS()      <<"  bin:"<< dec2bin(n->getLowerNS())<<"\n";
		outs()<<" LowerNES:      "<<n->getLowerNES()     <<"  bin:"<< dec2bin(n->getLowerNES())<<"\n";
		outs()<<" Signature:     "<<n->getSignature()    <<"  bin:"<< dec2bin(n->getSignature())<<"\n";
		outs()<<" Exit Signature:"<<n->getExitSignature()<<"  bin:"<< dec2bin(n->getExitSignature())<<"\n";
		outs()<<" D1:            "<<n->getd1Value()      <<"  bin:"<< dec2bin(n->getd1Value())<<"\n";
		outs()<<" D2:            "<<n->getd2Value()      <<"  bin:"<< dec2bin(n->getd2Value())<<"\n\n";
	}
}




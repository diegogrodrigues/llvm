/*
 * Signatures.cpp
 *
 *  Created on: Nov 12, 2013
 *      Author: diego
 */

#include "Signatures.h"


namespace SignaturesSpace {

Signatures::Signatures() {
	// TODO Auto-generated constructor stub

	switch (SIZE_SIGNATURE) {
		case 8:
			iniValUpper = "11000000";
			iniValLower = "10000000";
			iniOnes     = "11111111";
			iniZeros    = "00000000";
			break;
		case 16:
			iniValUpper = "1100000000000000";
			iniValLower = "1000000000000000";
			iniOnes     = "1111111111111111";
			iniZeros    = "0000000000000000";
			break;
		default:
			errs()<<"Error!!!!!!!";

	}
}
//Verify if two nodes have common predecessors
bool Signatures::haveCommonPred(Node *a,Node *b){
	BasicBlock *bbA =a->getBB();
	BasicBlock *bbB =b->getBB();
	for (pred_iterator PA = pred_begin(bbA), EA = pred_end(bbA); PA != EA; ++PA) {
		BasicBlock *predA = *PA;
		for (pred_iterator PB = pred_begin(bbB), EB = pred_end(bbB); PB != EB; ++PB) {
			BasicBlock *predB = *PB;

			if(predA==predB)
				return true;
		}
	}
	return false;
}
//
void Signatures::createSets(vector<Node*> vNodes){
	createNet(vNodes);
	createNetPred(vNodes);
	createA_Sig(vNodes);
}

//Creates the Nets and put them in the vector "nets".
void Signatures::createNet(vector<Node*> vNodes){
	for (int i=0;i< vNodes.size();i++){
		//Get the node.
		Node *nI = vNodes[i];
		bool found = false;
		BasicBlock *bbI =nI->getBB();
		//For each node verify if it has a common predecessor with any other node.
		//If it has, both nodes will be inserted in the same Net.
		for (int j=i+1;j< vNodes.size();j++){
			Node *nJ = vNodes[j];
			if(haveCommonPred(nI, nJ)){
				//Because Net is a set 'nI' will not generate duplicated values.
				insertInNet(nI, nJ);
				found = true;
			}
		}
		//If the node do not have a common predecessor with any other node, it will be inserted alone in the Net.
		if(!found){
			insertInNet(nI, NULL);
		}
	}
}
//Creates the Net_preds and put them in the vector nets_pred;
void Signatures::createNetPred(vector<Node*> vNodes){

	//Steps through all Nets
	for (int i=0;i< nets.size();i++){
		SetNode netAux = nets[i];
		SetNode netPred;
		//Steps through all nodes in a Net
		for (SetNode::iterator ni = netAux.begin(); ni != netAux.end(); ni++) {
			Node *node = *ni;
			//outs()<<"Node: "<<node->getBB()->getName()<<"\n";
			//Get all predecessors of the basic Block of the Node.
			for (pred_iterator P = pred_begin(node->getBB()), E = pred_end(node->getBB()); P != E; ++P) {
				BasicBlock * pred = *P;
				//Unfortunately this next operation is necessary.
				//Search for the correspondent Node.
				for (int j=0;j< vNodes.size();j++){
					Node *nI = vNodes[j];
					//outs() <<pred->getName() << " == "<< nI->getBB()->getName()<<"\n";
					if (pred == nI->getBB()){
						netPred.insert(nI);
					}
				}
			}
		}
		//If the Net does not have predecessors a empty netPred will be inserted.
		nets_pred.push_back(netPred);
	}

}
//
void Signatures::createA_Sig(vector<Node*> vNodes){

	for (int i=0;i< nets.size();i++){
		SetNode net = nets[i];
		/*
		 * Generates A_Sign_NES
		 */
		//Get Net_Pred
		SetNode net_pred = nets_pred[i];

		SetNode a_sign_nes;
		if(!net_pred.empty())
			a_sign_nes = net_pred;
		A_sig_NES.push_back(a_sign_nes);

		/*
		 * Generates A_Sign_NS
		 */
		SetNode netAux;
		for (SetNode::iterator ni = net.begin(); ni != net.end(); ni++) {
			Node *node = *ni;
			if(node->getType()== typeA){
				netAux.insert(node);
			}
		}
		A_sig_NS.push_back(netAux);
	}
}
//Verify if the Node is present
bool Signatures::isInNetPred(Node* a, int netIndex){
	SetNode net = nets_pred[netIndex];
	if(net.find(a)!=net.end()){
		return true;
	}
	return false;
}
//Insert elements in a SetNode
void Signatures::insertInNet(Node *a, Node * nB = NULL){

	//If the vector of Nets is empty, the insert a new SetNode
	if(nets.size()==0){
		SetNode netIns;
		netIns.insert(a);
		if(nB != NULL)
			netIns.insert(nB);
		nets.push_back(netIns);
	}else{
		//If the vector of Nets is NOT empty, verify if the Nodes are present in any SetNode of the vector.
		int sizeAux = nets.size();
		bool found = false;
		for (int i=0;i< sizeAux ;i++){
			SetNode netAux = nets[i];
			if(nB != NULL){
				//Check if any of the nodes is present in the current SetNode.
				//If any of them is present, perform the operation of update.
				//Because SetNode is a <set> the operation "insert" will not generate duplicated values.
				if(netAux.find(a) != netAux.end() || netAux.find(nB) != netAux.end()){
					found = true;
					netAux.insert(a);
					netAux.insert(nB);
					nets[i] = netAux;
					break;
				}
			}else{
				//Check if the node is present in the current Net.
				//If it is true, just inform that it was found.
				if(netAux.find(a) != netAux.end()){
					found = true;
					break;
				}
			}
		}
		//If the node was not found, insert it in as a new Net in the vector.
		if(!found){
			SetNode netIns;
			netIns.insert(a);
			if(nB != NULL)
				netIns.insert(nB);
			nets.push_back(netIns);
		}
	}
}

//Prints the content of the vector of Nets.
void Signatures::printNets(){
	outs()<<">>> Net values\n";
	if(nets.size()==0){
		outs()<<"The Net is Empty!\n";
		return;
	}
	for (int i=0;i< nets.size();i++){
		SetNode netAux = nets[i];
		outs()<<"Net <"<<i<<">: ";
		outs()<<"{";
		int count=0;
		for (SetNode::iterator ni = netAux.begin(); ni != netAux.end(); ni++) {
			Node *node = *ni;
			if(count>0)
			outs()<<", ";
			outs()<<node->getBB()->getName();
			count++;
		}
		outs()<<"}\n";
	}
}
//Prints the content of the vector of Net_Preds.
void Signatures::printNetPreds(){
	outs()<<">>> Net pred values\n";
	if(nets_pred.size()==0){
		outs()<<"The Net pred is Empty!\n";
		return;
	}
	for (int i=0;i< nets_pred.size();i++){
		SetNode netAux = nets_pred[i];
		outs()<<"Net pred <"<<i<<">: ";
		outs()<<"{";
		int count=0;
		for (SetNode::iterator ni = netAux.begin(); ni != netAux.end(); ni++) {
			Node *node = *ni;
			if(count>0)
			outs()<<", ";
			outs()<<node->getBB()->getName();
			count++;
		}
		outs()<<"}\n";
	}
}
//Prints the content of the vector of Net_Preds.
void Signatures::printA_Sig(){
	outs()<<">>> A_SIG_NES\n";
	if(A_sig_NES.size()==0){
		outs()<<"The A_sig_NES is Empty!\n";

	}else{
		for (int i=0;i< A_sig_NES.size();i++){
			SetNode netAux = A_sig_NES[i];
			outs()<<"A_sig_NES <"<<i<<">: ";
			if(netAux.empty()){
				outs()<<"{}\n";
			}else{
				outs()<<"{NES(";
				int count=0;
				for (SetNode::iterator ni = netAux.begin(); ni != netAux.end(); ni++) {
					Node *node = *ni;
					if(count>0)
					outs()<<"),NES(";
					outs()<<node->getBB()->getName();
					count++;
				}
				outs()<<")}\n";
			}
		}
	}
	outs()<<">>> A_SIG_NS\n";
	if(A_sig_NS.size()==0){
		outs()<<"The A_sig_NS is Empty!\n";
	}else{
		for (int i=0;i< A_sig_NS.size();i++){
			SetNode netAux = A_sig_NS[i];
			outs()<<"A_sig_NS <"<<i<<">: ";
			if(netAux.empty()){
				outs()<<"{}\n";
			}else{
				outs()<<"{NS(";
				int count=0;
				for (SetNode::iterator ni = netAux.begin(); ni != netAux.end(); ni++) {
					Node *node = *ni;
					if(count>0)
					outs()<<"),NS(";
					outs()<<node->getBB()->getName();
					count++;
				}
				outs()<<")}\n";
			}
		}
	}
	outs()<<"\n";
}

//
bool Signatures::calcSignatureValues(vector<Node*> vNodes){

	if(!calcUpperValues(vNodes))
		return false;

	if(!calcLowValues(vNodes))
		return false;

	if(!calcSignatures(vNodes))
		return false;

	if(!calcd1d2(vNodes))
		return false;

	return true;
}
//
bool Signatures::calcUpperValues(vector<Node*> vNodes){

	if(nets.size() != nets_pred.size()){
		outs() <<"Error: The size of set Net is different of size of set Net_pred!\n";
		return false;
	}
	if(nets.size() != A_sig_NS.size()){
		outs() <<"Error: The size of set Net is different of size of set A_sig_NS!\n";
		return false;
	}
	if(nets.size() != A_sig_NES.size()){
		outs() <<"Error: The size of set Net is different of size of set A_sig_NES!\n";
		return false;
	}

	valueUpperCounter = bin2dec(iniValUpper);

	//Step 1: for each Net, the upper half of all the signatures in the set A_sig(Net) are assigned the same unique value.
	//Set the value based on A_SIG
	for(int i =0; i < A_sig_NS.size();i++){
		//Remember that A_sig_NS and A_sig_NES must have the same size, and that each position (of vector) represent the same Net position.
		SetNode ns  = A_sig_NS[i];
		SetNode nes = A_sig_NES[i];

		//Generates values for Upper Signature
		for (SetNode::iterator itns = ns.begin(); itns != ns.end(); itns++) {
			Node *nNS = *itns;
			nNS->setUpperNS(valueUpperCounter);

		}
		//Generates values for Upper Exit Signature
		for (SetNode::iterator itnes = nes.begin(); itnes != nes.end(); itnes++) {
			Node *nNES = *itnes;
			nNES->setUpperNES(valueUpperCounter);
		}
		valueUpperCounter++;
	}
	//step 2: The upper half of any unassigned NS and NES of nodes are assigned different unique values.
	//outs()<<"valueUpperCounter: "<<valueUpperCounter<<"\n";
	for(int j =0 ; j < vNodes.size();j++){
		Node *node = vNodes[j];
		if(node->getUpperNS() == EMPTY_SIGNATURE)
			node->setUpperNS(++valueUpperCounter);
		if(node->getUpperNES() == EMPTY_SIGNATURE)
			node->setUpperNES(++valueUpperCounter);
	}
	return true;
}


/*void Signatures::dummy(){

	vector<SetNode> vecAux;

	SetNode n0 = nets_pred[5];
	vecAux.push_back(n0);

	SetNode n1 = nets[1];
	vecAux.push_back(n1);

	SetNode n2 = nets[2];
	vecAux.push_back(n2);

	SetNode n3 = nets[3];
	vecAux.push_back(n3);

	//SetNode n4 = nets[4];
	//vecAux.push_back(n4);

	SetNode n5;
	SetNode n6;

	SetNode::iterator it1 = n3.begin();
	std::advance(it1, 2);
	Node *node1 =*it1;
	n5.insert(node1);

	SetNode::iterator it2 = n3.begin();
	std::advance(it2, 1);
	Node *node2 =*it2;
	n5.insert(node2);

	SetNode::iterator it3 = n3.begin();
	std::advance(it3, 0);
	Node *node3 =*it3;
	n5.insert(node3);

	SetNode::iterator it4 = n2.begin();
	std::advance(it4, 0);
	Node *node4 =*it4;
	n5.insert(node4);

	printNets();
	int pos;
	if(isSetNodeInVecNode(vecAux,n5,pos)){
		outs()<< "isSetNodeInVecNode: TRUE  POS:"<< pos<<"\n";
	}else{
		outs()<< "isSetNodeInVecNode: FALSE\n";
	}
}*/

bool Signatures::isSetNodeInVecNode(vector<SetNode> vec, SetNode setSearch, int &pos){

	pos =-1;
	//outs() << "Start search\n";
	//Makes the search in all SETs of the vector.
	for(int i = 0 ; i < vec.size(); i++){
		SetNode setVec = vec[i];

		bool isInSet = false;
		//Now, for each NODE in set 'setSearch' verify if it exists in the set 'setVec'.
		for (SetNode::iterator itS = setSearch.begin(); itS != setSearch.end(); itS++) {
			Node *nodeSearch= *itS;
			//outs()<<"nodeSearch: "<<nodeSearch->getBB()->getName()<<"  i:"<<i<<"\n";
			bool found = false;//First assume that we not have found nodeVec in setSearch.
			//Search nodeVec in all elements of setSearch.
			for (SetNode::iterator itV = setVec.begin(); itV != setVec.end(); itV++) {
				Node *nodeVec = *itV;
				//outs()<<"      nodeVec: "<< nodeVec->getBB()->getName() <<"\n";
				//If at least one is equal, say that it has been found.
				if(nodeVec == nodeSearch){
					found = true;//If we found nodeVec in setSearch, we will  say that we have found.
					break;
				}
			}
			//outs()<<"\n";
			//If we found nodeVec, we set isInSet to true, otherwise it will be false.
			isInSet = found;
			//If we have not found nodeSearch in setVec
			if(!found){
				break;
			}
			//If all elements are equal return true.
		}
		//After we search all nodes of setSearch in all nodes of vec[i] we will have a answer if they mach or note.
		//If they match we do not need to continue the search, so we return true. Otherwise we need to continue the search.
		if(isInSet){
			//outs()<<"FOUND!!!!\n";
			pos = i;
			return true;
		}
		//else
			//outs()<<"NOT FOUND!!!!\n";

	}
	return false;
}



vector<SetNode> Signatures::getCommonPredsOfSuccX(SetNode nodes){

	vector<SetNode> vecPreds;


	//For each node of Set "nodes" verify if it has common successors with other nodes of the same set.
	for (SetNode::iterator itA = nodes.begin(); itA != nodes.end(); itA++) {
		SetNode setPreds;
		for (SetNode::iterator itB = nodes.begin(); itB != nodes.end(); itB++) {
			if(itA!=itB){
				//Set pointers to nodes.
				Node *nodeA = *itA;
				Node *nodeB = *itB;

				SetNode succA = nodeA->getSuccs();
				SetNode succB = nodeB->getSuccs();
				SetNode intersect;
				//If the intersection of two set of successors is not empty, the node have common sucessors.
				std::set_intersection(succA.begin(),succA.end(), succB.begin(),succB.end(), inserter(intersect,intersect.begin()) ) ;
				if(!intersect.empty()){
					for(SetNode::iterator it = intersect.begin(); it!=intersect.end(); it++){
						Node *succ = *it;
						//Verify if the type of the successor is X
						if(succ->getType() == typeX){
							setPreds.insert(nodeA);
							setPreds.insert(nodeB);
							//outs()<< ">>>>>> Size setPreds:"<<setPreds.size() << "\n";
						}
					}
				}
				//Verify if any of nA (node A) successors is present in nb (node b) sucessors.
				//if (na->)
 			}
		}
		if(!setPreds.empty()){
			int pos;
			if(!isSetNodeInVecNode(vecPreds,setPreds,pos))
				vecPreds.push_back(setPreds);
		}
		//outs()<< "###### Size vecPreds:"<<vecPreds.size() << "\n";
	}
	return vecPreds;
}

//Puts in a set the union of all Zl(preds). Preds are all predecessors of "node"
bool Signatures::getZlPred(Node *node, set<__uint64_t> *result){

	SetNode preds = node->getPreds();
	for(SetNode::iterator pred=preds.begin(); pred != preds.end(); pred++){
		Node *nodePred = *pred;
		//None of predecessor may have EMPTY_SIGNATURE
		if(nodePred->getLowerNES()==EMPTY_SIGNATURE){
			outs()<<"Empty signature error! Node: "<< node->getBB()->getName() << " Predecessor: "<<nodePred->getBB()->getName() <<"\n";
			return false;
		}else{
			//outs()<<"Node: "<< node->getBB()->getName() << " Predecessor: "<<nodePred->getBB()->getName() <<"\n";
			string valStr = dec2bin(nodePred->getLowerNES());
			set<__uint64_t> tmp = calcZl(nodePred->getLowerNES());
			result->insert(tmp.begin(), tmp.end());
		}
	}
	return true;
}

bool Signatures::calcLowValues(vector<Node*> vNodes){

	//Initialize values of lower signature.
	valueLowerCounter = bin2dec(iniValLower);

	//outs()<< "Initiate calcLowValues\n";
	// Case 1: Zl(S1) =/= Zl(S2), where S1,S2 belong to A_sig(Net). The only exception to this rule is when both S1 and S2
	//are exit signatures of sources of the same type X node. In such a case,Zl(S1) == Zl(S2). This rule will ensure
	//that the lower half of signatures whose upper half is the same will be different unless they qualify for the
	//exception case, when they have to be the same.
	//Set the value based on A_SIG
	for(int i =0; i < A_sig_NS.size();i++){
		//Remember that A_sig_NS and A_sig_NES must have the same size, and that each position (of vector) represent the same Net.
		SetNode nes = A_sig_NES[i];
		//Put in a vector of sets all nodes of 'nes' with common successors of type X.
		vector<SetNode> commonNES = getCommonPredsOfSuccX(nes);
		for (vector<SetNode>::iterator it = commonNES.begin(); it !=commonNES.end(); it++){
			//All nodes in the set must have the same Signature
			SetNode commonNode = *it;
			for (SetNode::iterator it2 = commonNode.begin(); it2 != commonNode.end(); it2++) {
				Node *nNES = *it2;
				nNES->setLowerNES(valueLowerCounter);
				//outs()<<"equal!\n";
			}
			valueLowerCounter++;
		}
	}
	for(int i =0; i < A_sig_NS.size();i++){
		//Remember that A_sig_NS and A_sig_NES must have the same size, and that each position (of vector) represent the same Net.
		SetNode ns  = A_sig_NS[i];
		SetNode nes = A_sig_NES[i];
		//Generates values for Node Exit Signature
		for (SetNode::iterator itnes = nes.begin(); itnes != nes.end(); itnes++) {
			Node *nNES = *itnes;
			if(nNES->getLowerNES()==EMPTY_SIGNATURE)
				nNES->setLowerNES(valueLowerCounter++);
		}
		//Generates values for Node Signature
		for (SetNode::iterator itns = ns.begin(); itns != ns.end(); itns++) {
			Node *nNS = *itns;
			nNS->setLowerNS(valueLowerCounter++);
		}
	}

	for(int j =0; j < nets.size(); j++){
		SetNode net = nets[j] ;
		//outs()<< "Case 2 !!!!!!!\n";
		//Case 2: Zl(NES(N2)) C  Zl(NS1)), where N1 belongs to Net, NT(N1)=A, and N2 belongs to pred(N1). This will ensure
		//that the value in the signature register inside a type A node, N1, will be the same if the prior execution
		//node is in predðN1Þ. This is because all the bits in 	ZlðNSðN1ÞÞ and only those bits will be masked at
		//the start of N1.
		//steps through all nodes of the net
		for(SetNode::iterator itNode=net.begin(); itNode!=net.end(); itNode++){
			Node *node = *itNode;
			if(node->getType() == typeA){
				//outs()<<"Type A: "<< node->getBB()->getName()<<"\n";
				set<__uint64_t> nZl;
				if(!getZlPred(node, &nZl))
					return false;
				if(!nZl.empty()){
					string ns = iniOnes;
					for(set<__uint64_t>::iterator itZl = nZl.begin(); itZl != nZl.end(); itZl++){
						int pos= *itZl;
						//outs()<<pos << " ";
						ns[pos]='0';
					}
					//outs()<<"\n";
					//outs()<<ns<<"\n";
					node->setLowerNS(bin2dec(ns));
				}
			}
		}
	}


	for(int j =0; j < nets.size(); j++){
		SetNode net = nets[j] ;

		//outs()<< "Case 3 !!!!!!!\n";
		//Case 3. Zl(NES(N2)) C/  Zl(NS1)), where N1 belongs to Net, NT(N1) = A, N2 belongs to A_sig(Net), and N2 does not belong to pred(N1).
		//This will ensure that any illegal jump from end of a nonsource node to the beginning of a type A node,
		//N1, will result in the expected and actual signature register values inside N1 being different.
		//OBS: Performance was not considered in this algorithm.

		for(SetNode::iterator itNode=net.begin(); itNode!=net.end(); itNode++){
			Node *node = *itNode;
			//Case 3. Step (a): Verify if the node type is 'A'.
			if(node->getType() == typeA){
				//outs()<<"node net: "<< node->getBB()->getName()<<"\n";
				SetNode a_sig_nes = A_sig_NES[j];
				if(a_sig_nes.size()>0){
					//Case 3. Step (b): Verify if the is any NES(N2) in A_SIG.
					set<__uint64_t> nZl;
					nZl.clear();
					for(SetNode::iterator itSig = a_sig_nes.begin(); itSig != a_sig_nes.end(); itSig++){
						Node * nodeSig =*itSig;
						//outs()<<"nodeSig:  "<< nodeSig->getBB()->getName()<<"\n";
						//Case 3. Step (c): Verify if NES(N2) is not in pred(N1)
						if(node->getPreds().find(nodeSig) == node->getPreds().end()){
							//outs()<<"Not in pred:  "<< nodeSig->getBB()->getName()<<"\n";
							set<__uint64_t> tmp = calcZl(nodeSig->getLowerNES(),16);
							nZl.insert(tmp.begin(), tmp.end());
						}
					}
					if(!nZl.empty()){
						//outs()<<"not empty\n";
						string ns = iniZeros;
						for(set<__uint64_t>::iterator itZl = nZl.begin(); itZl != nZl.end(); itZl++){
							int pos= *itZl;
							ns[pos]='1';
						}
						//outs()<<ns<<"\n";
						node->setLowerNS(bin2dec(ns));
					}
				}
			}
		}
		//outs()<< "End Case 2 & 3.\n";
	}
	//Set the ramaining values.
	for(int k =0 ; k < vNodes.size();k++){
		Node *node = vNodes[k];
		if(node->getLowerNS()== EMPTY_SIGNATURE)
			node->setLowerNS(++valueLowerCounter);
		if(node->getLowerNES()== EMPTY_SIGNATURE)
			node->setLowerNES(++valueLowerCounter);
	}
	return true;
}

bool Signatures::calcSignatures(vector<Node*> vNodes){

	for(int k =0 ; k < vNodes.size();k++){
		Node * node = vNodes[k];
		string signature = dec2bin(node->getUpperNS())+dec2bin(node->getLowerNS());
		string exitSignature = dec2bin(node->getUpperNES())+dec2bin(node->getLowerNES());
		//Signature
		node->setSignature(bin2dec(signature));
		//Exit signature
		node->setExitSignature(bin2dec(exitSignature));
	}
	return true;
}

bool Signatures::calcd1d2(vector<Node*> vNodes){


	for(int i =0 ; i < vNodes.size(); i++){
		Node *node = vNodes[i];
		__uint64_t d1 = 0;
		__uint64_t d2 = 0;
		//For each node N1, if N1 is of type A, the upper half of d1 is set to an all 1 pattern.
		//Its lower half is set equal to the lower half of NS(N1).
		if(node->getType() == typeA){
			string lower = dec2bin(node->getLowerNS());
			string d1Str = iniOnes+lower;
			d1 = bin2dec(d1Str);;
			node->setd1Value(d1);

		}else{
			//if N1 is of type X.
			d1 = node->getSignature();
			SetNode preds = node->getPreds();
			__uint64_t tmpExit =0;
			if(!preds.empty()){
				for(SetNode::iterator pred=preds.begin(); pred != preds.end(); pred++){
					Node *nodePred = *pred;
					tmpExit = nodePred->getExitSignature() ;
					//outs()<<" Pred  "<< nodePred->getBB()->getName()<<" type:"<< nodePred->getTypeStr()<<"\n";
					break;
				}
			}
			d1 = d1^tmpExit; //xor
			node->setd1Value(d1);
		}

		d2 = node->getSignature() ^ node->getExitSignature();
		node->setd2Value(d2);

	}
	return true;
}

bool Signatures::verifySignaturesD2(vector<Node*> vNodes){
	bool resultBool=true;
	//errs()<<"verifySignaturesD2!\n";
	for(int i =0 ; i < vNodes.size(); i++){
		Node *node = vNodes[i];
		//Verify
		//NS(Ni) and d2(Ni) == NES(Ni+1)
		__int64_t result = node->getSignature() ^ node->getd2Value();
		if(result != node->getExitSignature()){
			resultBool = false;
			errs()<<"Block Name: "<< node->getBB()->getName() << " Type: " <<node->getTypeStr();
			errs() <<"\n";
			errs()<<"NS:    "<<dec2bin(node->getSignature()) <<"\n";
			errs()<<"D2:    "<<dec2bin(node->getd2Value()) <<"\n";
			errs()<<"NES:   "<<dec2bin(node->getExitSignature()) <<"\n";
			errs()<<"Result:"<<dec2bin(result) <<"\n";
			errs()<<"Node D2 verification: ";
			errs()<<"Error!!\n";
		}
	}
	return resultBool;
}

bool Signatures::verifySignaturesD1(vector<Node*> vNodes){
	bool resultBool=true;
	//errs()<<"verifySignaturesD1!\n";
	for(int i =0 ; i < vNodes.size(); i++){
		Node *node = vNodes[i];
		set<Node*> succs = node->getSuccs();
		//Verify
		//NES(Ni) and d1(Ni+1) == NS(Ni+1), if NT(Ni+1) == A
		//NES(Ni) xor d1(Ni+1) == NS(Ni+1), if NT(Ni+1) == X
		if(!succs.empty()){
			__int64_t exitSigroot = node->getExitSignature();
			for(set<Node*>::iterator it = succs.begin(); it != succs.end(); it++){
				Node * succ = *it;
				__int64_t result = 0;
				string op;
				if(succ->getType()==typeA){
					op ="AND";
					result = node->getExitSignature() & succ->getd1Value();
				}else{
					op= "XOR";
					result = node->getExitSignature() ^ succ->getd1Value();
				}

				if( result != succ->getSignature()){
					resultBool = false;
					errs()<< "Block Name: "<< node->getBB()->getName() << " Type: " <<node->getTypeStr();
					errs()<< "  Num succ: " <<node->getSuccs().size();
					errs() <<"\n";
					errs()<< "  Succ Name: "<< succ->getBB()->getName()<< " Type:" <<succ->getTypeStr();
					errs() <<"\n";
					errs()<<"  NES   :"<<dec2bin(node->getExitSignature()) <<"\n";
					errs()<<"  D1    :"<<dec2bin(succ->getd1Value())<<"\n";
					errs()<<"  NS    :"<<dec2bin(succ->getSignature())<<"\n";
					errs()<<"  Result:"<<dec2bin(result) <<"\n";
				}
			}
		}

	}
	return resultBool;
}

//********************
} /* namespace Ceda */

/*
 * Util.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: diego
 */
#include "Util.h"

using namespace std;

namespace util{

	/*vector<int> intToVecInt(int x) {
		vector<int> ret;
		while(x) {
			if (x&1){
				ret.push_back(1);
			}else{
				ret.push_back(0);
			}
			x>>=1;
		}
		reverse(ret.begin(),ret.end());

		return ret;
	}

	string vecIntTostring(vector<int> val) {
		string ret;
		for(vector<int>::iterator it =val.begin(); it!=val.end(); it++ ){
			if(*it==1)
				ret+="1";
			else
				ret+="0";
		}
		return ret;
	}*/

	//return
	bool isEqualZl(__uint64_t v1, __uint64_t v2){
		if(v1!=v2)
			return false;
		return true;
	}

	__uint64_t bin2dec(string binary)
	{
		int len,i,exp;
		__uint64_t dec=0;

		len = binary.size();
		exp = len-1;
		for(i=0;i<len;i++,exp--)
			dec += binary[i]=='1'?pow(2,exp):0;
		return dec;

	}

	string dec2bin(__uint64_t x, int size) {

		string ret="";
		long tmp = x;
		if(tmp < 0)
			return "Negative not supported!!";
		while(x) {
			if (x&1){
				ret ="1"+ret;
			}else{
				ret ="0"+ret;
			}
			x>>=1;
		}
		while(ret.size()<size){
			ret = "0"+ret;
		}
		return ret;
	}

	set<__uint64_t> calcZl(__uint64_t val, int size)
	{
		set<__uint64_t> result;

		string valStr = dec2bin(val, size);
		//cout<<"\nvalStr: "<<valStr<<"\n";

		//cout<<valStr[0];
		//cout<<valStr[1];
		//cout<<valStr[2];
		//int pos = 15;
		for(int i = 0; i< valStr.size() ;i++){
			if(valStr[i]=='0'){
				result.insert(i);
				//cout<<i<< "  ";
			}
			//pos--;
		}
		//cout<<endl;
		return result;
	}


	__uint64_t xorBin(__uint64_t a, __uint64_t b, int size){

		//long tmp = a;
		//if(tmp < 0)
		//	return "Negative not supported!!";
		//tmp = b;
		//if(tmp < 0)
		//	return "Negative not supported!!";

		string strA = dec2bin(a,size);
		string strB = dec2bin(b,size);
		string result="";
		for(int i=1; i <=16; i++){

			int x = 0;
			int y = 0;
			int z = 0;
			if(strA[i]=='1')
				x = 1;

			if(strB[i]=='1')
				y = 1;
			z = x^y;

			result += z==0?"0":"1";
		}

		return bin2dec(result);
	}


	void test()
	{
/*		string str = dec2bin(-1);
		cout<<str<<"\n";
		return;

		bool error = false;
		for (int i =0; i<= 65535; i++){
			//convert to binary
			string str = dec2bin(i);
			cout<< str<<"\n";
			//convert to integer
			int j = bin2dec(str);
			if(j!=i){
				error =true;
				cout<<"Error. i:"<< i<<" j:"<<j<<"\n";
			}
		}
		if(error){
			cout<< "An error has occurred!\n";
		}else{
			cout<< "No error occurred!\n";
		}*/

	}

}




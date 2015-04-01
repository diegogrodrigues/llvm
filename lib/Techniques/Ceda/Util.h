/*
 * Util.h
 *
 *  Created on: Nov 13, 2013
 *      Author: diego
 */

#ifndef UTEIS_H_
#define UTEIS_H_

#include <vector>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <math.h>
#include <set>

using namespace std;

#define SIZE_SIGNATURE 16


namespace util{

	//vector<int> intToVecInt(int x);
	//string vecIntTostring(vector<int> val);
	bool isEqualZl(__uint64_t longval1, __uint64_t longva2);
	__uint64_t bin2dec(string binary);
	string dec2bin(__uint64_t longx, int size = SIZE_SIGNATURE);
	set<__uint64_t> calcZl(__uint64_t longval, int size = SIZE_SIGNATURE);
	void test();
	//__uint64_t xorBin(__uint64_t a, __uint64_t b, int size);
}

#endif /* UTEIS_H_ */


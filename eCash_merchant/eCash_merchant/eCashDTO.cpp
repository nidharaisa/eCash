//
//  eCashDTO.cpp
//  eCash_Client
//
//  Created by Raisa Islam on 19/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <bitset>
#include <functional>
#include <iostream>
#include <random>

#include "eCashDTO.hpp"
#include "RSA.hpp"

eCash::eCash(uint32_t id, uint32_t amount)
{
	this->amount = amount;
	this->uniqueId = id;
}

eCash::~eCash()
{
	std::cout << "~eCash : " << uniqueId << std::endl;

}

bool eCash::verifyCommitment()
{
	std::hash<uint32_t> hashPtr;
	std::bitset<8> mask(commit);

	std::cout << "commit string: " << mask << " uID: " << uniqueId << std::endl;

	for (int k=0; k<8; k++) {

		auto p = rPairs[k];

		if(mask[k]) {
			// revealed left part
			if(hID[k].first != hashPtr(p.first ^ p.second)) {
				// std::cout << "rev left for " << k << " " << hID[k].first << " " << hashPtr(p.first ^ p.second) << std::endl;
				return false;
			}
		}
		else {
			// revealed right part
			if(hID[k].second != hashPtr(p.first ^ p.second)) {
				// std::cout << "rev right for " << k << " " << hID[k].second << " " << hashPtr(p.first ^ p.second) << std::endl;
				return false;
			}
		}
	}

	std::cout << "commit verified. ret true\n";
	return true;
}

int eCash::prepareRawData(uint32_t *m)
{
	int sz = 0;
	m[sz++] = uniqueId;
	std::cout << "uID: " << uniqueId << std::endl;
	m[sz++] = amount;
	m[sz++] = hID.size();
	
	for (int i=0; i<hID.size(); i++) {
		m[sz++] = hID[i].first;
		m[sz++] = hID[i].second;
	}

	m[sz++] = signSz;
	memcpy(&m[sz], sign, signSz*4);
	sz += signSz;

	return sz;
}

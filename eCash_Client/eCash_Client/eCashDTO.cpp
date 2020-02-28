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
	this->addressId = id;
	this->amount = amount;
	this->uniqueId = arc4random_uniform(1<<17);

	// generateIDs
	generateSharedSecret();
}

eCash::~eCash()
{
	// std::cout << "~eCash : " << uniqueId << std::endl;

}

void eCash::generateSharedSecret()
{
	std::hash<uint32_t> hashPtr;
	
	for (int i=0; i<8; i++) {
		uint32_t r = arc4random();
		uint32_t l = addressId ^ r;

		uint32_t rl = arc4random();
		uint32_t rr = arc4random();
		
		uint32_t hl = hashPtr(l^rl);
		uint32_t hr = hashPtr(r^rr);

		randoms.push_back(std::make_pair(r, std::make_pair(rl, rr)));
		hID.push_back(std::make_pair(hl, hr));
	}
}

std::vector<std::pair<uint32_t, uint32_t>> eCash::bitCommitment(uint8_t inp)
{
	// Bit commitment
	std::vector<std::pair<uint32_t, uint32_t>> commit;

	std::hash<uint32_t> hashPtr;
	std::bitset<8> y(inp);

	std::cout << "masking input : " << y << std::endl;
	for (int k=0; k<8; k++) {
		
		std::pair<uint32_t, uint32_t> rev;
		
		if(true == y[k]) {
			// reveal left part
			rev = std::make_pair(randoms[k].first^addressId, randoms[k].second.first);
		}
		else {
			// reveal right part
			rev = std::make_pair(randoms[k].first, randoms[k].second.second);
		}
		commit.push_back(rev);
	}
	
	return commit;
}

int eCash::prepareRawData(uint32_t *m)
{
	int sz = 0;
	m[sz++] = uniqueId;
	std::cout << "uID: " << uniqueId << std::endl;
	m[sz++] = amount;
	m[sz++] = (uint32_t) hID.size();

	for (int i=0; i<hID.size(); i++) {
		m[sz++] = hID[i].first;
		m[sz++] = hID[i].second;
	}

	m[sz++] = signLen;
	memcpy(&m[sz], sign, signLen*4);
	sz += signLen;

	return sz;
}

int eCash::prepareForSign(uint32_t* m)
{
	uint32_t org[512];
	int sz = 0;
	org[sz++] = uniqueId;
	std::cout << "sign uID: " << uniqueId << std::endl;
	org[sz++] = amount;
	org[sz++] = hID.size();

	for (int i=0; i<hID.size(); i++) {
		org[sz++] = hID[i].first;
		org[sz++] = hID[i].second;
	}

	blindFactor = RSA::getInstance()->blindFold(org, m, sz);

	// receiver should store in 32bit array
	return sz;
}

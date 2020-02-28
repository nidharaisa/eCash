//
//  eCashDTO.cpp
//  eCash_Client
//
//  Created by Raisa Islam on 19/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

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
	// std::cout << "~eCash : " << uniqueId << std::endl;

}

uint32_t eCash::verifyCheater(uint32_t mask, std::vector<std::pair<uint32_t, uint32_t> > pairs)
{
	uint32_t address = 0;

	if(mask ^ commit) {
		for (int k=0; k<8; k++) {
			
			if(((mask & (1<<k)) && !(commit & (1<<k))) || (!(mask & (1<<k)) && (commit & (1<<k)))) {
				address = pairs[k].first ^ rPairs[k].first;
				break;
			}
		}
	}
	return address;
}

int eCash::prepareDataToSign(uint32_t *m)
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

	return sz;
}

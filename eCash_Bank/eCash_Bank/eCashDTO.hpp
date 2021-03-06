//
//  eCashDTO.hpp
//  eCash_Client
//
//  Created by Raisa Islam on 19/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#ifndef eCashDTO_hpp
#define eCashDTO_hpp

#include <stdio.h>
#include <vector>

class eCash{
public:
	eCash(uint32_t id, uint32_t amount);
	~eCash();

	uint32_t verifyCheater(uint32_t mask, std::vector<std::pair<uint32_t, uint32_t>> pairs);
	int prepareDataToSign(uint32_t *m);

public:
	u_int32_t amount{0};
	u_int32_t uniqueId{0};
	
	uint32_t rawData[1024];
	int rawDataSz{0};

	// <HL, HR>
	std::vector<std::pair<u_int32_t, uint32_t>> hID;
	// <R, RL> || <L, RL>
	std::vector<std::pair<uint32_t, uint32_t>> rPairs;

	uint32_t signSz{0};
	uint32_t sign[50];

	uint8_t commit{0};
};

#endif /* eCashDTO_hpp */

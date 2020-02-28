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

	void generateSharedSecret();
	
	int prepareRawData(uint32_t* m);
	int prepareForSign(uint32_t* m);

	std::vector<std::pair<uint32_t, uint32_t>> bitCommitment(uint8_t inp);

public:
	u_int32_t amount{0};
	u_int32_t uniqueId{0};
	
	std::pair<uint32_t, uint32_t> blindFactor;

	// <R, <RL, RR>>
	std::vector<std::pair<u_int32_t, std::pair<uint32_t, uint32_t>>> randoms;
	// <HL, HR>
	std::vector<std::pair<u_int32_t, uint32_t>> hID;

	uint32_t sign[50];
	uint32_t signLen{0};

private:
	uint32_t addressId{};
};

#endif /* eCashDTO_hpp */

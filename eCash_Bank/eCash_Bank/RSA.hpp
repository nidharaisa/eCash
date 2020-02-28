//
//  RSA.hpp
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <vector>

#include "eCashDTO.hpp"

#ifndef RSA_hpp
#define RSA_hpp

class RSA {

public:
	static RSA* getInstance();
	static void resetInstance();

	uint32_t* encryptMessage(uint32_t* m, int len);
	int decryptMessage(uint32_t* c, uint32_t* dest, int len);
	
	inline int digitalSign(uint32_t* m, uint32_t* dest, int len) {

		decryptMessage(m, dest, len);
		return len;
	}
	
	int blindUnfold(std::vector<std::unique_ptr<eCash>> &cashPool, uint8_t toSign, std::vector<std::pair<uint32_t, uint32_t>> &blindFacts, int amount);

private:
	RSA();
	~RSA();

	uint32_t gcdExtended(uint32_t a, uint32_t b, uint32_t *x, uint32_t *y);
	void generateEncryptionKey();
	void seive();
	uint32_t bigMod(uint32_t a, uint32_t pow);
	
public:
	uint32_t n;
	uint32_t e, d;

private:
	static RSA* instance;
	
	std::vector<uint32_t> primes;
};

#endif /* RSA_hpp */

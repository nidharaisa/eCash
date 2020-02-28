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

#ifndef RSA_hpp
#define RSA_hpp

class RSA {

public:
	static RSA* getInstance();
	static void resetInstance();

	uint32_t* encryptMessage(uint32_t* m, int len, uint32_t N);
	uint32_t* decryptMessage(uint32_t* c, int len, uint32_t N);

	inline uint32_t* digitalSign(uint32_t* m, int len) {
		return decryptMessage(m, len, n);
	}

	std::pair<uint32_t, uint32_t> blindFold(uint32_t* src, uint32_t* dest, int len);
	uint32_t* extractOrgSign(uint32_t* data, int len, uint32_t kInv, uint32_t N);

	std::vector<std::pair<uint32_t, uint32_t>> generateInversePairs(uint32_t N);
	uint32_t bigMod(uint32_t a, uint32_t pow, uint32_t N);

private:
	RSA();
	~RSA();
	
	uint32_t gcdExtended(uint32_t a, uint32_t b, uint32_t *x, uint32_t *y);
	void generateEncryptionKey();
	void seive();
	
	uint32_t modMultiplication(uint32_t a, uint32_t b, uint32_t mod);

public:
	uint32_t n;
	uint32_t e, d;
	
	uint32_t bankN{0};
	uint32_t bankE{0};

	int blindFactPos{0};
	std::vector<std::pair<uint32_t, uint32_t>> blindFactors;
	
private:
	static RSA* instance;
	
	std::vector<uint32_t> primes;
};

#endif /* RSA_hpp */

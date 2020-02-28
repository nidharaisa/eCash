//
//  RSA.cpp
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <iostream>
#include <numeric>
#include <unistd.h>

#include "RSA.hpp"

RSA* RSA::instance = nullptr;

RSA::RSA()
{
	seive();
	generateEncryptionKey();
}

RSA::~RSA()
{
	
}

// static
RSA* RSA::getInstance()
{
	if (!instance) {
		instance = new RSA();
	}
	return instance;
}

// static
void RSA::resetInstance()
{
	if (instance) {
		delete instance;
		instance = nullptr;
	}
}

void RSA::seive()
{
	int N = 10000;
	bool prime[N+1];
	memset(prime, true, sizeof(prime));

	for (int p = 4; p <= N; p += 2) prime[p] = false;

	for (int p = 3; p*p <= N; p += 2)
	{
		// If prime[p] is not changed, then it is a prime
		if (prime[p] == true)
		{
			// Update all multiples of p greater than or
			// equal to the square of it
			// numbers which are multiple of p and are
			// less than p^2 are already been marked.
			for (int i=p*p; i<=N; i += p)
				prime[i] = false;
		}
	}

	// store all prime numbers
	for (int p=2; p<=N; p++)
		if (prime[p]) {
			primes.push_back(p);
		}
}
uint32_t RSA::gcdExtended(uint32_t a, uint32_t b, uint32_t *x, uint32_t *y)
{
	// Base Case
	if (a == 0)
	{
		*x = 0, *y = 1;
		return b;
	}
	
	uint32_t x1, y1; // To store results of recursive call
	uint32_t gcd = gcdExtended(b%a, a, &x1, &y1);
	
	// Update x and y using results of recursive
	// call
	*x = y1 - (b/a) * x1;
	*y = x1;
	
	return gcd;
}

void RSA::generateEncryptionKey()
{
	uint32_t p, q, phi;
	p = primes[arc4random_uniform(primes.size()-50)+50];
	do {
		q = primes[arc4random_uniform(primes.size()-125)+125];
	} while (p == q);

	std::cout <<"values : " << p << " " << q <<  " " << (p*q) << std::endl;

	n = p*q;
	phi = (p-1)*(q-1);

	std::cout << "n : " << n << " " << " phi : " << phi << std::endl;

	// find e and d
	int r = (int) arc4random_uniform(20)+5;
	for(int i = 2; i < phi && r; i++)
	{
		if(1 == std::gcd(i, phi)) {

			uint32_t x, y;
			if (1 == gcdExtended(i, phi, &x, &y))
			{
				// N is added to handle negative x
				int res = (x%phi + phi) % phi;
				if (1 == (i*res)%phi) {
					
					e = i; d = res;
					std::cout << "keys #" << r << ": " << e << " " << d << std::endl;
					r--;
				}
			}
		}
		usleep(10);
	}
	std::cout << "keys : " << e << " " << d << std::endl;
}

uint32_t RSA::bigMod(uint32_t a, uint32_t pow)
{
	uint64_t answer = 1;
	uint64_t remainder = a%n;
	
	while(pow!=0)
	{
		if(pow%2==1)
		{
			answer = (answer*remainder)%n;
		}
		
		remainder = (remainder*remainder)%n;
		pow /= 2;
	}
	return (uint32_t) answer;
}

uint32_t* RSA::encryptMessage(uint32_t* m, int len)
{
	uint32_t c[len];
	
	for (int i=0; i<len; i++) {
		c[i] = bigMod(m[i], e);
	}
	
	return c;
}

int RSA::decryptMessage(uint32_t* c, uint32_t* dest, int len)
{
	for (int i=0; i<len; i++) {
		dest[i] = bigMod(c[i], d);
	}

	return len;
}

int RSA::blindUnfold(std::vector<std::unique_ptr<eCash> > &cashPool, uint8_t toSign, std::vector<std::pair<uint32_t, uint32_t>> &blindFacts, int amount)
{
	for (int i=0; i<blindFacts.size(); i++) {
		
		if (i == toSign) continue;

		auto *buff = cashPool[i]->rawData;
		int len = cashPool[i]->rawDataSz;

		std::cout << blindFacts[i].first << " " << blindFacts[i].second << "\n";

		uint64_t f = bigMod(blindFacts[i].second, e)%n;

		for (int k=0; k<len; k++) {
			buff[k] = (buff[k]* f)%n;
		}

		uint32_t *tem = (uint32_t*) &buff[0];

		if(amount != tem[1]){
			std::cout << "MISMATCH AMOUNT for cash req : " << i << " " << amount << " " << (int)tem[1] << ". Terminate request.\n";
			return false;
		}
		else {
			std::cout << "MINT req for $" << amount << " verified\n";
		}
	}
	return true;
}

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
			// std::cout << p << " ";
		}
}

std::vector<std::pair<uint32_t, uint32_t>> RSA::generateInversePairs(uint32_t N)
{
	std::vector<std::pair<uint32_t, uint32_t>> keys;
	// find e and d
	for(int i = 3; i < N; i+=2)
	{
		if(1 == std::gcd(i, N)) {
			int j = 2;
			bool flag = false;
			while(!flag && j<N) {
				if(1 == (i*j) % N) {
					flag = true;
				}
				else j++;
			}

			if (flag) {
				keys.push_back(std::make_pair(i, j));
				// std::cout << "pair : " << i << " " << j << std::endl;
			}
			if (keys.size() > 15) break;
			usleep(1000);
		}
	}
	std::cout << "generating blind factors: " << keys.size() << " " << N << std::endl;
	return keys;
}

void RSA::generateEncryptionKey()
{
	uint32_t p, q, phi;
	p = primes[arc4random_uniform(primes.size()-201) + 200];
	do {
		q = primes[arc4random_uniform(primes.size()-301) + 300];
	} while (p == q);

	// std::cout <<"values : " << p << " " << q << std::endl;

	n = p*q;
	phi = (p-1)*(q-1);
	
	std::cout << "n : " << n << " " << " phi : " << phi << std::endl;

	// find e and d
	int r = arc4random_uniform(30)+1;
	for(int i = 2; i < phi && r; i++)
	{
		if(1 == std::gcd(i, phi)) {
			int j = 2;
			bool flag = false;
			while(!flag && j<phi) {
				if(1 == (i*j) % phi) {
					flag = true;
				}
				else j++;
			}
			
			if (flag) {
				e = i; d = j;
				r--;
			}
		}
	}
	std::cout << "keys : " << e << " " << d << std::endl;
}

uint32_t RSA::bigMod(uint32_t a, uint32_t pow, uint32_t N)
{
	if(pow == 0) return 1;
	
	uint64_t res = bigMod(a, pow/2, N)%N;
	res = (res*res)%N;
	
	if(pow%2 == 1) res = (res*a)%N;
	return (uint32_t) res;
}

uint32_t* RSA::encryptMessage(uint32_t* m, int len, uint32_t N)
{
	uint32_t c[len];
	
	for (int i=0; i<len; i++) {
		c[i] = bigMod(m[i], e, N);
	}

	return c;
}

uint32_t* RSA::decryptMessage(uint32_t* c, int len, uint32_t N)
{
	uint32_t m[len];
	
	for (int i=0; i<len; i++) {
		uint32_t res = bigMod((uint32_t) c[i], d, N);
		m[i] = res;
	}

	return m;
}

std::pair<uint32_t, uint32_t> RSA::blindFold(uint32_t* src, uint32_t* dest, int len)
{
	while (blindFactors.empty()) {
		usleep(1000);
	}

	blindFactPos %= blindFactors.size();
	auto factors = blindFactors[blindFactPos++];

	// std::cout << "test : " << bigMod(3, 25, 100) << std::endl;
	
	// bank N
	std::cout << "choosen factor : " << factors.first << " " << factors.second << std::endl;
	uint32_t f = bigMod(factors.first, bankE, bankN);
	std::cout << "test# k: " << factors.first << " e: " << bankE << " n" << bankN << " res: " << f << std::endl;
	std::cout << "k^e %n " << f << std::endl;

	puts("encoded data: ");
	for (int i=0; i<len; i++) {
		uint64_t tem = (src[i]*f) % bankN;
		dest[i] = (uint32_t) tem;
		std::cout << src[i] << "," << dest[i] << " ";
	}
	puts("");

	return factors;
}

uint32_t* RSA::extractOrgSign(uint32_t* data, int len, uint32_t kInv, uint32_t N)
{
	std::cout << "extract inv: " << kInv << std::endl;
	for (int i=0; i<len; i++) {
		data[i] = (data[i]*kInv)%N;
	}

	return data;
}

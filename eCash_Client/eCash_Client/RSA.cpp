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

std::vector<std::pair<uint32_t, uint32_t>> RSA::generateInversePairs(uint32_t N)
{
	std::vector<std::pair<uint32_t, uint32_t>> keys;
	// find e and d
	for(uint32_t i = 3; i < N; i+=2)
	{
		if(1 == std::gcd(i, N)) {

			uint32_t x, y;
			if (1 == gcdExtended(i, N, &x, &y))
			{
				// N is added to handle negative x
				uint32_t res = (x%N + N) % N;
				if (1 == (i*res)%N) {
					keys.push_back(std::make_pair(i, res));
				}
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
	uint64_t remainder, answer = 1;

	remainder = a%N;
	
	while(pow!=0)
	{
		if(pow%2==1)
		{
			answer = (answer*remainder)%N;
		}
		
		remainder = (remainder*remainder)%N;
		
		pow /= 2;
	}
	return (uint32_t) answer;
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

	// bank N
	std::cout << "choosen factor : " << factors.first << " " << factors.second << std::endl;
	uint32_t f = bigMod(factors.first, bankE, bankN)%bankN;

	for (int i=0; i<len; i++) {
		uint64_t tem = modMultiplication(src[i], f, bankN);
		dest[i] = (uint32_t) tem;
	}

	return factors;
}

uint32_t* RSA::extractOrgSign(uint32_t* data, int len, uint32_t kInv, uint32_t N)
{
	for (int i=0; i<len; i++) {
		data[i] = modMultiplication(data[i], kInv, N); // (data[i]*kInv)%N;
	}

	return data;
}

uint32_t RSA::modMultiplication(uint32_t a, uint32_t b, uint32_t mod)
{
	uint32_t res = 0;
	a %= mod;
	
	while (b)
	{
		if (b & 1)
			res = (res + a) % mod;
	
		a = (2 * a) % mod;
		b >>= 1;
	}

	return res;
}

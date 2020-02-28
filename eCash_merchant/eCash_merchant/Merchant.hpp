//
//  Merchant.hpp
//  eCash_merchant
//
//  Created by Raisa Islam on 21/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#ifndef Merchant_hpp
#define Merchant_hpp

#include <stdio.h>
#include <map>

#include "eCashDTO.hpp"
#include "ReqBuilder.hpp"
#include "Thread.h"

class Merchant : public Thread{
	
public:
	static Merchant* getInstance();
	static void resetInstance();
	
	void runner();
	void userInput(std::string str);
	
private:
	Merchant();
	~Merchant();

	int receiveData();
	int sendData(uint8_t* data, int len, int sockFD);

	int processCash(uint32_t* data, int len);
	int checkBitCommitment(uint8_t* data, int len);
	
	int depositCash(uint32_t uID, bool cheat = false);
	
private:
	static Merchant* instance;
	
	std::map<uint32_t, std::unique_ptr<eCash>> eCashPool;

	uint32_t accID{12348};
	int sockFD{-1};

	uint32_t bankN{0};
	uint32_t bankE{0};
	ReqBuilder reqBuild;
};


#endif /* Merchant_hpp */

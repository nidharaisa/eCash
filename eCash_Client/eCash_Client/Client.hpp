//
//  Client.hpp
//  eCash_Client
//
//  Created by Raisa Islam on 19/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#ifndef Client_hpp
#define Client_hpp

#include <stdio.h>
#include <map>

#include "eCashDTO.hpp"
#include "Thread.h"
#include "ReqBuilder.hpp"

class Client : public Thread{

public:
	static Client* getInstance();
	static void resetInstance();
	
	void runner();
	void test();
	void userInput(std::string str);
	
private:
	Client();
	~Client();
	
	int receiveData();
	int sendData(uint8_t* data, int len, int port);
	int sendBlindReq(int num);
	int sendBlindFactors(uint8_t toBeSigned);
	int processSignedData(uint8_t* buff, int len);
	int transferCash(uint32_t uID, bool cheat = false);
	int depositCash(uint32_t uID, bool cheat);

private:
	static Client* instance;
	
	std::map<uint32_t, std::unique_ptr<eCash> > eCashPool;
	std::vector<std::unique_ptr<eCash>> tempCashPool;

	uint32_t mintReq{0};
	
	uint32_t accID{12345};
	int sockFD{-1};

	uint32_t bankN{0};
	uint32_t bankE{0};
	
	ReqBuilder reqBuild;
};


#endif /* Client_hpp */

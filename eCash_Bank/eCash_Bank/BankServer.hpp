//
//  BankServer.hpp
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#ifndef BankServer_hpp
#define BankServer_hpp

#include <stdio.h>
#include <map>

#include "Thread.h"
#include "ReqBuilder.hpp"

struct AccountDetail {
public:
	AccountDetail(uint32_t id, int balance, std::string name) {
		this->accountId = id;
		this->balance = balance;
		this->name = name;
	}

public:
	uint32_t accountId{0};
	int balance{0};
	int askedReq{-1};
	int mintAMount{0};
	uint8_t toBeSignedIndex{0};

	std::string name{""};
	std::vector<std::unique_ptr<eCash>> blindCash;

};

class BankServer : public Thread {

public:
	static BankServer* getInstance();
	static void resetInstance();

	void runner();
	
	bool createAccount(uint32_t id, std::string name, int balance);

private:
	BankServer();
	~BankServer();
	
	void receiver();
	int receiveData();
	void updateKeys();
	int sendData(uint8_t* data, size_t len, struct sockaddr_in &toAddr);
	
	int processBlindReq(uint8_t* src, uint8_t* dest, int len);
	int processBlindfactorReq(uint8_t* buff, uint8_t* res, int len);
	int processDepositReq(uint8_t* buff, int len);

private:
	static BankServer* instance;
	
	int serverSock{-1};
	uint8_t readBuff[1024];

	uint32_t n;
	uint32_t e, d;
	
	ReqBuilder reqBuild;

	// customer accounts
	std::map<uint32_t, std::unique_ptr<AccountDetail>> customers;
	// uid, ecash
	std::map<uint32_t, std::unique_ptr<eCash>> ecashPool;
	
};

#endif /* BankServer_hpp */

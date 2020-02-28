//
//  Merchant.cpp
//  eCash_merchant
//
//  Created by Raisa Islam on 21/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>

#include "Merchant.hpp"
#include "RSA.hpp"

#define DATA_PORT 17000
#define DATA_SIZE 1024
#define BANK_PORT 8888
#define CLIENT_PORT 9999

Merchant* Merchant::instance = nullptr;

Merchant::Merchant()
{
	RSA::getInstance();

	struct sockaddr_in servaddr;
	
	// Creating socket file descriptor
	if ((sockFD = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
		// Filling server information
	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(DATA_PORT);
	
		// Bind the socket with the server address
	if(::bind(sockFD, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
		{
		perror("bind failed");
		exit(EXIT_FAILURE);
		}
	
	int enable = 1;
	setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	printf("sockFD : %d\n", sockFD);
}

Merchant::~Merchant()
{
	puts("destroy server");
	stop();
	join();

	::close(sockFD);
}

// static
Merchant* Merchant::getInstance()
{
	if (!instance) {
		instance = new Merchant();
	}
	return instance;
}

	// static
void Merchant::resetInstance()
{
	if (instance) {
		delete instance;
		instance = nullptr;
	}
}

void Merchant::runner()
{
	puts("started thread");
	while (isRunning()) {
		receiveData();
		usleep(1000);
	}
}

int Merchant::receiveData()
{
	size_t readlen = 0;
	int recvBuffLen = 2048;
	uint8_t recvBuff[recvBuffLen];

	struct sockaddr_in fromaddr;
	socklen_t fromlen = sizeof(fromaddr);
	
	// puts("wait for data");
	readlen = ::recvfrom(sockFD, (uint8_t*) recvBuff, recvBuffLen, 0, (struct sockaddr*) &fromaddr, &fromlen);
	
	if (readlen > 0) {
		std::cout << "received data from port: " << ntohs(fromaddr.sin_port) << std::endl;
	}
	
	if (ntohs(fromaddr.sin_port) == BANK_PORT) {
		std::cout << "From bank: " << (int) recvBuff[0] << " len : " << readlen << std::endl;
		switch (recvBuff[0]) {
			
			default: {
				break;
			}
		}
		
	}
	else if (ntohs(fromaddr.sin_port) == CLIENT_PORT) {
		// received client data
		std::cout << "From client: " << (int) recvBuff[0] << " len : " << readlen << std::endl;
		switch (recvBuff[0]) {
			case 30: {
				// received cash
				if(processCash((uint32_t*) &recvBuff[1], (readlen-1)/4)) {
					
				}
				
				break;
			}
			case 32: {
				
				bool ok = checkBitCommitment(&recvBuff[1], readlen-1);
				std::cout << "checked commit: " << ok << std::endl;

				break;
			}
			default: {
				break;
			}
		}
	}

	return 0;
}

int Merchant::sendData(uint8_t* data, int len, int port)
{
	struct sockaddr_in toaddr;
	
	bzero(&toaddr, sizeof(toaddr));
	toaddr.sin_addr.s_addr = INADDR_ANY;
	toaddr.sin_port = htons(port);
	toaddr.sin_family = AF_INET;
	
	return ::sendto(sockFD, data, len, MSG_DONTWAIT, (const struct sockaddr*) &toaddr, sizeof(toaddr));
}

int Merchant::processCash(uint32_t *data, int len)
{
	int ind = 0;

	uint32_t uID = data[ind++];
	uint32_t amount = data[ind++];

	auto cash = std::make_unique<eCash>(0, amount);
	cash->uniqueId = uID;

	std::cout << "uID: " << uID << " am: " << amount << std::endl;

	int sz = data[ind++];
	for(int i=0; i<sz; i++) {
		uint32_t l = data[ind++];
		uint32_t r = data[ind++];
		cash->hID.push_back(std::make_pair(l, r));
	}

	int cp = cash->signSz = data[ind++];
	cp *= 4;
	
	for (int k=0; k<cash->signSz; k++) {
		std::cout << data[ind+k] << std::endl;
	}
	
	memcpy(&cash->sign, &data[ind], cp);
	ind += cash->signSz;

	cash->commit = (uint8_t) arc4random();

	std::cout << "commit: " << (int) cash->commit << std::endl;

	uint8_t buff[100];
	int ret = reqBuild.createCommitReq(buff, cash->commit, uID);
	
	eCashPool[uID] = std::move(cash);
	return sendData(buff, ret, CLIENT_PORT);
}

void Merchant::userInput(std::string str)
{
	if ("" == str) return ;

	if ("get pkey" == str) {
		uint8_t req[10];
		int len = reqBuild.createPublicInfoReq(req);
		sendData(req, len, BANK_PORT);
		return ;
	}
	
	std::vector<std::string> input;
	std::stringstream iss(str);
	std::string action;
	
	while (iss >> action) {
		input.push_back(action);
	}

	if ("show" == input[0]) {
		for(auto iter = eCashPool.begin(); iter != eCashPool.end(); iter++){
			std::cout << "available cash: " << iter->first << " " << iter->second->amount << std::endl;
		}
	}
	else if("deposit" == input[0]) {

		uint32_t uid = static_cast<uint32_t>(std::stoul(input[1]));
		bool cheat = false;

		if (input.size() > 2) {
			cheat = (input[2] != "n");
		}

		depositCash(uid, cheat);
	}
}

int Merchant::checkBitCommitment(uint8_t *data, int len)
{
	int ind = 0;
	uint32_t uID;
	std::memcpy(&uID, &data[0], 4);
	ind += 4;

	if(eCashPool.find(uID) != eCashPool.end()) {
		
		uint8_t sz = data[ind++]; // sz in uint8_t
		uint32_t* tem = (uint32_t*) &data[ind];
		int j = 0;

		for (int i=0; i<sz; i++) {
			eCashPool[uID]->rPairs.push_back(std::make_pair(tem[j], tem[j+1]));
			j += 2;
		}
		return eCashPool[uID]->verifyCommitment();
	}

	return false;
}

int Merchant::depositCash(uint32_t uID, bool cheat)
{
	if (eCashPool.find(uID) == eCashPool.end()) return -1;
	
	uint8_t buff[2048];
	int len = reqBuild.createDepositReq(buff, accID, eCashPool[uID]);
	
	if(!cheat) eCashPool.erase(uID);

	return sendData(buff, len, BANK_PORT);

}

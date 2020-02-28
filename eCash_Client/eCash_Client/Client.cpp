//
//  Client.cpp
//  eCash_Client
//
//  Created by Raisa Islam on 19/2/20.
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

#include "Client.hpp"
#include "RSA.hpp"

#define DATA_PORT 9999
#define DATA_SIZE 1024
#define BANK_PORT 8888
#define MERCHANT_PORT 17000

Client* Client::instance = nullptr;
static int mint_cheat = 0;

Client::Client()
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
	
	// createSockets();
}

Client::~Client()
{
	puts("destroy server");
	stop();
	join();
	
	// ::close(bankSock);
	::close(sockFD);
}

// static
Client* Client::getInstance()
{
	if (!instance) {
		instance = new Client();
	}
	return instance;
}

// static
void Client::resetInstance()
{
	if (instance) {
		delete instance;
		instance = nullptr;
	}
}

void Client::runner()
{
	puts("started thread");
	while (isRunning()) {
		receiveData();
		usleep(1000);
	}
}

int Client::receiveData()
{
	
	size_t readlen = 0;
	int recvBuffLen = 2048;
	uint8_t recvBuff[recvBuffLen];

	struct sockaddr_in fromaddr;
	socklen_t fromlen = sizeof(fromaddr);

	// puts("wait for data");
	readlen = ::recvfrom(sockFD, (uint8_t*) recvBuff, recvBuffLen, 0, (struct sockaddr*) &fromaddr, &fromlen);

	if (readlen > 0) {
		// std::cout << "received data from port: " << ntohs(fromaddr.sin_port) << std::endl;
	}

	if (ntohs(fromaddr.sin_port) == BANK_PORT) {
		// received Bank data
		std::cout << "From bank: " << (int) recvBuff[0] << " len : " << readlen << std::endl;
		switch (recvBuff[0]) {
			case 11: { // REQ_PUBLIC_INFO_RESPONSE
				uint32_t* tem = (uint32_t*) &recvBuff[1];
				bankN = RSA::getInstance()->bankN = tem[0];
				bankE = RSA::getInstance()->bankE = tem[1];
				std::cout << "bank n: " << bankN << " e: " << bankE << std::endl;
				RSA::getInstance()->blindFactors = RSA::getInstance()->generateInversePairs(bankN);
				break;
			}
			case 21: { // REQ_ASK_MINT_N_BLIND_REQ
				int len = sendBlindReq(recvBuff[1]);
				std::cout << "sent " << (int) recvBuff[1] << " blind req. sz : " << len << std::endl;
				break;
			}
			case 23: {
				// send blind factors
				std::cout << "wants to sign : " << (int) recvBuff[1] << std::endl;
				sendBlindFactors(recvBuff[1]);
				break;
			}
			case 25: {
				std::cout << "received signed data\n";
				processSignedData(&recvBuff[1], readlen-1);
				break;
			}
			default: {
				break;
			}
		}
		
	}
	else if (ntohs(fromaddr.sin_port) == MERCHANT_PORT) {
		// received merchant data
		std::cout << "From Merchant: " << (int) recvBuff[0] << " len : " << readlen << std::endl;
		switch (recvBuff[0]) {
			case 31: {
				uint8_t com = recvBuff[1];
				uint32_t uID;
				std::memcpy(&uID, &recvBuff[2], 4);
				uint8_t buff[1024];
				int len = reqBuild.createBitCommitment(buff, uID, eCashPool[uID]->bitCommitment(com));
				sendData(buff, len, MERCHANT_PORT);
				break;
			}
			default: {
				std:: cout << "recv: " << recvBuff << std::endl;
				break;
			}
		}
	}

	return 0;
}

int Client::sendData(uint8_t* data, int len, int port)
{
	struct sockaddr_in toaddr;

	bzero(&toaddr, sizeof(toaddr));
	toaddr.sin_addr.s_addr = INADDR_ANY;
	toaddr.sin_port = htons(port);
	toaddr.sin_family = AF_INET;

	return ::sendto(sockFD, data, len, MSG_DONTWAIT, (const struct sockaddr*) &toaddr, sizeof(toaddr));
}

void Client::userInput(std::string str)
{
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

	if ("mint" == input[0]) {
		
		uint8_t req[100];

		mintReq = std::atoi(input[1].c_str());
		std::cout << "mint " << accID << " amount : " << mintReq << std::endl;
		
		if (input.size() > 2) {
			mint_cheat = 5;
		}

		tempCashPool.clear();
		int len = reqBuild.createMintReq(accID, mintReq, req);
		sendData(req, len, BANK_PORT);
	}
	else if ("show" == input[0]) {
		for(auto iter = eCashPool.begin(); iter != eCashPool.end(); iter++){
			std::cout << "available cash: " << iter->first << " " << iter->second->amount << std::endl;
		}
	}
	else if("transfer" == input[0]) {

		uint32_t uid = static_cast<uint32_t>(std::stoul(input[1]));
		
		bool cheat = false;
		if (input.size() > 2) {
			cheat = (input[2] != "n");
		}

		transferCash(uid, cheat);
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

int Client::sendBlindReq(int num)
{
	// create n ecash
	for (int i=0; i<num; i++) {
		auto cash = std::make_unique<eCash>(accID, mintReq);
		tempCashPool.push_back(std::move(cash));
	}
	
	if (mint_cheat > 0) {
		int rand = arc4random_uniform(num-2);
		tempCashPool[rand]->amount += mint_cheat;
		mint_cheat = 0;
		std::cout << "CHEAT amount: " << tempCashPool[rand]->amount << " for " << rand << " req index\n";
	}

	// send reqs
	uint8_t buff[2096];
	int len = reqBuild.createBlindReq(buff, accID, tempCashPool);
	return sendData(buff, len, BANK_PORT);
}

int Client::sendBlindFactors(uint8_t toBeSigned)
{
	uint8_t buff[2048];
	int len = reqBuild.createMintSendBlindFactors(buff, accID, toBeSigned, tempCashPool);
	return sendData(buff, len, BANK_PORT);
}

void Client::test()
{

	uint8_t req[10];
	int len = reqBuild.createPublicInfoReq(req);
	std::cout << "sent : " << sendData(req, len, BANK_PORT) << std::endl;
	
	return;
	auto cash = std::make_unique<eCash>(accID, 100);

	puts("start test");
	uint32_t m[1024];

	int ret = cash->prepareForSign(m);
	std::cout << "data sz : " << ret << std::endl;
	
	// send data
}

int Client::processSignedData(uint8_t *buff, int len)
{
	int signedInd = buff[0];
	std::cout << "signed : " << signedInd << " " << tempCashPool[signedInd]->uniqueId << std::endl;

	// THIS IS DIGITAL_SIGN. NOT TO decode

	tempCashPool[signedInd]->signLen = (len-1)/4;
	uint32_t *temp =  RSA::getInstance()->extractOrgSign((uint32_t*) &buff[1], tempCashPool[signedInd]->signLen, tempCashPool[signedInd]->blindFactor.second, bankN);
	memcpy(&tempCashPool[signedInd]->sign, temp, len-1);

	eCashPool[tempCashPool[signedInd]->uniqueId] = std::move(tempCashPool[signedInd]);
	tempCashPool.clear();

	return true;
}

int Client::transferCash(uint32_t uID, bool cheat)
{
	if (eCashPool.find(uID) == eCashPool.end()) return -1;

	uint8_t buff[2048];
	int len = reqBuild.createTransferBalance(buff, accID, eCashPool[uID]);
	// if(!cheat) eCashPool.erase(uID);

	return sendData(buff, len, MERCHANT_PORT);
}


int Client::depositCash(uint32_t uID, bool cheat)
{
	if (eCashPool.find(uID) == eCashPool.end()) return -1;

	uint8_t com = (uint8_t) arc4random();
	auto v = eCashPool[uID]->bitCommitment(com);
	
	uint8_t buff[2048];
	int len = reqBuild.createDepositReq(buff, accID, eCashPool[uID], com, v);

	if(!cheat) eCashPool.erase(uID);
	
	return sendData(buff, len, BANK_PORT);
	
}

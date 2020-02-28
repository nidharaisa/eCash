//
//  BankServer.cpp
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "BankServer.hpp"
#include "RSA.hpp"

#define SERVER_PORT 8888
#define DATA_SIZE 1024

BankServer* BankServer::instance = nullptr;

BankServer::BankServer()
{
	struct sockaddr_in servaddr;
	
	// Creating socket file descriptor
	if ((serverSock = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
	// Filling server information
	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(SERVER_PORT);

	// Bind the socket with the server address
	if(::bind(serverSock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	int enable = 1;
	setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	printf("sockFD : %d\n", serverSock);

	updateKeys();

}

BankServer::~BankServer()
{
	stop();
	join();
	::close(serverSock);
}

// static
BankServer* BankServer::getInstance()
{
	if (!instance) {
		instance = new BankServer();
	}
	return instance;
}

// static
void BankServer::resetInstance()
{
	puts("destroy server");
	if (instance) {
		delete instance;
		instance = nullptr;
	}
}

bool BankServer::createAccount(uint32_t id, std::string name, int balance)
{
	if(customers.find(id) != customers.end())
		return false;

	customers[id] = std::make_unique<AccountDetail>(id, balance, name);
	return true;
}

void BankServer::updateKeys()
{
	n = RSA::getInstance()->n;
	e = RSA::getInstance()->e;
	d = RSA::getInstance()->d;
}

void BankServer::runner()
{
	receiver();
}

void BankServer::receiver()
{
	puts("started thread");
	while (isRunning()) {
		receiveData();
		usleep(1000);
	}
}

int BankServer::receiveData()
{
	struct sockaddr_in fromaddr;
	socklen_t fromlen = sizeof(fromaddr);

	uint16_t readlen = -1;

	readlen = ::recvfrom(serverSock, (uint8_t*) readBuff, DATA_SIZE, 0, (struct sockaddr*) &fromaddr, &fromlen);
	readBuff[readlen] = '\0';
	if(readlen > 0) {
		printf("Client : %d. readlen: %d\n", readBuff[0], readlen);
		
		uint8_t buff[2048];
		int len = 0;
		
		switch (readBuff[0]) {
			case 10: {
				len = reqBuild.createPublicInfoResponse(buff);
				break;
			}
			case 20: {
				// verify mint req
				uint32_t* tem = (uint32_t*) &readBuff[1];
				uint32_t id = tem[0];
				uint32_t amount = tem[1];

				if (customers.find(id) != customers.end()) {
					// found account
					if (customers[id]->balance >= amount) {
						customers[id]->blindCash.clear();
						customers[id]->askedReq = arc4random_uniform(10)+3;
						customers[id]->mintAMount = amount;

						len = reqBuild.createMintAskReq(buff, customers[id]->askedReq);
					}
					std::cout <<"MINT from " << customers[id]->name << " " <<	customers[id]->balance << " amount: " << amount << " should send: " << customers[id]->askedReq << std::endl;
				}
				break;
			}
			case 22: {
				
				std::cout << "received blind reqs\n";
				len = processBlindReq(&readBuff[1], buff, readlen-1);
				break;
			}
			case 24 : {
				std::cout << "received blind factors " << readlen << "\n";
				len = processBlindfactorReq(&readBuff[1], buff, readlen-1);
				break;
			}
			case 40: { // deposit req
				std::cout << "recv deposit req\n";
				processDepositReq(&readBuff[1], readlen-1);
				
				break;
			}
			default:
				printf("recv : %d\n", readBuff[0]);
				break;
		}
		if (len > 0) {
			int ret = sendData(buff, len, fromaddr);
		}
	}
	return readlen;
}

int BankServer::sendData(uint8_t *data, size_t len, struct sockaddr_in &toAddr)
{
	return ::sendto(serverSock, data, len, MSG_DONTWAIT, (const struct sockaddr*) &toAddr, sizeof(toAddr));
}

int BankServer::processBlindReq(uint8_t *src, uint8_t* dest, int len)
{
	int index = 0;
	uint32_t acc;

	std::memcpy(&acc, src, 4);
	index += 4;

	if(customers.find(acc) == customers.end()) return false;

	int vSize = src[index++];
	if(customers[acc]->askedReq != vSize) return false;

	for(int i=0; i<vSize; i++) {

		auto cash = std::make_unique<eCash>(acc, (uint32_t) customers[acc]->mintAMount);

		int sz = (int) src[index++];

		// 32bit data, sz
		std::memcpy(cash->rawData, &src[index++], sz*4);
		cash->rawDataSz = sz;

		index += (sz*4)-1;
		customers[acc]->blindCash.push_back(std::move(cash));
	}

	customers[acc]->toBeSignedIndex = arc4random_uniform(vSize);
	std::cout << "will sign : " << (int) customers[acc]->toBeSignedIndex << std::endl;
	return reqBuild.createMintAskBlindFactors(dest, customers[acc]->toBeSignedIndex);
}

int BankServer::processBlindfactorReq(uint8_t *buff, uint8_t *res, int len)
{
	uint32_t acc, index = 0;
	
	memcpy(&acc, buff, 4);
	index += 4;
	
	if (customers.find(acc) == customers.end())
		return -1;
	
	int sz = (int) buff[index++];
	if(sz != customers[acc]->blindCash.size()) {
		std::cout << "factor size MISMATCH : " << sz << " " << customers[acc]->blindCash.size() << std::endl;
		return -1;
	}

	std::vector<std::pair<uint32_t, uint32_t>> blindFactors;
	uint32_t* tem = (uint32_t*) &buff[index];

	int j = 0;
	for (int i=0; i<sz; i++) {
		blindFactors.push_back(std::make_pair(tem[j], tem[j+1]));
		j += 2;
	}

	if(!RSA::getInstance()->blindUnfold(customers[acc]->blindCash, customers[acc]->toBeSignedIndex, blindFactors, customers[acc]->mintAMount))
		return -1;

	auto &cash = customers[acc]->blindCash[customers[acc]->toBeSignedIndex];
	len = reqBuild.createSignedData(cash->rawData, res, cash->rawDataSz, customers[acc]->toBeSignedIndex);

	customers[acc]->blindCash.clear();
	customers[acc]->balance -= customers[acc]->mintAMount;
	customers[acc]->mintAMount = 0;

	return len;
}

int BankServer::processDepositReq(uint8_t *buff, int len)
{
	int ind = 0;

	uint32_t* tem = (uint32_t*) &buff[0];

	uint32_t acc = tem[ind++];

	if(customers.find(acc) == customers.end()) {
		std::cout << "account not found\n";
		return -1;
	}
	
	uint32_t uID = tem[ind++];
	uint32_t amount = tem[ind++];
	uint32_t hidSz = tem[ind++];
	
	std::cout << "recv deposit req. uID: " << uID << " am: " << amount << "hids: " << hidSz << std::endl;

	auto cash = std::make_unique<eCash>(0, amount);
	cash->uniqueId = uID;

	for (int i=0; i<hidSz; i++) {
		cash->hID.push_back(std::make_pair(tem[ind], tem[ind+1]));
		ind += 2;
	}
	
	cash->signSz = tem[ind++];
	std::memcpy(&cash->sign, &tem[ind], cash->signSz*4);
	ind += cash->signSz;

	uint32_t sdata[50], dest[50];
	int ret = cash->prepareDataToSign(sdata);
	ret = RSA::getInstance()->digitalSign(sdata, dest, ret);

	if(0 != std::strncmp((char*) &dest, (char*) &cash->sign, ret*4)) {
		std::cout << "SIGN DIDN'T MATCH\n";
		return -1;
	}

	for (int i=0; i<8; i++) {
		cash->rPairs.push_back(std::make_pair(tem[ind], tem[ind+1]));
		ind += 2;
	}

	cash->commit = buff[len-1];
	
	if(ecashPool.find(uID) == ecashPool.end()) {
		ecashPool[uID] = std::move(cash);
		customers[acc]->balance += amount;
		
		std::cout << "deposit SUCCESSFUL\n";
		return true;
	}
	else {
		// cheating
		auto &pre = ecashPool[uID];
		uint32_t addr = pre->verifyCheater(cash->commit, cash->rPairs);
		if(0 == addr) {
			std::cout << "Depositer is CHEATING\n";
		}
		else {
			std::cout << addr << " is CHEATING\n";
		}
	}

	return len;
}

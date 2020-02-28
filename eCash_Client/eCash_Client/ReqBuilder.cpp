//
//  ReqBuilder.cpp
//  eCash_Client
//
//  Created by Raisa Islam on 21/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#define SIGN_ANYWAY				01
#define REQ_PUBLIC_INFO 			10
#define REQ_PUBLIC_INFO_RESPONSE		11
#define REQ_MINT_INIT				20 // client tries to mint
#define REQ_MINT_ASK_N_BLIND_REQ		21 // bank asks for n blind requests
#define REQ_MINT_SEND_N_BLIND_REQ	22 // client sends n blind requests
#define REQ_MINT_ASK_BLIND_FACT		23 // bank asks for n blind factors
#define REQ_MINT_SEND_BLIND_FACT		24 // client sends n blind factors
#define REQ_MINT_SIGNED				25 // bank sends digital sign
#define REQ_TRANSFER				30 // trannsfer balance
#define REQ_TRANSFER_COMMIT_SEQ		31 // send rand seq for bit commitment
#define REQ_TRANSFER_COMMIT_VAL		32 // send <l, lr> or <r, rr> pairs

#define DEPOSIT_CASH				40 //

#include <iostream>

#include "RSA.hpp"
#include "ReqBuilder.hpp"

// REQ_PUBLIC_INFO 10
int ReqBuilder::createPublicInfoReq(uint8_t* req)
{
	req[0] = REQ_PUBLIC_INFO;
	return 1;
}
// REQ_PUBLIC_INFO_RESPONSE 11
int ReqBuilder::createPublicInfoResponse(uint8_t* res)
{
	res[0] = REQ_PUBLIC_INFO_RESPONSE;
	uint32_t* tem = (uint32_t*) &res[1];
	
	tem[0] = RSA::getInstance()->n;
	tem[1] = RSA::getInstance()->e;

	return 9;
}
// REQ_MINT_INIT 20
int ReqBuilder::createMintReq(uint32_t accID, uint32_t amount, uint8_t *req)
{
	int sz = 0;
	uint32_t* tem = (uint32_t*) &req[1];

	req[0] = REQ_MINT_INIT;
	sz++;

	tem[0] = accID;
	sz += 4;

	tem[1] = amount;
	sz += 4;

	return sz;
}
// REQ_MINT_ASK_N_BLIND_REQ		21
int ReqBuilder::createMintAskReq(uint8_t *req)
{
	int index = 0;
	req[index++] = REQ_MINT_ASK_N_BLIND_REQ;
	req[index++] = arc4random_uniform(10)+2;

	return index;
}

// REQ_MINT_SEND_N_BLIND_REQ 22
int ReqBuilder::createBlindReq(uint8_t *req, uint32_t &acc, std::vector<std::unique_ptr<eCash>> &cashPool)
{
	int index = 0;
	req[index++] = REQ_MINT_SEND_N_BLIND_REQ;
	std::memcpy(&req[index], &acc, 4);
	index += 4;
	req[index++] = cashPool.size();

	for (int i=0; i<cashPool.size(); i++) {
		// req[index++] = i+1;
		int ret = cashPool[i]->prepareForSign((uint32_t*) &req[index+1]);
		req[index] = (uint8_t) ret;
		index += (ret*4)+1;
	}
	return index;
}

// REQ_MINT_SEND_BLIND_FACT 24
int ReqBuilder::createMintSendBlindFactors(uint8_t *req, uint32_t &acc, uint8_t toBeSigned, std::vector<std::unique_ptr<eCash>> &cashPool)
{
	int index = 0;
	req[index++] = REQ_MINT_SEND_BLIND_FACT;
	std::memcpy(&req[index], &acc, 4);
	index += 4;
	req[index++] = cashPool.size();

	uint32_t* tem = (uint32_t*) &req[index];

	int j = 0;
	for (int i=0; i<cashPool.size(); i++) {
		tem[j++] = (i!= (int)toBeSigned) ? cashPool[i]->blindFactor.first : 0;
		tem[j++] = (i!= (int)toBeSigned) ? cashPool[i]->blindFactor.second : 0;
		index += 8;
	}

	return index;
}
// REQ_TRANSFER 30
int ReqBuilder::createTransferBalance(uint8_t *buff, uint32_t &acc, std::unique_ptr<eCash> &cash)
{
	int ind = 0;
	buff[ind++] = REQ_TRANSFER;

	int len = cash->prepareRawData((uint32_t*) &buff[ind]);
	ind += (len*4);

	return ind;
}
// REQ_TRANSFER_COMMIT_VAL 32
int ReqBuilder::createBitCommitment(uint8_t* buff, uint32_t uId, std::vector<std::pair<uint32_t, uint32_t> > v)
{
	int ind = 0;
	buff[ind++] = REQ_TRANSFER_COMMIT_VAL;
	
	std::memcpy(&buff[ind], &uId, 4);
	ind += 4;

	buff[ind++] = v.size();

	uint32_t *tem = (uint32_t*) &buff[ind];
	int j = 0;
	for (int i=0; i<v.size(); i++) {
		tem[j++] = v[i].first;
		tem[j++] = v[i].second;
	}

	ind += (j*4);

	return ind;
}

	//
int ReqBuilder::createDepositReq(uint8_t *buff, uint32_t acc, std::unique_ptr<eCash> &cash, uint8_t commit, std::vector<std::pair<uint32_t, uint32_t>> &rPairs)
{
	int ind = 0;
	buff[ind++] = DEPOSIT_CASH;
	std::memcpy(&buff[ind], &acc, 4);
	ind += 4;

	uint32_t* tem = (uint32_t*) &buff[ind];
	int k = cash->prepareRawData(tem);

	// uint8_t com = (uint8_t) arc4random();

	// add rPairs
	for (int i=0; i< rPairs.size(); i++) {
		tem[k++] = rPairs[i].first;
		tem[k++] = rPairs[i].second;
	}
	ind += (k*4);

	buff[ind++] = commit;
	
	return ind;
}

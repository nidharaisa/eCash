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

// REQ_TRANSFER_COMMIT_SEQ 31
int ReqBuilder::createCommitReq(uint8_t *buff, uint8_t commit, uint32_t uID)
{
	buff[0] = REQ_TRANSFER_COMMIT_SEQ;
	buff[1] = commit;

	memcpy(&buff[2], &uID, 4);

	return 6;
}
//
int ReqBuilder::createDepositReq(uint8_t *buff, uint32_t acc, std::unique_ptr<eCash> &cash)
{
	int ind = 0;
	buff[ind++] = DEPOSIT_CASH;
	std::memcpy(&buff[ind], &acc, 4);
	ind += 4;

	uint32_t* tem = (uint32_t*) &buff[ind];
	int k = cash->prepareRawData(tem);

	// add rPairs
	for (int i=0; i<cash->rPairs.size(); i++) {
		tem[k++] = cash->rPairs[i].first;
		tem[k++] = cash->rPairs[i].second;
	}
	ind += (k*4);
	
	buff[ind++] = cash->commit;

	return ind;
}

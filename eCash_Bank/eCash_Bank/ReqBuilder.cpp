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
int ReqBuilder::createMintReq(uint32_t accID, int amount, uint8_t *req)
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
int ReqBuilder::createMintAskReq(uint8_t *req, uint8_t n)
{
	int index = 0;
	req[index++] = REQ_MINT_ASK_N_BLIND_REQ;
	req[index++] = n;

	return index;
}

// REQ_MINT_ASK_BLIND_FACT 23
int ReqBuilder::createMintAskBlindFactors(uint8_t *req, uint8_t reqNum)
{
	int index = 0;
	req[index++] = REQ_MINT_ASK_BLIND_FACT;
	req[index++] = reqNum;

	return index;
}
// REQ_MINT_SIGNED 25
int ReqBuilder::createSignedData(uint32_t *src, uint8_t *dest, int len32, int signing)
{
	dest[0] = REQ_MINT_SIGNED;
	dest[1] = signing;

	uint32_t* tem = (uint32_t*)&dest[2];

	len32 = RSA::getInstance()->digitalSign(src, tem, len32);

	return (len32*4)+2;
}

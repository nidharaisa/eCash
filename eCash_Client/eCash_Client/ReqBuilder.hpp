//
//  ReqBuilder.hpp
//  eCash_Client
//
//  Created by Raisa Islam on 21/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#ifndef ReqBuilder_hpp
#define ReqBuilder_hpp

#include <stdio.h>
#include <string>

#include "eCashDTO.hpp"

class ReqBuilder {
public:
	inline ReqBuilder(){}
	inline ~ReqBuilder(){}

	int createPublicInfoReq(uint8_t* req);
	int createPublicInfoResponse(uint8_t* res);
	int createMintReq(uint32_t accID, uint32_t amount, uint8_t* req);
	int createMintAskReq(uint8_t* req);
	int createBlindReq(uint8_t* req, uint32_t &acc, std::vector<std::unique_ptr<eCash>> &cashPool);
	int createMintGetBlindFactors(uint8_t* req, std::vector<std::unique_ptr<eCash>> &cashPool);
	int createMintSendBlindFactors(uint8_t* req, uint32_t &acc, uint8_t toBeSigned, std::vector<std::unique_ptr<eCash>> &cashPool);
	
	// merchant-client req
	int createTransferBalance(uint8_t* buff, uint32_t &acc, std::unique_ptr<eCash> &cash);
	int createBitCommitment(uint8_t* buff, uint32_t uId, std::vector<std::pair<uint32_t, uint32_t>> v);
	
	// deposit
	int createDepositReq(uint8_t *buff, uint32_t acc, std::unique_ptr<eCash> &cash, uint8_t commit, std::vector<std::pair<uint32_t, uint32_t>> &rPairs);
};

#endif /* ReqBuilder_hpp */

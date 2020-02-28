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
	int createMintReq(uint32_t accID, int amount, uint8_t* req);
	int createMintAskReq(uint8_t* req, uint8_t n);
	int createBlindReq(uint8_t* req, std::vector<std::unique_ptr<eCash>> cashPool);
	int createMintGetBlindFactors(uint8_t* req, std::vector<std::unique_ptr<eCash>> &cashPool);
	int createMintAskBlindFactors(uint8_t* req, uint8_t reqNum);
	int createSignedData(uint32_t* src, uint8_t* dest, int len, int signing);
};

#endif /* ReqBuilder_hpp */

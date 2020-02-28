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
	
	int createCommitReq(uint8_t* buff, uint8_t commit, uint32_t uID);
	int createDepositReq(uint8_t* buff, uint32_t acc, std::unique_ptr<eCash> &cash);
};

#endif /* ReqBuilder_hpp */

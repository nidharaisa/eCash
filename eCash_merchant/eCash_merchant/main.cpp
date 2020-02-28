//
//  main.cpp
//  eCash_merchant
//
//  Created by Raisa Islam on 21/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <iostream>
#include <string>

#include "Merchant.hpp"

int main(int argc, const char * argv[]) {
	// insert code here...

	Merchant::getInstance()->start();

	std::string str;
	
	while (getline(std::cin, str)) {
		std::cout << "stdin : " << str << std::endl;
		if (str == "stop") {
			break;
		}
		Merchant::getInstance()->userInput(str);
	}

	Merchant::resetInstance();
	
	return 0;
}

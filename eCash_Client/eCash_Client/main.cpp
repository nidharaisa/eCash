//
//  main.cpp
//  eCash_Client
//
//  Created by Raisa Islam on 19/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "Client.hpp"

int main(int argc, const char * argv[]) {
	// insert code here...

	Client::getInstance()->start();

	std::string str;
	std::string action;

	while (getline(std::cin, str)) {
		std::cout << "stdin : " << str << std::endl;
		if (str == "stop") {
			break;
		}
		Client::getInstance()->userInput(str);
	}

	Client::resetInstance();
	return 0;
}

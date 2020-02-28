//
//  main.cpp
//  eCash_Bank
//
//  Created by Raisa Islam on 18/2/20.
//  Copyright © 2020 Raisa Islam. All rights reserved.
//

#include <iostream>
#include <unistd.h>

#include "BankServer.hpp"

int main(int argc, const char * argv[]) {
	// insert code here...

	BankServer::getInstance()->start();
	
	BankServer::getInstance()->createAccount(12345, "Alice", 500);
	BankServer::getInstance()->createAccount(12348, "Bob", 10);

	std:: string str;
	while (std::cin >> str) {

		if(str == "stop") {
			puts("STOP called");
			BankServer::resetInstance();
			break;
		}
		
		usleep(1000);
	}
	
	BankServer::resetInstance();
	return 0;
}

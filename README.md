# eCash

This programming assignment is based on the protocol number 4 (Chapter 6.4 Digital Cash), described in Applied Cryptography, 2nd edition by Schneier; the protocol implements an electronic cash system, in which the digital cash cannot be copied and reused more than once and the privacy of the customer’s identity is guaranteed.

Using your preferred programming language, you need to implement an e-cash system, which allows money transaction between three parties: Customer, Merchant, and Bank. The electronic cash (e-cash) used during these transaction is a file which contains:
* The amount of the transaction involved
* An uniqueness string number
* Identity strings that contain the identity of the customer (this information remains secret unless the customer tries to use the e-cash illicitly – more than once)
* Bank’s signature

The services for each party are described as follows:

## Customer ##
* Generates N orders for each money order the customer wants to make and assigns a different random uniqueness string number for each of the N e-cash money orders
* Implements the secret splitting (Chapter 3.6 ) and bit commitment(Chapter 4.9 ) protocols used to generate the identity string that describe the customer’s name, address and any other piece of identifying information that the bank wants to see
* Implements a blind signature (Chapter 5.3 ) protocol for all N money orders
* Automatically complies to reveal the half of the identity string chosen by the merchant

## Merchant ##
* Verification of the legitimacy of the bank’s signature
* Random generator of the selector string, which determines the half of the identity string the customer is required to reveal

## Bank ##
* Random choice of 1 out of N money orders sent by the customer to remain unopened
* An algorithm that certifies that all the N-1 money orders have been filled with valid information
* A procedure to certify that the orders received from the merchants have not been used previously
and storage of the uniqueness string and identity strings of the orders in a database file
* Appropriate measures against reuse of the e-cash

Special consideration is the ID of the customer can use an integer number for convenience of bitwise operations.

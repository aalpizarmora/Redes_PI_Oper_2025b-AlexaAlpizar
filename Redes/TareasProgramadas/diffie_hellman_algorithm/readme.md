# Programming Assignment 2 - Diffie-Hellman Key Exchange

* **Secure Key Exchange Protocol**

## Student:
- Alexa Alpízar Mora, C20281

## Program Explanation

This program implements the Diffie-Hellman Key Exchange protocol, which allows two parties to establish a shared secret key over an insecure communication channel. The protocol enables secure key exchange without pre-shared secrets.

## Algorithm Overview

### **Mathematical Foundation:**
The security of Diffie-Hellman relies on the **Discrete Logarithm Problem** - it's computationally difficult to calculate the shared secret even when knowing the public parameters and public keys.

### **Key Steps:**
1. **Public Parameters**: Both parties agree on a prime number `P` and generator `G`
2. **Private Keys**: Each party generates their own private key (`a` for Alice, `b` for Bob)
3. **Public Keys**: Each computes their public key using modular exponentiation
4. **Key Exchange**: Parties exchange public keys
5. **Shared Secret**: Both compute the same shared secret using the other party's public key

### **Mathematical Operations:**
- Alice: `A = G^a mod P` → sends to Bob
- Bob: `B = G^b mod P` → sends to Alice  
- Shared Secret: `secret = B^a mod P = A^b mod P = G^(a*b) mod P`

## #Dependencies

* **Python 3.6** or higher
* **No external libraries required** - Uses only Python standard library

## User Manual

### Execution:

src/

```bash
    python3 diffie_hellman.py
```

### Output example

```bash

----------------------------------------
DIFFIE-HELLMAN KEY EXCHANGE
----------------------------------------

Enter Public Parameters:
Prime number P: 23
Generator G: 9

Enter Private Keys:
Alice's private key a: 4
Bob's private key b: 3

----------------------------------------
CALCULATIONS:
----------------------------------------

Alice:
Private key: a = 4
Public key: A = 9^4 mod 23 = 6

Bob:
Private key: b = 3
Public key: B = 9^3 mod 23 = 16

Key Exchange:
Alice sends A = 6 to Bob
Bob sends B = 16 to Alice

Shared Secret:
Alice computes: 16^4 mod 23 = 9
Bob computes: 6^3 mod 23 = 9

 Result: Secrets match! Shared key = 9
```
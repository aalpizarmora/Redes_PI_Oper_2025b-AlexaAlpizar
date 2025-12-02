# Diffie-Hellman Key Exchange

def power(a, b, p):
    # Compute a^b mod p using modular exponentiation
    return pow(a, b, p)

def main():
    print("\n"+"-" * 40)
    print("DIFFIE-HELLMAN KEY EXCHANGE")
    print("-" * 40)
    
    # Get public parameters from user
    print("\nEnter Public Parameters:")
    P = int(input("Prime number P: "))
    G = int(input("Generator G: "))
    
    # Get private keys from users
    print("\nEnter Private Keys:")
    a = int(input("Alice's private key a: "))
    b = int(input("Bob's private key b: "))
    
    print("\n" + "-" * 40)
    print("CALCULATIONS:")
    print("-" * 40)
    
    # Alice's calculations
    A = power(G, a, P)
    print(f"\nAlice:")
    print(f"Private key: a = {a}")
    print(f"Public key: A = {G}^{a} mod {P} = {A}")
    
    # Bob's calculations
    B = power(G, b, P)
    print(f"\nBob:")
    print(f"Private key: b = {b}")
    print(f"Public key: B = {G}^{b} mod {P} = {B}")
    
    # Key exchange
    print(f"\nKey Exchange:")
    print(f"Alice sends A = {A} to Bob")
    print(f"Bob sends B = {B} to Alice")
    
    # Shared secret calculation
    secret_alice = power(B, a, P)
    secret_bob = power(A, b, P)
    
    print(f"\nShared Secret:")
    print(f"Alice computes: {B}^{a} mod {P} = {secret_alice}")
    print(f"Bob computes: {A}^{b} mod {P} = {secret_bob}")
    
    print(f"\n Result: Secrets match! Shared key = {secret_alice}")

if __name__ == "__main__":
    main()
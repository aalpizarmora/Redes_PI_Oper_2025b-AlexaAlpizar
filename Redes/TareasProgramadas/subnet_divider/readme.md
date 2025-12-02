# Programming Assignment 1 - Subnet Divider

* **IPv4 Subnetting**

## Student:
- Alexa Alpízar Mora, C20281

## Program Explanation

This program distributes a given IPv4 address set according to a list of range requests and follows the order specified by the user. It receives as parameters the IPv4 address to distribute, the list of requests, and the specified order.

## Project Structure

- src/
    
    main.py # Main entry point

    simulation.py # Simulation and assignment logic

    subnet_tree.py # Binary tree for subnets

- data/

    requests_example1.txt # Small example

    requests_large.txt # Large example (500, 300, 150, 100, 80, 50, 30, 20, 10 hosts)

- README.md

## Dependencies

* **Python 3.6** or higher
* **No external libraries required** - Only Python standard modules

## User Manual

### Program Execution:

src/

```bash
python3 src/main.py <base_ip> <requests_file> <order>
```

example:
```bash
python3 main.py 172.16.64.0 ../data/requests_example1.txt LIF
```

- <base_ip>: Base IPv4 address (example: 192.168.0.0, 10.0.0.0, 172.16.0.0)

- <requests_file>: Path to requests file (example: data/requests_example1.txt)

- <<order>order>: Assignment mode - LIF or HIF
    - LIF: Assigns from lowest IPs to highest IPs
    - HIF: Assigns from highest IPs to lowest IPs

### Output Example

```bash
Base IP: 172.16.64.0
Requests file: ../data/requests_example1.txt
Order: LIF
Label: A, Hosts: 16
Label: B, Hosts: 127
Label: C, Hosts: 30
Label: D, Hosts: 1024
Label: E, Hosts: 511
Label: F, Hosts: 128
- Range A: original 16, adjusted 32 (/27)
- Range B: original 127, adjusted 256 (/24)
- Range C: original 30, adjusted 32 (/27)
- Range D: original 1024, adjusted 2048 (/21)
- Range E: original 511, adjusted 1024 (/22)
- Range F: original 128, adjusted 256 (/24)
Container network: 172.16.64.0/20 (4096 IPs)

Set          Hosts  Net             Mask  Broadcast       First           Last
D            2048   172.16.64.0     /21   172.16.71.255   172.16.64.1     172.16.71.254
E            1024   172.16.72.0     /22   172.16.75.255   172.16.72.1     172.16.75.254
B            256    172.16.76.0     /24   172.16.76.255   172.16.76.1     172.16.76.254
F            256    172.16.77.0     /24   172.16.77.255   172.16.77.1     172.16.77.254
A            32     172.16.78.0     /27   172.16.78.31    172.16.78.1     172.16.78.30
C            32     172.16.78.32    /27   172.16.78.63    172.16.78.33    172.16.78.62
```
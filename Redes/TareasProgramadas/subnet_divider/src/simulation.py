import re
import ipaddress
from typing import List
from subnet_tree import SubnetTree, Subnet, Request

class Simulation:
    def __init__(self):
        self.ip_base_str = ""
        self.file = ""
        self.ip_order = ""
        self.requests: List[Request] = []
        self.format = re.compile(r'(\d+)\s+addresses\s+for\s+range\s+([A-Za-z0-9_]+)')
    
    def start(self, args: List[str]) -> int:
        # Validate arguments
        if len(args) != 4:
            print(f"Usage: {args[0]} <base_ip> <requests_file> <order>")
            print("Order: LIF (Low IP First) | HIF (High IP First)")
            return -1
        
        self.ip_base_str = args[1]
        self.file = args[2]
        self.ip_order = args[3]
        
        # Validate ip_order
        if self.ip_order not in ["LIF", "HIF"]:
            print("Invalid ipOrder: use 'LIF' or 'HIF'")
            return -1
        
        self.print_arguments()
        
        if self.open_file() == -1:
            return -1
        
        self.print_requests()
        self.adjust_requests()
        self.print_adjusted_requests()
        
        # Build subnet tree and assign subnets
        tree = SubnetTree()
        self.assign_subnets_and_insert(tree)
        
        # Print final subnet assignments
        print("\nSet          Hosts  Net             Mask  Broadcast       First           Last")
        tree.print_tree()
        
        return 0
    
    # Opens and reads the requests file
    def open_file(self) -> int:
        try:
            with open(self.file, 'r') as file:
                for line in file:
                    match = self.format.match(line.strip())
                    if match:
                        request = Request(
                            hosts=int(match.group(1)),
                            label=match.group(2)
                        )
                        self.requests.append(request)
            return 0
        except FileNotFoundError:
            print(f"Error opening file: {self.file}")
            return -1
    
    # Adjust requests to nearest power of two
    def adjust_requests(self):
        for request in self.requests:
            total_needed = request.hosts + 2  # hosts + network + broadcast
            
            adjusted = 1
            power = 0
            while adjusted < total_needed:
                adjusted <<= 1
                power += 1
            
            request.adjusted_hosts = adjusted
            request.mask = 32 - power
    
    def assign_subnets_and_insert(self, tree: SubnetTree):
        base_ip = int(ipaddress.IPv4Address(self.ip_base_str))
        
        # Sort requests by adjusted size descending
        sorted_requests = sorted(self.requests, key=lambda x: x.adjusted_hosts, reverse=True)
        
        # Calculate total needed IPs
        total_needed = sum(1 << (32 - req.mask) for req in sorted_requests)
        
        # Find container mask
        container_mask = self.find_container_mask(total_needed)
        container_block_size = 1 << (32 - container_mask)
        
        # Align container network address
        container_net_addr = self.align_down(base_ip, container_block_size)
        container_broadcast = container_net_addr + container_block_size - 1
        
        print(f"Container network: {self.uint_to_ip(container_net_addr)}"
            f"/{container_mask} ({container_block_size} IPs)")
        
        subnets_list = []  # List to hold subnets before inserting into tree
        
        if self.ip_order == "HIF":
            # High IP First: the bigger blocks get higher IPs
            # Start from the top of the container
            current_ip = container_broadcast + 1  # Just above broadcast
            
            for request in sorted_requests:
                block_size = 1 << (32 - request.mask)
                current_ip -= block_size  # Retroceded by block size
                net_address = self.align_down(current_ip, block_size)
                
                subnet = Subnet(
                    label=request.label,
                    net_address=net_address,
                    broadcast=net_address + block_size - 1,
                    first_valid=net_address + 1,
                    last_valid=net_address + block_size - 2,
                    mask=request.mask,
                    hosts_count=request.adjusted_hosts
                )
                subnets_list.append(subnet)
                
        else:  # LIF
            # Low IP First: the bigger blocks get lower IPs
            current_ip = container_net_addr
            
            for request in sorted_requests:
                block_size = 1 << (32 - request.mask)
                # Align current_ip to block size
                if current_ip % block_size != 0:
                    current_ip = self.align_up(current_ip, block_size)
                
                subnet = Subnet(
                    label=request.label,
                    net_address=current_ip,
                    broadcast=current_ip + block_size - 1,
                    first_valid=current_ip + 1,
                    last_valid=current_ip + block_size - 2,
                    mask=request.mask,
                    hosts_count=request.adjusted_hosts
                )
                subnets_list.append(subnet)
                current_ip += block_size
        
        # Insert subnets into tree based on order
        if self.ip_order == "HIF":
            # For HIF, we want to show from higher to lower IP (bigger with higher first)
            subnets_list.sort(key=lambda x: x.net_address, reverse=True)
        else:  # LIF
            # For LIF, we want to show from lower to higher IP
            subnets_list.sort(key=lambda x: x.net_address)
        
        # Insert into tree
        # Insted of inserting during creation, we insert after sorting
        for subnet in subnets_list:
            tree.insert(subnet)
    
    # Alignment helper methods
    @staticmethod
    def align_down(ip: int, block_size: int) -> int:
        return ip & ~(block_size - 1)
    
    @staticmethod
    def align_up(ip: int, block_size: int) -> int:
        return (ip + block_size - 1) & ~(block_size - 1)
    
    # Find the smallest mask that can contain total_needed addresses
    @staticmethod
    def find_container_mask(total_needed: int) -> int:
        bits = 0
        while (1 << bits) < total_needed:
            bits += 1
        return 32 - bits
    
    # Convert uint to IP string
    @staticmethod
    def uint_to_ip(ip: int) -> str:
        return str(ipaddress.IPv4Address(ip))
    
    def print_arguments(self):
        print(f"Base IP: {self.ip_base_str}")
        print(f"Requests file: {self.file}")
        print(f"Order: {self.ip_order}")
    
    def print_requests(self):
        for request in self.requests:
            print(f"Label: {request.label}, Hosts: {request.hosts}")
    
    def print_adjusted_requests(self):
        for request in self.requests:
            print(f"- Range {request.label}: original {request.hosts}, "
                  f"adjusted {request.adjusted_hosts} (/{request.mask})")

# Main usage
if __name__ == "__main__":
    import sys
    simulation = Simulation()
    exit_code = simulation.start(sys.argv)
    sys.exit(exit_code)
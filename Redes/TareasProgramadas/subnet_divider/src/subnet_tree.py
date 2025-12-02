# Manages the binary tree structure for storing and printing subnets

from dataclasses import dataclass
from typing import Optional
import ipaddress

# Data classes for Subnet and Request
@dataclass
class Request:
    label: str
    hosts: int
    adjusted_hosts: int = 0
    mask: int = 0
    
@dataclass
class Subnet:
    label: str
    net_address: int
    first_valid: int
    last_valid: int
    broadcast: int
    mask: int
    hosts_count: int
    
    def __lt__(self, other):
        return self.net_address < other.net_address

# Binary Tree Node and Tree for Subnets
class SubnetNode:
    def __init__(self, data: Subnet):
        self.data = data
        self.left = None
        self.right = None

class SubnetTree:
    def __init__(self):
        self.root = None
    
    def insert(self, data: Subnet):
        self.root = self._insert_node(self.root, data)
    
    def _insert_node(self, node: Optional[SubnetNode], data: Subnet) -> SubnetNode:
        if node is None:
            return SubnetNode(data)
        
        if data < node.data:
            node.left = self._insert_node(node.left, data)
        else:
            node.right = self._insert_node(node.right, data)
        
        return node
    
    def print_tree(self):
        self._inorder_print(self.root)
    
    def _inorder_print(self, node: Optional[SubnetNode]):
        if node is None:
            return
        
        self._inorder_print(node.left)
        self._print_subnet(node.data)
        self._inorder_print(node.right)
    
    def _print_subnet(self, subnet: Subnet):
        net_ip = self.uint_to_ip(subnet.net_address)
        bc_ip = self.uint_to_ip(subnet.broadcast)
        first_ip = self.uint_to_ip(subnet.first_valid)
        last_ip = self.uint_to_ip(subnet.last_valid)
        
        print(f"{subnet.label:<12} {subnet.hosts_count:<6} {net_ip:<15} /{subnet.mask:<4} "
            f"{bc_ip:<15} {first_ip:<15} {last_ip}")
    
    @staticmethod
    def uint_to_ip(ip: int) -> str:
        return str(ipaddress.IPv4Address(ip))
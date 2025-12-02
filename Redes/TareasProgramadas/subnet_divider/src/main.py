# Principal module for subnet divider simulation

import sys
import os

sys.path.insert(0, os.path.dirname(__file__))

from simulation import Simulation

def main():
    # Simulation main execution
    simulation = Simulation()
    exit_code = simulation.start(sys.argv)
    sys.exit(exit_code)

if __name__ == "__main__":
    main()
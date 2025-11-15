#include "Tenedor.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    int puerto = 8080;
    int isla = 0;
    
    if (argc > 1) puerto = std::atoi(argv[1]);
    if (argc > 2) isla = std::atoi(argv[2]);
    
    if (isla < 0 || isla > 6) {
        std::cout << "Error: La isla debe ser un numero entre 0 y 6" << std::endl;
        return 1;
    }
    
    std::cout << "=== SERVIDOR TENEDOR ===" << std::endl;
    std::cout << "Puerto: " << puerto << std::endl;
    std::cout << "Isla: " << isla << std::endl;
    std::cout << "========================" << std::endl;
    
    Tenedor tenedor(puerto, isla);
    tenedor.iniciar();
    
    return 0;
}
#include "Tenedor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    int puerto = 8080;
    
    if (argc > 1) {
        puerto = std::atoi(argv[1]);
    }
    
    std::cout << "=== TENEDOR SIMPLE ===" << std::endl;
    std::cout << "Iniciando en puerto: " << puerto << std::endl;
    std::cout << "======================" << std::endl;
    
    Tenedor tenedor(puerto);
    tenedor.iniciar();
    
    return 0;
}
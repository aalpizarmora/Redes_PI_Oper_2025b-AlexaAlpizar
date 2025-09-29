#include "Cliente.h"
#include <iostream>
#include <string>

int main(int argc, const char* argv[]) {
    Cliente* cliente = nullptr;

    if (argc == 3) {
        std::string ip = argv[1];
        int puerto = std::stoi(argv[2]); // Convertir a entero
        cliente = new Cliente(ip.c_str(), puerto);
        std::cout << "Cliente creado con IP: " << ip << " y puerto: " << puerto << std::endl;
    } 
    else if (argc == 1) {
        cliente = new Cliente(); // Constructor por defecto
        std::cout << "Cliente creado con valores por defecto." << std::endl;
    } 
    else {
        std::cerr << "Uso: " << argv[0] << " [IP] [Puerto]" << std::endl;
        return 1;
    }
    cliente->start();

    delete cliente; // Liberar memoria
    return 0;
}

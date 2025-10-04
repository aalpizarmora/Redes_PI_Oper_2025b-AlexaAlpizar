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

    std::cout << "\n=== CLIENTE PROTOCOLO ===" << std::endl;
    std::cout << "Comandos disponibles:" << std::endl;
    std::cout << "1. list - Listar archivos" << std::endl;
    std::cout << "2. request <nombre> - Solicitar archivo" << std::endl;
    std::cout << "3. submit <nombre> - Subir archivo" << std::endl;
    std::cout << "4. delete <nombre> - Eliminar archivo" << std::endl;
    std::cout << "0. salir - Terminar" << std::endl;

    std::string comando, nombre;
    
    while (true) {
        std::cout << "\nIngrese comando: ";
        std::cin >> comando;
        
        if (comando == "0" || comando == "salir") break;
        else if (comando == "list") cliente->listFiles();
        else if (comando == "request" || comando == "submit" || comando == "delete") {
            std::cout << "Ingrese nombre del archivo: ";
            std::cin >> nombre;
            
            if (comando == "request") cliente->requestFile(nombre.c_str());
            else if (comando == "delete") cliente->deleteFile(nombre.c_str());
            else if (comando == "submit") {
                std::cout << "Ingrese contenido: ";
                std::string contenido;
                std::cin.ignore();
                std::getline(std::cin, contenido);
                cliente->submitFile(nombre.c_str(), contenido.c_str());
            }
        }
        else {
            std::cout << "Comando no reconocido" << std::endl;
        }
    }

    delete cliente;
    return 0;
}



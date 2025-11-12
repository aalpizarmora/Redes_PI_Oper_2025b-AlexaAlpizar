#include "Socket.h"
#include <iostream>
#include <string>

void probarTenedor(const std::string& host, int puerto, const std::string& path) {
    try {
        Socket cliente('s', false);
        cliente.MakeConnection(host.c_str(), puerto);
        
        std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
        cliente.Write(request.c_str(), request.size());
        
        char buffer[4096];
        std::string response;
        int bytes = cliente.Read(buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            response = buffer;
            std::cout << "=== RESPUESTA (" << path << ") ===" << std::endl;
            std::cout << response << std::endl;
            std::cout << "=============================" << std::endl;
        }
        
        cliente.Close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Probando tenedor en puerto 9090..." << std::endl;
    
    probarTenedor("localhost", 9090, "/");
    probarTenedor("localhost", 9090, "/tabla");
    probarTenedor("localhost", 9090, "/figura?nombre=circulo");
    
    return 0;
}
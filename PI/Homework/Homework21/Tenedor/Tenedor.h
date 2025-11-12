#ifndef TENEDOR_H
#define TENEDOR_H

#include "Socket.h"
#include <map>
#include <string>
#include <vector>

/**
 * @class Tenedor
 * @brief Servidor tenedor básico
 */
class Tenedor {
public:
    Tenedor(int puerto = 8080);
    ~Tenedor();
    
    void iniciar();
    
private:
    void procesarConexion(VSocket* cliente);
    std::string procesarHTTP(const std::string& request);
    std::string generarTablaHTML();
    
    int puerto;
    std::map<std::string, std::string> figuras; // figura -> "ip:puerto"
};

#endif // TENEDOR_H
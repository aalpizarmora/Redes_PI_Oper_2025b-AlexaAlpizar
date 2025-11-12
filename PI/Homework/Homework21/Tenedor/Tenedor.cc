#include "Tenedor.h"
#include <iostream>
#include <sstream>

Tenedor::Tenedor(int puerto) : puerto(puerto) {
    // Datos de ejemplo para pruebas
    figuras["circulo"] = "172.16.123.5:8081";
    figuras["cuadrado"] = "172.16.123.6:8081"; 
    figuras["triangulo"] = "172.16.123.7:8081";
    
    std::cout << "Tenedor creado en puerto " << puerto << std::endl;
    std::cout << "Figuras de ejemplo cargadas: " << figuras.size() << std::endl;
}

Tenedor::~Tenedor() {
    std::cout << "Tenedor destruido" << std::endl;
}

void Tenedor::iniciar() {
    try {
        VSocket* servidor = new Socket('s', false);
        
        servidor->Bind(puerto);
        servidor->MarkPassive(5);
        
        std::cout << " Tenedor escuchando en puerto " << puerto << std::endl;
        std::cout << " URLs disponibles:" << std::endl;
        std::cout << " http://localhost:" << puerto << "/" << std::endl;
        std::cout << " http://localhost:" << puerto << "/tabla" << std::endl;
        std::cout << " http://localhost:" << puerto << "/figura?nombre=circulo" << std::endl;
        
        while (true) {
            VSocket* cliente = servidor->AcceptConnection();
            std::cout << "Cliente conectado" << std::endl;
            
            // Procesar en el mismo hilo (sencillo para pruebas)
            procesarConexion(cliente);
        }
        
    } catch (const std::exception& e) {
        std::cerr << " Error: " << e.what() << std::endl;
    }
}

void Tenedor::procesarConexion(VSocket* cliente) {
    try {
        char buffer[1024];
        std::string request;
        
        // Leer request HTTP
        int bytes = cliente->Read(buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            request = buffer;
            
            std::cout << "Request: " << request.substr(0, 100) << "..." << std::endl;
            
            // Procesar y generar respuesta
            std::string respuesta = procesarHTTP(request);
            
            // Enviar respuesta
            cliente->Write(respuesta.c_str(), respuesta.size());
            std::cout << "Respuesta enviada (" << respuesta.size() << " bytes)" << std::endl;
        }
        
        cliente->Close();
        delete cliente;
        
    } catch (const std::exception& e) {
        std::cerr << " Error con cliente: " << e.what() << std::endl;
        if (cliente) {
            cliente->Close();
            delete cliente;
        }
    }
}

std::string Tenedor::procesarHTTP(const std::string& request) {
    std::string contenido;
    
    if (request.find("GET /tabla") != std::string::npos) {
        contenido = generarTablaHTML();
    }
    else if (request.find("GET /figura?nombre=") != std::string::npos) {
        // Extraer nombre de figura
        size_t inicio = request.find("nombre=") + 7;
        size_t fin = request.find(" ", inicio);
        std::string figura = request.substr(inicio, fin - inicio);
        
        auto it = figuras.find(figura);
        if (it != figuras.end()) {
            contenido = "<html><body><h1>Figura: " + figura + "</h1>"
                       "<p>Ubicación: " + it->second + "</p>"
                       "<p><em>Nota: En etapa 4, aquí se conectaría al servidor real</em></p>"
                       "</body></html>";
        } else {
            contenido = "<html><body><h1>Figura no encontrada: " + figura + "</h1></body></html>";
        }
    }
    else {
        // Página principal
        contenido = "<html><body>"
                   "<h1>🚀 Servidor Tenedor - Etapa 4</h1>"
                   "<p><strong>Estado:</strong> Funcionando correctamente</p>"
                   "<ul>"
                   "<li><a href='/tabla'>Ver tabla de rutas</a></li>"
                   "<li>Ejemplo: <a href='/figura?nombre=circulo'>Buscar círculo</a></li>"
                   "<li>Ejemplo: <a href='/figura?nombre=cuadrado'>Buscar cuadrado</a></li>"
                   "</ul>"
                   "<p><em>Este es un servidor básico para pruebas iniciales</em></p>"
                   "</body></html>";
    }
    
    // Construir respuesta HTTP
    std::string respuesta = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(contenido.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + contenido;
    
    return respuesta;
}

std::string Tenedor::generarTablaHTML() {
    std::stringstream html;
    
    html << "<html><head><title>Tabla de Rutas - Tenedor</title></head><body>"
         << "<h1>🗺️ Tabla de Rutas del Tenedor</h1>"
         << "<table border='1' style='border-collapse: collapse;'>"
         << "<tr><th>Figura</th><th>Servidor</th><th>Estado</th></tr>";
    
    for (const auto& [figura, servidor] : figuras) {
        html << "<tr>"
             << "<td>" << figura << "</td>"
             << "<td>" << servidor << "</td>"
             << "<td> Disponible</td>"
             << "</tr>";
    }
    
    html << "</table>"
         << "<p><strong>Total:</strong> " << figuras.size() << " figuras registradas</p>"
         << "<p><a href='/'>Volver al inicio</a></p>"
         << "</body></html>";
    
    return html.str();
}
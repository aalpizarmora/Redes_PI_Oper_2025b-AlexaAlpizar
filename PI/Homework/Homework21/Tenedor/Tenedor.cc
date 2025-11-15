#include "Tenedor.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// Inicialización de constantes estáticas
const int Tenedor::PUERTO_DISCOVERY = 8888;
const int Tenedor::INTERVALO_BROADCAST = 30;
const int Tenedor::TIMEOUT_FIGURA = 120;

Tenedor::Tenedor(int puerto, int isla) : 
    puerto(puerto), 
    isla(isla),
    running(false),
    servidorSocket(-1) {
    
    std::cout << "Tenedor creado en puerto " << puerto << ", isla " << isla << std::endl;
}

Tenedor::~Tenedor() {
    detener();
    std::cout << "Tenedor destruido" << std::endl;
}

void Tenedor::iniciar() {
    running = true;
    
    // Iniciar servidor HTTP en hilo separado
    httpThread = std::thread(&Tenedor::iniciarServidorHTTP, this);
    
    // Iniciar sistema de discovery en hilo separado
    discoveryThread = std::thread(&Tenedor::iniciarDiscovery, this);
    
    // Iniciar limpieza periodica de figuras inactivas
    cleanupThread = std::thread(&Tenedor::limpiarFigurasInactivas, this);
    
    // Esperar a que todos los hilos terminen
    httpThread.join();
    discoveryThread.join();
    cleanupThread.join();
}

void Tenedor::detener() {
    running = false;
    
    // Cerrar socket del servidor HTTP para liberar el accept()
    if (servidorSocket != -1) {
        close(servidorSocket);
        servidorSocket = -1;
    }
}

// ========== SERVIDOR HTTP PRINCIPAL ==========

void Tenedor::iniciarServidorHTTP() {
    try {
        // Crear socket del servidor
        servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (servidorSocket < 0) {
            throw std::runtime_error("No se pudo crear socket del servidor");
        }
        
        // Permitir reutilizar la dirección
        int reuse = 1;
        setsockopt(servidorSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        
        // Configurar dirección del servidor
        struct sockaddr_in direccionServidor;
        memset(&direccionServidor, 0, sizeof(direccionServidor));
        direccionServidor.sin_family = AF_INET;
        direccionServidor.sin_addr.s_addr = INADDR_ANY;
        direccionServidor.sin_port = htons(puerto);
        
        // Hacer bind
        if (bind(servidorSocket, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor)) < 0) {
            close(servidorSocket);
            throw std::runtime_error("No se pudo hacer bind al puerto " + std::to_string(puerto));
        }
        
        // Escuchar conexiones
        if (listen(servidorSocket, 5) < 0) {
            close(servidorSocket);
            throw std::runtime_error("No se pudo poner el socket en modo escucha");
        }
        
        std::cout << "=========================================" << std::endl;
        std::cout << "  Tenedor iniciado correctamente" << std::endl;
        std::cout << "  HTTP escuchando en puerto: " << puerto << std::endl;
        std::cout << "  Discovery en puerto: " << PUERTO_DISCOVERY << std::endl;
        std::cout << "  Isla: " << isla << std::endl;
        std::cout << "  Broadcast: " << obtenerDireccionBroadcast(isla) << std::endl;
        std::cout << "=========================================" << std::endl;
        
        while (running) {
            // Aceptar conexiones con timeout
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(servidorSocket, &readfds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int ready = select(servidorSocket + 1, &readfds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(servidorSocket, &readfds)) {
                struct sockaddr_in direccionCliente;
                socklen_t longitudCliente = sizeof(direccionCliente);
                
                int clienteSocket = accept(servidorSocket, (struct sockaddr*)&direccionCliente, &longitudCliente);
                if (clienteSocket >= 0) {
                    std::cout << "Cliente HTTP conectado desde " 
                              << inet_ntoa(direccionCliente.sin_addr) << std::endl;
                    
                    // Manejar cliente en el mismo hilo (para simplificar)
                    manejarClienteHTTP(clienteSocket);
                }
            }
        }
        
        close(servidorSocket);
        servidorSocket = -1;
        
    } catch (const std::exception& e) {
        std::cerr << "Error en servidor HTTP: " << e.what() << std::endl;
    }
}

void Tenedor::manejarClienteHTTP(int clienteSocket) {
    try {
        char buffer[4096];
        std::string request;
        
        // Leer request del cliente
        ssize_t bytesLeidos = read(clienteSocket, buffer, sizeof(buffer) - 1);
        if (bytesLeidos > 0) {
            buffer[bytesLeidos] = '\0';
            request = buffer;
            
            std::cout << "Request HTTP: " << request.substr(0, request.find('\n')) << std::endl;
            
            // Procesar request y generar respuesta
            std::string respuesta = procesarHTTP(request);
            
            // Enviar respuesta
            write(clienteSocket, respuesta.c_str(), respuesta.size());
            std::cout << "Respuesta enviada (" << respuesta.size() << " bytes)" << std::endl;
        }
        
        close(clienteSocket);
        
    } catch (const std::exception& e) {
        std::cerr << "Error manejando cliente HTTP: " << e.what() << std::endl;
        close(clienteSocket);
    }
}

std::string Tenedor::procesarHTTP(const std::string& request) {
    std::string contenido;
    
    if (request.find("GET /tabla") != std::string::npos) {
        contenido = generarTablaHTML();
    }
    else if (request.find("GET /figura?nombre=") != std::string::npos) {
        size_t inicio = request.find("nombre=") + 7;
        size_t fin = request.find(" ", inicio);
        std::string figura = request.substr(inicio, fin - inicio);
        
        std::lock_guard<std::mutex> lock(figurasMutex);
        auto it = figuras.find(figura);
        if (it != figuras.end()) {
            contenido = "<html><body><h1>Figura: " + figura + "</h1>"
                       "<p><strong>Servidor:</strong> " + it->second + "</p>"
                       "<p><em>Conectado via discovery</em></p>"
                       "</body></html>";
        } else {
            contenido = "<html><body><h1>Figura no encontrada: " + figura + "</h1></body></html>";
        }
    }
    else {
        std::lock_guard<std::mutex> lock(figurasMutex);
        contenido = "<html><body>"
                   "<h1>Servidor Tenedor - Sistema de Discovery</h1>"
                   "<p><strong>Figuras registradas:</strong> " + std::to_string(figuras.size()) + "</p>"
                   "<ul>"
                   "<li><a href='/tabla'>Ver tabla de rutas</a></li>"
                   "<li>Ejemplo: <a href='/figura?nombre=circulo'>Buscar circulo</a></li>"
                   "<li>Ejemplo: <a href='/figura?nombre=cuadrado'>Buscar cuadrado</a></li>"
                   "<li>Ejemplo: <a href='/figura?nombre=triangulo'>Buscar triangulo</a></li>"
                   "</ul>"
                   "</body></html>";
    }
    
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
    
    std::lock_guard<std::mutex> lock(figurasMutex);
    
    html << "<html><head><title>Tabla de Rutas - Tenedor</title>"
         << "<style>table { border-collapse: collapse; width: 80%; margin: 20px auto; }"
         << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
         << "th { background-color: #f2f2f2; }</style></head><body>"
         << "<h1>Tabla de Rutas del Tenedor</h1>"
         << "<table>"
         << "<tr><th>Figura</th><th>Servidor</th><th>Estado</th></tr>";
    
    auto now = std::chrono::steady_clock::now();
    
    for (const auto& pair : figuras) {
        const std::string& figura = pair.first;
        const std::string& servidor = pair.second;
        
        auto it = ultimaActividad.find(figura);
        bool activa = false;
        if (it != ultimaActividad.end()) {
            auto duracion = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
            activa = (duracion.count() < TIMEOUT_FIGURA);
        }
        
        html << "<tr>"
             << "<td>" << figura << "</td>"
             << "<td>" << servidor << "</td>"
             << "<td>" << (activa ? "Activa" : "Inactiva") << "</td>"
             << "</tr>";
    }
    
    html << "</table>"
         << "<p><strong>Total:</strong> " << figuras.size() << " figuras</p>"
         << "<p><a href='/'>Volver al inicio</a></p>"
         << "</body></html>";
    
    return html.str();
}

// ========== SISTEMA DE DISCOVERY ==========

void Tenedor::iniciarDiscovery() {
    std::cout << "Iniciando sistema de discovery..." << std::endl;
    broadcastPresencia();
    escucharBroadcasts();
}

void Tenedor::escucharBroadcasts() {
    try {
        int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpSocket < 0) throw std::runtime_error("No se pudo crear socket UDP");
        
        int reuse = 1;
        setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        
        struct sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(PUERTO_DISCOVERY);
        
        if (bind(udpSocket, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
            close(udpSocket);
            throw std::runtime_error("No se pudo hacer bind al puerto de discovery");
        }
        
        std::cout << "Escuchando broadcasts en puerto " << PUERTO_DISCOVERY << std::endl;
        
        while (running) {
            char buffer[1024];
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(udpSocket, &readfds);
            
            struct timeval timeout = {1, 0};
            int ready = select(udpSocket + 1, &readfds, NULL, NULL, &timeout);
            
            if (ready > 0 && FD_ISSET(udpSocket, &readfds)) {
                ssize_t bytes = recvfrom(udpSocket, buffer, sizeof(buffer)-1, 0,
                                       (struct sockaddr*)&clientAddr, &clientAddrLen);
                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    std::string mensaje(buffer);
                    std::string direccionRemota = obtenerIPDesdeSockaddr(clientAddr);
                    
                    std::cout << "Mensaje recibido de " << direccionRemota << ": " << mensaje << std::endl;
                    
                    if (mensaje.find("REGISTRO_FIGURA:") == 0) {
                        procesarRegistro(mensaje, direccionRemota);
                    }
                }
            }
            
            static auto ultimoBroadcast = std::chrono::steady_clock::now();
            auto ahora = std::chrono::steady_clock::now();
            auto duracion = std::chrono::duration_cast<std::chrono::seconds>(ahora - ultimoBroadcast);
            
            if (duracion.count() >= INTERVALO_BROADCAST) {
                broadcastPresencia();
                ultimoBroadcast = ahora;
            }
        }
        
        close(udpSocket);
        
    } catch (const std::exception& e) {
        std::cerr << "Error en sistema de discovery: " << e.what() << std::endl;
    }
}

void Tenedor::broadcastPresencia() {
    try {
        int broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (broadcastSocket < 0) throw std::runtime_error("No se pudo crear socket de broadcast");
        
        int broadcastEnable = 1;
        setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
        
        struct sockaddr_in broadcastAddr;
        memset(&broadcastAddr, 0, sizeof(broadcastAddr));
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_port = htons(PUERTO_DISCOVERY);
        inet_pton(AF_INET, obtenerDireccionBroadcast(isla).c_str(), &broadcastAddr.sin_addr);
        
        std::string mensaje = "TENEDOR_ACTIVO:" + std::to_string(puerto);
        sendto(broadcastSocket, mensaje.c_str(), mensaje.size(), 0,
              (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        
        std::cout << "Broadcast enviado: " << mensaje << std::endl;
        close(broadcastSocket);
        
    } catch (const std::exception& e) {
        std::cerr << "Error en broadcast: " << e.what() << std::endl;
    }
}

void Tenedor::procesarRegistro(const std::string& mensaje, const std::string& direccionRemota) {
    size_t inicioNombre = mensaje.find(':') + 1;
    size_t finNombre = mensaje.find(':', inicioNombre);
    size_t inicioPuerto = finNombre + 1;
    
    if (inicioNombre != std::string::npos && finNombre != std::string::npos && inicioPuerto != std::string::npos) {
        std::string nombreFigura = mensaje.substr(inicioNombre, finNombre - inicioNombre);
        std::string puertoFigura = mensaje.substr(inicioPuerto);
        std::string direccionCompleta = direccionRemota + ":" + puertoFigura;
        
        std::lock_guard<std::mutex> lock(figurasMutex);
        figuras[nombreFigura] = direccionCompleta;
        ultimaActividad[nombreFigura] = std::chrono::steady_clock::now();
        
        std::cout << "Figura registrada: " << nombreFigura << " -> " << direccionCompleta << std::endl;
    }
}

void Tenedor::limpiarFigurasInactivas() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(figurasMutex);
        
        for (auto it = ultimaActividad.begin(); it != ultimaActividad.end(); ) {
            auto duracion = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
            if (duracion.count() > TIMEOUT_FIGURA) {
                std::cout << "Removiendo figura inactiva: " << it->first << std::endl;
                figuras.erase(it->first);
                it = ultimaActividad.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// ========== UTILIDADES DE RED ==========

std::string Tenedor::obtenerDireccionBroadcast(int isla) {
    switch (isla) {
        case 0: return "172.16.123.15";
        case 1: return "172.16.123.31";
        case 2: return "172.16.123.47";
        case 3: return "172.16.123.63";
        case 4: return "172.16.123.79";
        case 5: return "172.16.123.95";
        case 6: return "172.16.123.111";
        default: return "172.16.123.15";
    }
}

std::string Tenedor::obtenerIPDesdeSockaddr(const struct sockaddr_in& addr) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ipStr, INET_ADDRSTRLEN);
    return std::string(ipStr);
}

std::string Tenedor::obtenerInfoFigura(const std::string& figura, const std::string& servidor) {
    // Por ahora retorna información básica
    // Aquí se integraría con el servidor de figuras cuando lo tengamos
    return "<html><body><h1>Figura: " + figura + "</h1>"
           "<p><strong>Servidor:</strong> " + servidor + "</p>"
           "<p><em>Informacion detallada de la figura aparecera aqui cuando el servidor de figuras este integrado</em></p>"
           "</body></html>";
}
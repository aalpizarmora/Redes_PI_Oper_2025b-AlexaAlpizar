#ifndef TENEDOR_H
#define TENEDOR_H

#include <map>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

/**
 * @class Tenedor
 * @brief Servidor tenedor con sistema de discovery por broadcast
 */
class Tenedor {
public:
    Tenedor(int puerto = 8080, int isla = 0);
    ~Tenedor();
    
    void iniciar();
    void detener();
    
private:
    // Servidor HTTP principal (usando sockets del sistema)
    void iniciarServidorHTTP();
    void manejarClienteHTTP(int clienteSocket);
    std::string procesarHTTP(const std::string& request);
    std::string generarTablaHTML();
    
    // Sistema de Discovery
    void iniciarDiscovery();
    void escucharBroadcasts();
    void broadcastPresencia();
    void procesarRegistro(const std::string& mensaje, const std::string& direccionRemota);
    void limpiarFigurasInactivas();
    
    // Interacción con servidor de Figuras
    std::string obtenerInfoFigura(const std::string& figura, const std::string& servidor);
    
    // Utilidades de red
    std::string obtenerDireccionBroadcast(int isla);
    std::string obtenerIPDesdeSockaddr(const struct sockaddr_in& addr);
    
    // Miembros de datos
    int puerto;
    int isla;
    std::atomic<bool> running;
    int servidorSocket;  // Socket del servidor HTTP
    
    // Hilos
    std::thread httpThread;
    std::thread discoveryThread;
    std::thread cleanupThread;
    
    // Sincronización
    std::mutex figurasMutex;
    
    // Almacenamiento de figuras
    std::map<std::string, std::string> figuras; // figura -> "ip:puerto"
    std::map<std::string, std::chrono::steady_clock::time_point> ultimaActividad;
    
    // Constantes del protocolo
    static const int PUERTO_DISCOVERY;
    static const int INTERVALO_BROADCAST;
    static const int TIMEOUT_FIGURA;
};

#endif // TENEDOR_H
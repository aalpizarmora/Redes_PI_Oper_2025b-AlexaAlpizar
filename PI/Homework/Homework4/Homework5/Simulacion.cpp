#include "Simulacion.h"

// Buffers compartidos
string buffer_clienteTenedor;
string buffer_tenedorServidor;

// Semáforos
sem_t sem_clienteTenedor;
sem_t sem_respuestaCliente;
sem_t sem_tenedorServidor;
sem_t sem_respuestaServidor;

void cliente() {
    // Enviar solicitud al tenedor
    buffer_clienteTenedor = "GET /figuresList/whale HTTP/1.1";
    cout << "Cliente: envía '" << buffer_clienteTenedor << "'\n";
    sem_post(&sem_clienteTenedor); // Indica al tenedor que hay mensaje

    // Esperar respuesta del tenedor
    sem_wait(&sem_respuestaCliente);
    cout << "Cliente: recibe '" << buffer_clienteTenedor << "'\n";
}

void tenedor() {
    string mensaje_cliente, respuesta_servidor_local;

    // Leer solicitud del cliente
    sem_wait(&sem_clienteTenedor);
    mensaje_cliente = buffer_clienteTenedor;
    cout << "Tenedor: recibe '" << mensaje_cliente << "'\n";

    // Traducir y enviar al servidor
    buffer_tenedorServidor = "TOMAR /figuresList/whale";
    cout << "Tenedor: envía al servidor '" << buffer_tenedorServidor << "'\n";
    sem_post(&sem_tenedorServidor);

    // Esperar respuesta del servidor
    sem_wait(&sem_respuestaServidor);
    respuesta_servidor_local = buffer_tenedorServidor;
    cout << "Tenedor: recibe del servidor '" << respuesta_servidor_local << "'\n";

    // Traducir respuesta para el cliente
    buffer_clienteTenedor = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContenido: whale";
    cout << "Tenedor: envía al cliente '" << buffer_clienteTenedor << "'\n";
    sem_post(&sem_respuestaCliente);
}

void servidor() {
    string mensaje_tenedor;

    // Esperar solicitud del tenedor
    sem_wait(&sem_tenedorServidor);
    mensaje_tenedor = buffer_tenedorServidor;
    cout << "Servidor: recibe '" << mensaje_tenedor << "'\n";

    // Generar respuesta
    buffer_tenedorServidor = "RESPUESTA 250 ARCHIVO-ENTREGADO\nContenido: whale";
    cout << "Servidor: envía '" << buffer_tenedorServidor << "'\n";
    sem_post(&sem_respuestaServidor);
}

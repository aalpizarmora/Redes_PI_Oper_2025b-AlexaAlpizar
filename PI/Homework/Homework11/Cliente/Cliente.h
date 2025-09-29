#ifndef CLIENTE_H
#define CLIENTE_H

#include "Socket.h"
#include "SSLSocket.h"

/**
 * @class Cliente
 * @brief Representa un cliente que se puede conectar a un servidor vía TCP o SSL.
 *
 * La clase Cliente encapsula la creación de un socket, conexión con el servidor,
 * envío de solicitudes y recepción de respuestas.
 */
class Cliente{
    public:
    /**
     * @brief Constructor por defecto.
     *
     * Crea un cliente con IP y puerto por defecto.
     */
    Cliente();

    /**
     * @brief Constructor con parámetros.
     * @param IP Dirección IP del servidor al que se conectará el cliente.
     * @param puerto Puerto del servidor al que se conectará el cliente.
     */
    Cliente(const char* IP, int puerto);

    /**
     * @brief Destructor de la clase Cliente.
     *
     * Libera recursos asociados, incluyendo el socket si fue creado.
     */
    ~Cliente();

    /**
     * @brief Inicia la comunicación con el servidor.
     *
     * Dependiendo de la implementación, decide si usar TCP o SSL y
     * procesa la comunicación.
     */
    void start();

    private:
    VSocket* socketCliente; /**< Puntero al socket utilizado para la conexión. */
    int puerto;             /**< Puerto del servidor. */
    const char* IP;         /**< Dirección IP del servidor. */
    char buffer[512];       /**< Buffer para enviar/recibir datos. */

    /**
     * @brief Conecta el cliente al servidor mediante TCP.
     * @return 0 si la conexión fue exitosa, -1 si hubo un error.
     */
    int connectTCP();

    /**
     * @brief Conecta el cliente al servidor mediante SSL.
     * @return 0 si la conexión fue exitosa, -1 si hubo un error.
     */
    int connectSSL();

    /**
     * @brief Muestra la respuesta recibida del servidor.
     */
    void showResponse();

    /**
     * @brief Envía una solicitud al servidor.
     */
    void sendRequest();
};

#endif
